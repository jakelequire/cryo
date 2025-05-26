/********************************************************************************
 *  Copyright 2025 Jacob LeQuire                                                *
 *  SPDX-License-Identifier: Apache-2.0                                         *
 *    Licensed under the Apache License, Version 2.0 (the "License");           *
 *    you may not use this file except in compliance with the License.          *
 *    You may obtain a copy of the License at                                   *
 *                                                                              *
 *    http://www.apache.org/licenses/LICENSE-2.0                                *
 *                                                                              *
 *    Unless required by applicable law or agreed to in writing, software       *
 *    distributed under the License is distributed on an "AS IS" BASIS,         *
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
 *    See the License for the specific language governing permissions and       *
 *    limitations under the License.                                            *
 *                                                                              *
 ********************************************************************************/

/**
 * @file parseUsingKw.c
 * @brief Implementation of the 'using' keyword for importing modules and specific exports
 *
 * The `using` keyword is used to import standard library modules and specific exports from modules.
 * - Full module: `using Std;`
 * - Module path: `using Std::IO;`
 * - Specific exports: `using Std::IO::{FS, Console};`
 */

#include "frontend/parser.h"
#include "tools/logger/logger_config.h"
#include "diagnostics/diagnostics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>

// Forward declarations for helper functions
static void parseModuleChain(Lexer *lexer, struct ModuleChainEntry *moduleChain, size_t *chainLength,
                             ParsingContext *context, Arena *arena, CompilerState *state);
static void parseTypeList(Lexer *lexer, const char *modulePath, const char **moduleChain, size_t chainLength,
                          ParsingContext *context, Arena *arena, CompilerState *state);
static void cleanupModuleChain(char **names, size_t length);
static bool importModule(const char *primaryModule, const char **moduleChain, size_t chainLength,
                         CompilerState *state);
static bool importSpecificExports(const char *modulePath, const char **exports, size_t exportCount,
                                  CompilerState *state);
static char *buildModulePath(const char *compilerRoot, const char *primaryModule, const char **moduleChain,
                             size_t chainLength);
static bool isValidModulePath(const char *path);
static char **findCryoFiles(const char *dirPath, size_t *fileCount);
static char *findModuleFile(const char *dirPath, const char *moduleName);

/**
 * @brief Parse a using keyword statement
 *
 * Handles all forms of the using statement:
 * - using Std;
 * - using Std::IO;
 * - using Std::IO::{FS, Console};
 *
 * @param lexer Current lexer state
 * @param context Parsing context
 * @param arena Memory arena
 * @param state Compiler state
 * @param globalTable Global symbol table
 * @return ASTNode* Node representing the using statement
 */
