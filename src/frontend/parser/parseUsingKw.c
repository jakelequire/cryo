/********************************************************************************
 *  Copyright 2024 Jacob LeQuire                                                *
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
#include "symbolTable/cInterfaceTable.h"
#include "frontend/parser.h"

// The `using` keyword is used to import a STD Library module only.
// Syntax: `using <module>::<?scope/fn>::<?scope/fn>;`
// Eventually, you will be able to import specific functions or scopes from a module.
// Like this: `using Std::Types::{Int, Float};`
void parseUsingKeyword(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context,
                       Arena *arena, CompilerState *state, TypeTable *typeTable,
                       CryoGlobalSymbolTable *globalTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing using keyword...");
    consume(__LINE__, lexer, TOKEN_KW_USING, "Expected `using` keyword.",
            "parseUsingKeyword", table, arena, state, typeTable, context);

    // Get primary module name
    Token primaryModuleToken = lexer->currentToken;
    char *primaryModule = strndup(primaryModuleToken.start, primaryModuleToken.length);
    logMessage("INFO", __LINE__, "Parser", "Primary module: %s", primaryModule);

    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.",
            "parseUsingKeyword", table, arena, state, typeTable, context);
    consume(__LINE__, lexer, TOKEN_DOUBLE_COLON, "Expected `::` after primary module.",
            "parseUsingKeyword", table, arena, state, typeTable, context);

    // Parse module chain
    struct ModuleChainEntry moduleChain[MAX_MODULE_CHAIN];
    size_t chainLength = 0;
    parseModuleChain(lexer, moduleChain, &chainLength, table, context, arena, state, typeTable);

    // Handle type list if present
    if (lexer->currentToken.type == TOKEN_LBRACE)
    {
        const char *lastModule = moduleChain[chainLength - 1].name;
        parseTypeList(lexer, lastModule, table, context, arena,
                      state, typeTable, globalTable);
    }

    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected `;` to end using statement.",
            "parseUsingKeyword", table, arena, state, typeTable, context);

    // Import the module chain
    const char *moduleChainStr[MAX_MODULE_CHAIN];
    for (size_t i = 0; i < chainLength; i++)
    {
        moduleChainStr[i] = moduleChain[i].name;
    }

    if (!context->isParsingModuleFile)
    {
        importUsingModule(primaryModule, moduleChainStr, chainLength, state, globalTable);
    }

    free(primaryModule);
}

// Helper function implementations
static void cleanupModuleChain(char **names, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        free(names[i]);
    }
}

static void parseModuleChain(Lexer *lexer, struct ModuleChainEntry *moduleChain, size_t *chainLength,
                             CryoSymbolTable *table, ParsingContext *context, Arena *arena,
                             CompilerState *state, TypeTable *typeTable)
{
    while (true)
    {
        Token current = lexer->currentToken;

        // Check for special cases before adding to chain
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
            logMessage("ERROR", __LINE__, "Parser", "Module chain exceeds maximum length");
            cleanupModuleChain((char **)moduleChain, *chainLength);
            return;
        }

        // Must be an identifier followed by either :: or ; or {
        consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.",
                "parseModuleChain", table, arena, state, typeTable, context);

        // After an identifier, we should either see :: or end of chain
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
            logMessage("ERROR", __LINE__, "Parser", errorMsg);
            break;
        }

        consume(__LINE__, lexer, TOKEN_DOUBLE_COLON, "Expected `::` after identifier.",
                "parseModuleChain", table, arena, state, typeTable, context);
    }
}

static void parseTypeList(Lexer *lexer, const char *lastModule, CryoSymbolTable *table,
                          ParsingContext *context, Arena *arena, CompilerState *state,
                          TypeTable *typeTable, CryoGlobalSymbolTable *globalTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing specific types within braces...");
    consume(__LINE__, lexer, TOKEN_LBRACE, "Expected `{` after `::`.",
            "parseTypeList", table, arena, state, typeTable, context);

    struct TypeEntry typeNames[MAX_MODULE_CHAIN];
    size_t typeCount = 0;

    printf("DEBUG: Parsing specific types within braces...\n");

    do
    {
        Token typeToken = lexer->currentToken;
        typeNames[typeCount].name = strndup(typeToken.start, typeToken.length);
        typeNames[typeCount].length = typeToken.length;
        typeCount++;

        if (typeCount >= MAX_MODULE_CHAIN)
        {
            logMessage("ERROR", __LINE__, "Parser", "Type list exceeds maximum length");
            cleanupModuleChain((char **)typeNames, typeCount);
            break;
        }

        consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected type identifier.",
                "parseTypeList", table, arena, state, typeTable, context);

        if (lexer->currentToken.type == TOKEN_RBRACE)
        {
            break;
        }

        consume(__LINE__, lexer, TOKEN_COMMA, "Expected `,` after type identifier.",
                "parseTypeList", table, arena, state, typeTable, context);
    } while (true);

    consume(__LINE__, lexer, TOKEN_RBRACE, "Expected `}` after type list.",
            "parseTypeList", table, arena, state, typeTable, context);

    // Convert to array for import
    const char *typeArray[MAX_MODULE_CHAIN];
    for (size_t i = 0; i < typeCount; i++)
    {
        printf("DEBUG: Type Name: %s\n", typeNames[i].name);
        typeArray[i] = typeNames[i].name;
    }

    importSpecificNamespaces(lastModule, typeArray, typeCount, state, globalTable);
    cleanupModuleChain((char **)typeNames, typeCount);
}

// Now that we have the module chain, we can import the module into the current scope.
// This function will import into the Global Symbol Table.
void importUsingModule(const char *primaryModule, const char *moduleChain[], size_t moduleCount,
                       CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    // Import the module and scope or function
    // This will allow the module to be used in the current scope.

    if (strcmp(primaryModule, "Std") != 0)
    {
        // Safeguard for now until we have more modules.
        logMessage("ERROR", __LINE__, "Parser", "Primary module must be `Std`.");
        return;
    }

    int lastModuleIndex = moduleCount - 1;

    // The primary module should be the first module in the chain regardless of the chain length.
    const char *rootLevelModule = moduleChain[0];
    // This should be giving the path: {CRYO_ROOT}/Std/{rootLevelModule} | {CRYO_ROOT}/Std/Types
    const char *primaryModulePath = getSTDLibraryModulePath(rootLevelModule, state);
    if (primaryModulePath == NULL)
    {
        logMessage("ERROR", __LINE__, "Parser", "Invalid primary module path.");
        return;
    }
    printf("Primary Module Path To Find Module File: %s\n", primaryModulePath);

    // Check if the module file exists
    const char **moduleFiles = getFilesInModuleDir(primaryModulePath);
    if (moduleFiles == NULL)
    {
        logMessage("ERROR", __LINE__, "Parser", "getSTDLibPath: No module files found in directory: %s", primaryModulePath);
    }

    int modCount = 0;
    while (moduleFiles[modCount] != NULL)
    {
        modCount++;
    }

    const char *moduleFile = findModuleFile(moduleFiles, modCount, rootLevelModule);
    if (moduleFile == NULL)
    {
        logMessage("ERROR", __LINE__, "Parser", "getSTDLibPath: Module file not found in directory: %s", primaryModulePath);
    }

    // DEBUG -------------------------------------
    printf("DEBUG: Module Count: %zu\n", moduleCount);
    printf("DEBUG: Primary Module: %s\n", primaryModule);
    printf("DEBUG: Module Chain: ");
    for (size_t i = 0; i < moduleCount; i++)
    {
        printf("%s", moduleChain[i]);
        if (i < moduleCount - 1)
        {
            printf(",");
        }
    }
    // DEBUG -------------------------------------

    // Compile the module file definitions
    int result = compileAndImportModuleToCurrentScope(moduleFile, state, globalTable);
    if (result != 0)
    {
        logMessage("ERROR", __LINE__, "Parser", "Failed to compile and import module file definitions.");
        return;
    }

    DEBUG_BREAKPOINT;
}

const char *getSTDLibraryModulePath(const char *moduleName, CompilerState *state)
{
    // Get the path to the STD Library module
    // This will be used to import the module into the current scope.
    // Root Directory: {CRYO_ROOT}/std/
    const char *rootDir = state->settings->rootDir;
    printf("getSTDLibPath: Root Directory: %s\n", rootDir);
    printf("getSTDLibPath: Module Name: %s\n", moduleName);

    char *modulePath = (char *)malloc(strlen(rootDir) + strlen(moduleName) + 7);
    strcpy(modulePath, rootDir);
    strcat(modulePath, "/Std/");
    strcat(modulePath, moduleName);
    strcat(modulePath, "/");

    const char *modulePathStr = (const char *)modulePath;
    printf("getSTDLibPath: Module Path: %s\n", modulePathStr);

    bool isValidDir = dirExists(modulePathStr);
    if (!isValidDir)
    {

        logMessage("ERROR", __LINE__, "Parser", "getSTDLibPath: Invalid module path: %s", modulePathStr);
        free(modulePath);
        return NULL;
    }

    printf("getSTDLibPath: Module Path: %s\n", modulePathStr);

    return modulePathStr;
}

// If the `using` keyword is used to import a whole module, this function will return all files in the module directory.
const char **getFilesInModuleDir(const char *modulePath)
{
    // Get all files in the module directory
    // This will be used to import the module into the current scope.
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(modulePath)) != NULL)
    {
        // Count the number of files in the directory
        size_t fileCount = 0;
        while ((ent = readdir(dir)) != NULL)
        {
            if (!nonCryoFileCheck(ent->d_name))
            {
                continue;
            }
            printf("getFilesInModuleDir: Found File: %s\n", ent->d_name);
            fileCount++;
        }

        // Allocate memory for the file paths
        const char **moduleFiles = (const char **)malloc(fileCount * sizeof(const char *));
        rewinddir(dir);

        // Store the file paths
        size_t i = 0;
        while ((ent = readdir(dir)) != NULL)
        {
            // Check for only valid Cryo files
            if (!nonCryoFileCheck(ent->d_name))
            {
                continue;
            }
            char *filePath = (char *)malloc(strlen(modulePath) + strlen(ent->d_name) + 1);
            strcpy(filePath, modulePath);
            strcat(filePath, ent->d_name);
            moduleFiles[i] = (const char *)filePath;
            printf("[DEBUG] getFilesInModuleDir: File Path: %s\n", moduleFiles[i]);
            i++;
        }

        closedir(dir);
        return moduleFiles;
    }

    return NULL;
}

bool nonCryoFileCheck(const char *fullPath)
{
    // Check if the file is a Cryo file
    // This will be used to import the module into the current scope.
    const char *needle = ".cryo";
    if (strstr(fullPath, needle) != NULL)
    {
        return true;
    }

    return false;
}

// A module file is a file that encapsulates a module's scope or functions accessible to the current scope.
// The module files naming convention is: `{module_name}.mod.cryo`. This function will return the file path to the module file.
// If the module file is not found, this function will return NULL.
const char *findModuleFile(const char **moduleFiles, size_t moduleCount, const char *moduleName)
{
    char *needle = (char *)malloc(strlen(moduleName) + 6);
    strcpy(needle, moduleName);
    strcat(needle, ".mod.cryo");
    const char *needleStr = (const char *)needle;

    printf("findModuleFile: File To Find: %s\n", needleStr);
    printf("Module Count: %zu\n", moduleCount);

    for (size_t i = 0; i < moduleCount; i++)
    {
        const char *currentFilePath = moduleFiles[i];
        const char *currentFile = trimFilePath(currentFilePath);
        printf("Checking File: %s Against: %s\n", currentFile, needleStr);

        if (strstr(currentFile, needleStr) != NULL)
        {
            if (!fileExists(currentFilePath))
            {
                logMessage("ERROR", __LINE__, "Parser", "findModuleFile: Invalid module file path: %s", currentFile);
                continue;
            }
            printf("findModuleFile: Found Module File: %s\n", currentFile);
            printf("findModuleFile: Returning String: %s\n", currentFilePath);
            return currentFilePath;
        }
    }

    return NULL;
}

const char *findRegularFile(const char **moduleFiles, size_t moduleCount, const char *fileName)
{
    for (size_t i = 0; i < moduleCount; i++)
    {
        const char *currentFilePath = moduleFiles[i];
        const char *currentFile = trimFilePath(currentFilePath);
        if (strstr(currentFile, fileName) != NULL)
        {
            if (!fileExists(currentFilePath))
            {
                logMessage("ERROR", __LINE__, "Parser", "findRegularFile: Invalid file path: %s", currentFile);
                continue;
            }
            return currentFilePath;
        }
    }

    return NULL;
}

// =================================================================================================
// `using` Keyword Parsing While Importing Modules Is Active.

// This function views the `using` keyword slightly differently while in a `.mod.cryo` file.
// It will take the modules within the current module file and combine them into a singular compilation unit.

// =================================================================================================
// Compile and Import Module File Definitions

int compileAndImportModuleToCurrentScope(const char *modulePath, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    // Compile the module file definitions
    // This will be used to import the module into the current scope.
    logMessage("INFO", __LINE__, "Parser", "Compiling module file definitions...");

    const char *dependencyDir = GetDependencyDirStr(globalTable);

    ASTNode *programNode = compileModuleFileToProgramNode(modulePath, dependencyDir, state, globalTable);
    if (programNode == NULL)
    {
        logMessage("ERROR", __LINE__, "Parser", "Failed to compile module file definitions.");
        return 1;
    }

    DEBUG_BREAKPOINT;
}

void importSpecificNamespaces(const char *primaryModule, const char *namespaces[], size_t namespaceCount,
                              CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    printf("DEBUG: Importing Specific Namespaces... Primary Module: %s\n", primaryModule);
    const char *rootTypeDir = getSTDLibraryModulePath(primaryModule, state);
    if (rootTypeDir == NULL)
    {
        logMessage("ERROR", __LINE__, "Parser", "Invalid primary module path.");
        return;
    }
    printf("DEBUG: Root Type Dir: %s\n", rootTypeDir);
    printf("DEBUG: Primary Module: %s\n", primaryModule);

    const char **moduleFiles = getFilesInModuleDir(rootTypeDir);
    if (moduleFiles == NULL)
    {
        logMessage("ERROR", __LINE__, "Parser", "No module files found in directory: %s", rootTypeDir);
        return;
    }

    // Count total module files
    size_t fileCount = 0;
    while (moduleFiles[fileCount] != NULL)
    {
        fileCount++;
    }

    // Allocate array for modules to compile
    const char **modulesToCompile = (const char **)malloc(sizeof(char *) * namespaceCount);
    memset(modulesToCompile, 0, sizeof(char *) * namespaceCount);

    // Match each namespace to its corresponding .cryo file
    for (size_t i = 0; i < namespaceCount; i++)
    {
        const char *currentNamespace = namespaces[i];
        // Create the expected filename pattern (e.g., "String.cryo")
        char *expectedFile = (char *)malloc(strlen(currentNamespace) + 6); // +6 for ".cryo\0"
        sprintf(expectedFile, "%s.cryo", currentNamespace);

        // Search through module files for a match
        bool found = false;
        for (size_t j = 0; j < fileCount; j++)
        {
            const char *currentFile = moduleFiles[j];
            // Get just the filename without the path
            const char *lastSlash = strrchr(currentFile, '/');
            const char *filename = lastSlash ? lastSlash + 1 : currentFile;

            if (strcmp(filename, expectedFile) == 0)
            {
                modulesToCompile[i] = strdup(moduleFiles[j]); // Store the full path
                found = true;
                printf("DEBUG: Found Module File: %s\n", filename);
                break;
            }
            else
            {
                printf("DEBUG: No Match: %s\n", filename);
                printf("DEBUG: Expected: %s\n", expectedFile);
            }
        }

        if (!found)
        {
            logMessage("ERROR", __LINE__, "Parser",
                       "Module file not found in directory for namespace %s: %s",
                       currentNamespace, rootTypeDir);
        }
    }

    // DEBUG: print the modules to compile
    for (size_t i = 0; i < namespaceCount; i++)
    {
        printf("DEBUG: Module To Compile: %s\n",
               modulesToCompile[i] ? modulesToCompile[i] : "(null)");
    }

    // Now compile each found module
    for (size_t i = 0; i < namespaceCount; i++)
    {
        if (modulesToCompile[i])
        {
            logMessage("INFO", __LINE__, "Parser", "Compiling module: %s", modulesToCompile[i]);
            int result = compileAndImportModuleToCurrentScope(modulesToCompile[i], state, globalTable);
            if (result != 0)
            {
                logMessage("ERROR", __LINE__, "Parser",
                           "Failed to compile module: %s", modulesToCompile[i]);
            }
            free((void *)modulesToCompile[i]);
        }
    }
    free(modulesToCompile);

    DEBUG_BREAKPOINT;
}

// =================================================================================================

ASTNode *compileModuleFileDefinitions(const char *modulePath, CryoGlobalSymbolTable *globalTable, CompilerState *state)
{
    // Compile the module file definitions
    // This will be used to import the module into the current scope.
    logMessage("INFO", __LINE__, "Parser", "Compiling module file definitions...");

    const char *dependencyDir = GetDependencyDirStr(globalTable);

    ASTNode *programNode = compileModuleFileToProgramNode(modulePath, dependencyDir, state, globalTable);
    if (programNode == NULL)
    {
        logMessage("ERROR", __LINE__, "Parser", "Failed to compile module file definitions.");
        return NULL;
    }

    DEBUG_BREAKPOINT;
    return NULL;
}
