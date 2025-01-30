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
#include "diagnostics/diagnostics.h"
#include <dirent.h>

ASTNode *parseModuleDeclaration(CryoVisibilityType visibility,
                                Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    // The syntax for the module keyword is as follows:
    // <visibility> module <module_name>;
    // This will include all the definitions within the module.
    logMessage(LMI, "INFO", "Parser", "Parsing module declaration...");

    consume(__LINE__, lexer, TOKEN_KW_MODULE, "Expected `module` keyword.",
            "parseModuleDeclaration", arena, state, typeTable, context);

    // Get the module name
    Token moduleNameToken = lexer->currentToken;
    char *moduleName = strndup(moduleNameToken.start, moduleNameToken.length);

    logMessage(LMI, "INFO", "Parser", "Module name: %s", moduleName);

    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected module name.",
            "parseModuleDeclaration", arena, state, typeTable, context);

    const char *getCurrentFileLocation = getCurrentFileLocationFromLexer(lexer);
    logMessage(LMI, "INFO", "Parser", "Current file location: %s", getCurrentFileLocation);

    const char *dirPath = removeFileFromPath(getCurrentFileLocation);
    logMessage(LMI, "INFO", "Parser", "Directory path: %s", dirPath);

    const char *fileArray[MAX_FILES];
    const char **fileList = getDirFileList(dirPath);
    int fileCount = 0;
    while (*fileList != NULL)
    {
        logMessage(LMI, "INFO", "Parser", "File: %s", *fileList);
        fileArray[fileCount] = *fileList;
        fileCount++;
        fileList++;
    }
    // Null terminate the file array
    fileArray[fileCount] = NULL;

    const char *moduleFilePath = getModuleFile(fileArray, moduleName);
    logMessage(LMI, "INFO", "Parser", "Module file path: %s", moduleFilePath);
    // Handle the module parsing

    int moduleParsingResult = handleModuleParsing(moduleFilePath, state, globalTable, arena);
    if (moduleParsingResult == 0)
    {
        logMessage(LMI, "INFO", "Parser", "Module parsing successful.");
        GDM->printStackTrace(GDM);
    }
    else
    {
        logMessage(LMI, "ERROR", "Parser", "Module parsing failed.");
        CONDITION_FAILED;
    }

    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected `;` after module declaration.",
            "parseModuleDeclaration", arena, state, typeTable, context);

    // Create the module node
    ASTNode *moduleNode = createModuleNode(moduleName, arena, state, typeTable, lexer);
    if (!moduleNode)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to create module node.");
        CONDITION_FAILED;
    }

    logMessage(LMI, "INFO", "Parser", "Module node created successfully.");

    return moduleNode;
}

int handleModuleParsing(const char *moduleSrcPath, CompilerState *state, CryoGlobalSymbolTable *globalTable, Arena *arena)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Handling module parsing...");

    logMessage(LMI, "INFO", "Parser", "File path: %s", moduleSrcPath);
    const char *fileName = getFileNameFromPath(moduleSrcPath);
    const char *rootDir = state->settings->rootDir;
    const char *outputFileName = changeFileExtension(fileName, ".ll");
    const char *moduleDir = appendPathToFileName(rootDir, "build/out/deps", true);
    const char *outputPath = appendPathToFileName(moduleDir, outputFileName, true);

    logMessage(LMI, "INFO", "Parser", "Output path: %s", outputPath);

    SymbolTable *moduleSymbolTable = compileToReapSymbols(moduleSrcPath, outputPath, state, arena, globalTable);
    if (moduleSymbolTable == NULL)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to compile module file: %s", moduleSrcPath);
        return 1;
    }

    logMessage(LMI, "INFO", "Parser", "Module file compiled successfully.");

    // Import the module symbols into the global symbol table
    ImportReapedTable(globalTable, moduleSymbolTable);

    printGlobalSymbolTable(globalTable);

    return 0;
}

const char *getModuleFile(const char **dirList, const char *moduleName)
{
    __STACK_FRAME__
    for (int i = 0; i < sizeof(dirList); i++)
    {
        const char *fileName = dirList[i];
        if (strstr(fileName, moduleName) != NULL)
        {
            return fileName;
        }
    }
    return NULL;
}

const char **getDirFileList(const char *dir)
{
    __STACK_FRAME__
    DIR *dp;
    struct dirent *ep;
    const char **fileList = (const char **)malloc(sizeof(const char *) * 128);
    int fileCount = 0;

    dp = opendir(dir);
    if (dp != NULL)
    {
        while ((ep = readdir(dp)))
        {
            if (ep->d_type == DT_REG)
            {
                if (!isValidCryoFile(ep->d_name))
                {
                    // If the file is not a .cryo file, skip it
                    continue;
                }
                const char *fileName = ep->d_name;
                const char *fullFilePath = appendPathToFileName(dir, fileName, true);
                fileList[fileCount] = fullFilePath;
                fileCount++;
            }
        }
        (void)closedir(dp);
    }
    else
    {
        perror("Couldn't open the directory");
    }

    return fileList;
}

bool isValidCryoFile(const char *fileName)
{
    __STACK_FRAME__
    if (strstr(fileName, ".cryo") != NULL)
    {
        return true;
    }
    return false;
}