ASTNode *parseUsingKeyword(Lexer *lexer, ParsingContext *context,
                           Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing using keyword...");

    // Consume 'using' keyword
    consume(__LINE__, lexer, TOKEN_KW_USING, "Expected `using` keyword.",
            "parseUsingKeyword", arena, state, context);

    // Get primary module name (e.g., 'Std')
    Token primaryModuleToken = lexer->currentToken;
    char *primaryModule = strndup(primaryModuleToken.start, primaryModuleToken.length);
    logMessage(LMI, "INFO", "Parser", "Primary module: %s", primaryModule);

    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected module identifier.",
            "parseUsingKeyword", arena, state, context);

    // Check if this is a simple "using Std;" statement
    if (lexer->currentToken.type == TOKEN_SEMICOLON)
    {
        // Import the entire standard library
        if (!importFullStandardLibrary(primaryModule, state))
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to import full standard library: %s", primaryModule);
        }

        consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected `;` to end using statement.",
                "parseUsingKeyword", arena, state, context);

        // Create AST node for the using statement
        const char *moduleChain[] = {primaryModule};
        ASTNode *usingNode = createUsingNode(primaryModule, moduleChain, 1, arena, state, lexer);

        return usingNode;
    }

    // For module paths like "using Std::IO;"
    consume(__LINE__, lexer, TOKEN_DOUBLE_COLON, "Expected `::` or `;` after module identifier.",
            "parseUsingKeyword", arena, state, context);

    // Parse module chain (e.g., "IO::Console")
    struct ModuleChainEntry moduleChain[MAX_MODULE_CHAIN];
    size_t chainLength = 0;
    parseModuleChain(lexer, moduleChain, &chainLength, context, arena, state);

    // Convert module chain to string array for easier handling
    const char *moduleChainStr[MAX_MODULE_CHAIN];
    for (size_t i = 0; i < chainLength; i++)
    {
        moduleChainStr[i] = moduleChain[i].name;
    }

    // Handle specific exports if present (e.g., "{FS, Console}")
    if (lexer->currentToken.type == TOKEN_LBRACE)
    {
        // Get full module path for specific exports
        char *modulePath = buildModulePath(state->settings->compilerRootPath, primaryModule, moduleChainStr, chainLength);

        if (modulePath)
        {
            parseTypeList(lexer, modulePath, moduleChainStr, chainLength, context, arena, state);
            free(modulePath);
        }
        else
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to build module path for exports");
        }
    }
    else
    {
        // Import the entire module path
        if (!context->isParsingModuleFile)
        {
            if (!importModule(primaryModule, moduleChainStr, chainLength, state))
            {
                logMessage(LMI, "ERROR", "Parser", "Failed to import module: %s", primaryModule);
            }
        }
    }

    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected `;` to end using statement.",
            "parseUsingKeyword", arena, state, context);

    // Create AST node for the using statement
    ASTNode *usingNode = createUsingNode(primaryModule, moduleChainStr, chainLength, arena, state, lexer);

    return usingNode;
}

/**
 * @brief Import the entire standard library
 *
 * @param primaryModule The primary module name (e.g., "Std")
 * @param state Compiler state
 * @param globalTable Global symbol table
 * @return true if successful, false otherwise
 */
bool importFullStandardLibrary(const char *primaryModule, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Importing full standard library: %s", primaryModule);

    // Currently only support Std as primary module
    if (strcmp(primaryModule, "Std") != 0)
    {
        logMessage(LMI, "ERROR", "Parser", "Only the 'Std' module is currently supported");
        return false;
    }

    // Build path to standard library directory
    const char *compilerRoot = state->settings->compilerRootPath;
    char *stdLibPath = (char *)malloc(strlen(compilerRoot) + strlen("/cryo/Std/") + 1);
    if (!stdLibPath)
    {
        logMessage(LMI, "ERROR", "Parser", "Memory allocation failed for standard library path");
        return false;
    }

    strcpy(stdLibPath, compilerRoot);
    strcat(stdLibPath, "/cryo/Std/");

    if (!isValidModulePath(stdLibPath))
    {
        logMessage(LMI, "ERROR", "Parser", "Invalid standard library path: %s", stdLibPath);
        free(stdLibPath);
        return false;
    }

    // Find all .cryo files in the standard library directory and subdirectories
    importStandardLibraryRecursive(stdLibPath, state);

    free(stdLibPath);
    return true;
}

/**
 * @brief Recursively import all .cryo files in a directory and its subdirectories
 *
 * @param dirPath Directory path to start from
 * @param state Compiler state
 * @param globalTable Global symbol table
 */
void importStandardLibraryRecursive(const char *dirPath, CompilerState *state)
{
    __STACK_FRAME__
    DIR *dir = opendir(dirPath);
    if (!dir)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to open directory: %s", dirPath);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        // Skip . and .. directories
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        // Build full path
        char *fullPath = (char *)malloc(strlen(dirPath) + strlen(entry->d_name) + 2);
        if (!fullPath)
        {
            logMessage(LMI, "ERROR", "Parser", "Memory allocation failed for path");
            continue;
        }

        strcpy(fullPath, dirPath);
        strcat(fullPath, "/");
        strcat(fullPath, entry->d_name);

        // Check if it's a directory
        DIR *checkDir = opendir(fullPath);
        if (checkDir)
        {
            // It's a directory, recursively import
            closedir(checkDir);
            importStandardLibraryRecursive(fullPath, state);
        }
        else if (strstr(entry->d_name, ".cryo") && !strstr(entry->d_name, ".mod.cryo"))
        {
            // It's a .cryo file (but not a module file), import it
            logMessage(LMI, "INFO", "Parser", "Importing file: %s", fullPath);
            DTM->symbolTable->startSnapshot(DTM->symbolTable);
            ASTNode *node = compileForASTNode(fullPath, state);
            if (!node)
            {
                logMessage(LMI, "ERROR", "Parser", "Failed to compile file: %s", fullPath);
            }
            DTM->symbolTable->endSnapshot(DTM->symbolTable);
        }

        free(fullPath);
    }

    closedir(dir);
}

/**
 * @brief Parse a module chain (e.g., "IO::Console")
 *
 * @param lexer Current lexer state
 * @param moduleChain Array to store module chain entries
 * @param chainLength Pointer to store the length of the chain
 * @param context Parsing context
 * @param arena Memory arena
 * @param state Compiler state
 */
static void parseModuleChain(Lexer *lexer, struct ModuleChainEntry *moduleChain, size_t *chainLength,
                             ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__

    while (true)
    {
        Token current = lexer->currentToken;

        // Check for end of chain
        if (lexer->currentToken.type == TOKEN_LBRACE ||
            lexer->currentToken.type == TOKEN_SEMICOLON)
        {
            break;
        }

        // Add current identifier to chain
        moduleChain[*chainLength].name = strndup(current.start, current.length);
        moduleChain[*chainLength].length = current.length;
        (*chainLength)++;

        if (*chainLength >= MAX_MODULE_CHAIN)
        {
            logMessage(LMI, "ERROR", "Parser", "Module chain exceeds maximum length");
            cleanupModuleChain((char **)moduleChain, *chainLength);
            return;
        }

        // Consume identifier
        consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected module identifier.",
                "parseModuleChain", arena, state, context);

        // Check for end of chain or continue with next module
        if (lexer->currentToken.type == TOKEN_LBRACE ||
            lexer->currentToken.type == TOKEN_SEMICOLON)
        {
            break;
        }

        if (lexer->currentToken.type != TOKEN_DOUBLE_COLON)
        {
            char errorMsg[256];
            snprintf(errorMsg, sizeof(errorMsg),
                     "Expected `::` or `;` after identifier, got %s",
                     CryoTokenToString(lexer->currentToken.type));
            logMessage(LMI, "ERROR", "Parser", errorMsg);
            break;
        }

        consume(__LINE__, lexer, TOKEN_DOUBLE_COLON, "Expected `::` after identifier.",
                "parseModuleChain", arena, state, context);
    }

    // Log the parsed module chain
    logMessage(LMI, "INFO", "Parser", "Module chain parsed with %zu components", *chainLength);

    // Debug info
    DEBUG_PRINT_FILTER({
        printf("DEBUG: Module Chain Length: %zu\n", *chainLength);
        printf("DEBUG: Module Chain: \n");
        for (size_t i = 0; i < *chainLength; i++)
        {
            printf("%s", moduleChain[i].name);
            if (i < *chainLength - 1)
            {
                printf("::");
            }
            printf("\n");
        }
    });
}

/**
 * @brief Parse a list of specific exports (e.g., "{FS, Console}")
 *
 * @param lexer Current lexer state
 * @param modulePath Path to the module directory
 * @param moduleChain Module chain leading to the exports
 * @param chainLength Length of the module chain
 * @param context Parsing context
 * @param arena Memory arena
 * @param state Compiler state
 * @param globalTable Global symbol table
 */
static void parseTypeList(Lexer *lexer, const char *modulePath, const char **moduleChain, size_t chainLength,
                          ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing specific exports within braces...");

    consume(__LINE__, lexer, TOKEN_LBRACE, "Expected `{` for export list.",
            "parseTypeList", arena, state, context);

    // Array to store export names
    struct TypeEntry exportNames[MAX_MODULE_CHAIN];
    size_t exportCount = 0;

    do
    {
        Token exportToken = lexer->currentToken;
        exportNames[exportCount].name = strndup(exportToken.start, exportToken.length);
        exportNames[exportCount].length = exportToken.length;
        exportCount++;

        if (exportCount >= MAX_MODULE_CHAIN)
        {
            logMessage(LMI, "ERROR", "Parser", "Export list exceeds maximum length");
            cleanupModuleChain((char **)exportNames, exportCount);
            break;
        }

        consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected export identifier.",
                "parseTypeList", arena, state, context);

        if (lexer->currentToken.type == TOKEN_RBRACE)
        {
            break;
        }

        consume(__LINE__, lexer, TOKEN_COMMA, "Expected `,` after export identifier.",
                "parseTypeList", arena, state, context);
    } while (true);

    consume(__LINE__, lexer, TOKEN_RBRACE, "Expected `}` after export list.",
            "parseTypeList", arena, state, context);

    // Convert to array for import
    const char *exportArray[MAX_MODULE_CHAIN];
    for (size_t i = 0; i < exportCount; i++)
    {
        exportArray[i] = exportNames[i].name;
        logMessage(LMI, "INFO", "Parser", "Export to import: %s", exportNames[i].name);
    }

    // Import specific exports from module
    if (!context->isParsingModuleFile)
    {
        importSpecificExports(modulePath, exportArray, exportCount, state);
    }

    // Cleanup
    cleanupModuleChain((char **)exportNames, exportCount);
}

/**
 * @brief Free memory allocated for a module chain
 *
 * @param names Array of strings to free
 * @param length Number of strings
 */
static void cleanupModuleChain(char **names, size_t length)
{
    __STACK_FRAME__
    for (size_t i = 0; i < length; i++)
    {
        free(names[i]);
    }
}

/**
 * @brief Import a module given a module path
 *
 * @param primaryModule Primary module name (e.g., "Std")
 * @param moduleChain Module chain to import
 * @param chainLength Length of module chain
 * @param state Compiler state
 * @param globalTable Global symbol table
 * @return true if successful, false otherwise
 */
static bool importModule(const char *primaryModule, const char **moduleChain, size_t chainLength,
                         CompilerState *state)
{
    __STACK_FRAME__
    // Currently only support Std as primary module
    if (strcmp(primaryModule, "Std") != 0)
    {
        logMessage(LMI, "ERROR", "Parser", "Only the 'Std' module is currently supported");
        return false;
    }

    // Build full module path
    char *modulePath = buildModulePath(state->settings->compilerRootPath, primaryModule, moduleChain, chainLength);
    if (!modulePath)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to build module path");
        return false;
    }

    logMessage(LMI, "INFO", "Parser", "Importing module from path: %s", modulePath);

    // Check if path exists
    if (!isValidModulePath(modulePath))
    {
        logMessage(LMI, "ERROR", "Parser", "Invalid module path: %s", modulePath);
        free(modulePath);
        return false;
    }

    // Find module file (.mod.cryo) if it exists
    char *moduleFile = findModuleFile(modulePath, moduleChain[chainLength - 1]);
    if (moduleFile)
    {
        // Import the module file
        logMessage(LMI, "INFO", "Parser", "Importing module file: %s", moduleFile);
        DTM->symbolTable->startSnapshot(DTM->symbolTable);
        ASTNode *node = compileForASTNode(moduleFile, state);
        free(moduleFile);
        DTM->symbolTable->endSnapshot(DTM->symbolTable);
        if (!node)
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to compile module file");
            free(modulePath);
            return false;
        }
    }
    else
    {
        // If no module file, import all .cryo files in the directory
        size_t fileCount = 0;
        char **cryoFiles = findCryoFiles(modulePath, &fileCount);

        if (!cryoFiles || fileCount == 0)
        {
            logMessage(LMI, "ERROR", "Parser", "No .cryo files found in module directory");
            free(modulePath);
            return false;
        }

        // Import each .cryo file
        for (size_t i = 0; i < fileCount; i++)
        {
            logMessage(LMI, "INFO", "Parser", "Importing file: %s", cryoFiles[i]);
            DTM->symbolTable->startSnapshot(DTM->symbolTable);
            ASTNode *node = compileForASTNode(cryoFiles[i], state);

            if (!node)
            {
                logMessage(LMI, "ERROR", "Parser", "Failed to compile file: %s", cryoFiles[i]);
            }
            DTM->symbolTable->endSnapshot(DTM->symbolTable);

            free(cryoFiles[i]);
        }

        free(cryoFiles);
    }

    free(modulePath);
    return true;
}

/**
 * @brief Import specific exports from a module
 *
 * @param modulePath Path to the module directory
 * @param exports Array of export names to import
 * @param exportCount Number of exports
 * @param state Compiler state
 * @param globalTable Global symbol table
 * @return true if successful, false otherwise
 */
static bool importSpecificExports(const char *modulePath, const char **exports, size_t exportCount,
                                  CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Importing specific exports from module: %s", modulePath);

    // Check if path exists
    if (!isValidModulePath(modulePath))
    {
        logMessage(LMI, "ERROR", "Parser", "Invalid module path: %s", modulePath);
        return false;
    }

    // Find all .cryo files in the directory
    size_t fileCount = 0;
    char **cryoFiles = findCryoFiles(modulePath, &fileCount);

    if (!cryoFiles || fileCount == 0)
    {
        logMessage(LMI, "ERROR", "Parser", "No .cryo files found in module directory");
        return false;
    }

    // For each export, find corresponding file and import it
    bool allExportsFound = true;
    for (size_t i = 0; i < exportCount; i++)
    {
        const char *exportName = exports[i];
        bool exportFound = false;

        // Look for a file named <ExportName>.cryo
        char *expectedFileName = (char *)malloc(strlen(exportName) + 7); // +7 for ".cryo\0"
        if (!expectedFileName)
        {
            logMessage(LMI, "ERROR", "Parser", "Memory allocation failed");
            continue;
        }

        sprintf(expectedFileName, "%s.cryo", exportName);

        for (size_t j = 0; j < fileCount; j++)
        {
            // Get just the filename without path
            const char *fileName = strrchr(cryoFiles[j], '/');
            if (fileName)
            {
                fileName++; // Skip the slash
            }
            else
            {
                fileName = cryoFiles[j]; // No slash in path
            }

            if (strcmp(fileName, expectedFileName) == 0)
            {
                // Found the file for this export
                logMessage(LMI, "INFO", "Parser", "Importing export %s from file: %s",
                           exportName, cryoFiles[j]);
                DTM->symbolTable->startSnapshot(DTM->symbolTable);
                ASTNode *node = compileForASTNode(cryoFiles[j], state);
                if (!node)
                {
                    logMessage(LMI, "ERROR", "Parser", "Failed to compile file: %s", cryoFiles[j]);
                    allExportsFound = false;
                }
                DTM->symbolTable->endSnapshot(DTM->symbolTable);
                exportFound = true;
                break;
            }
        }

        free(expectedFileName);

        if (!exportFound)
        {
            logMessage(LMI, "ERROR", "Parser", "Export not found: %s", exportName);
            allExportsFound = false;
        }
    }

    // Cleanup
    for (size_t i = 0; i < fileCount; i++)
    {
        free(cryoFiles[i]);
    }
    free(cryoFiles);

    return allExportsFound;
}

/**
 * @brief Build a full module path from components
 *
 * @param compilerRoot Compiler root directory
 * @param primaryModule Primary module name (e.g., "Std")
 * @param moduleChain Module chain
 * @param chainLength Length of module chain
 * @return char* Dynamically allocated path string (caller must free)
 */
static char *buildModulePath(const char *compilerRoot, const char *primaryModule, const char **moduleChain,
                             size_t chainLength)
{
    __STACK_FRAME__
    // Calculate required size
    size_t pathLen = strlen(compilerRoot) + strlen("/cryo/") + strlen(primaryModule) + 2; // +2 for slash and null

    for (size_t i = 0; i < chainLength; i++)
    {
        pathLen += strlen(moduleChain[i]) + 1; // +1 for slash
    }

    // Allocate memory for path
    char *path = (char *)malloc(pathLen);
    if (!path)
    {
        logMessage(LMI, "ERROR", "Parser", "Memory allocation failed for module path");
        return NULL;
    }

    // Build path
    strcpy(path, compilerRoot);
    strcat(path, "/cryo/");
    strcat(path, primaryModule);
    strcat(path, "/");

    for (size_t i = 0; i < chainLength; i++)
    {
        strcat(path, moduleChain[i]);
        strcat(path, "/");
    }

    return path;
}

/**
 * @brief Check if a module path is valid (exists and is a directory)
 *
 * @param path Path to check
 * @return true if valid, false otherwise
 */
static bool isValidModulePath(const char *path)
{
    __STACK_FRAME__
    DIR *dir = opendir(path);
    if (!dir)
    {
        return false;
    }

    closedir(dir);
    return true;
}

/**
 * @brief Find all .cryo files in a directory
 *
 * @param dirPath Directory path
 * @param fileCount Pointer to store number of files found
 * @return char** Array of file paths (caller must free each string and the array)
 */
static char **findCryoFiles(const char *dirPath, size_t *fileCount)
{
    __STACK_FRAME__
    DIR *dir = opendir(dirPath);
    if (!dir)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to open directory: %s", dirPath);
        return NULL;
    }

    // Count .cryo files first
    struct dirent *entry;
    *fileCount = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        if (strstr(entry->d_name, ".cryo") && !strstr(entry->d_name, ".mod.cryo"))
        {
            (*fileCount)++;
        }
    }

    if (*fileCount == 0)
    {
        closedir(dir);
        return NULL;
    }

    // Allocate array for file paths
    char **files = (char **)malloc(sizeof(char *) * (*fileCount));
    if (!files)
    {
        logMessage(LMI, "ERROR", "Parser", "Memory allocation failed for file array");
        closedir(dir);
        return NULL;
    }

    // Fill array with file paths
    rewinddir(dir);
    size_t index = 0;

    while ((entry = readdir(dir)) != NULL && index < *fileCount)
    {
        if (strstr(entry->d_name, ".cryo") && !strstr(entry->d_name, ".mod.cryo"))
        {
            // Build full path
            files[index] = (char *)malloc(strlen(dirPath) + strlen(entry->d_name) + 2); // +2 for slash and null
            if (!files[index])
            {
                logMessage(LMI, "ERROR", "Parser", "Memory allocation failed for file path");

                // Cleanup already allocated paths
                for (size_t i = 0; i < index; i++)
                {
                    free(files[i]);
                }

                free(files);
                closedir(dir);
                return NULL;
            }

            strcpy(files[index], dirPath);
            strcat(files[index], "/");
            strcat(files[index], entry->d_name);

            index++;
        }
    }

    closedir(dir);
    return files;
}

/**
 * @brief Find a module file (.mod.cryo) in a directory
 *
 * @param dirPath Directory path
 * @param moduleName Name of the module to find
 * @return char* Path to module file if found (caller must free), NULL otherwise
 */
static char *findModuleFile(const char *dirPath, const char *moduleName)
{
    __STACK_FRAME__
    DIR *dir = opendir(dirPath);
    if (!dir)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to open directory: %s", dirPath);
        return NULL;
    }

    // Create expected filename
    char *expectedName = (char *)malloc(strlen(moduleName) + 10); // +10 for ".mod.cryo\0"
    if (!expectedName)
    {
        logMessage(LMI, "ERROR", "Parser", "Memory allocation failed for module filename");
        closedir(dir);
        return NULL;
    }

    sprintf(expectedName, "%s.mod.cryo", moduleName);

    // Look for the module file
    struct dirent *entry;
    char *moduleFile = NULL;

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, expectedName) == 0)
        {
            // Found the module file
            moduleFile = (char *)malloc(strlen(dirPath) + strlen(expectedName) + 2); // +2 for slash and null
            if (!moduleFile)
            {
                logMessage(LMI, "ERROR", "Parser", "Memory allocation failed for module file path");
                free(expectedName);
                closedir(dir);
                return NULL;
            }

            strcpy(moduleFile, dirPath);
            strcat(moduleFile, "/");
            strcat(moduleFile, expectedName);
            break;
        }
    }

    free(expectedName);
    closedir(dir);

    return moduleFile;
}
