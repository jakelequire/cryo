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
#include <dirent.h>

ASTNode *parseModuleDeclaration(CryoVisibilityType visibility,
                                Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable, CryoGlobalSymbolTable *globalTable)
{
    // The syntax for the module keyword is as follows:
    // <visibility> module <module_name>;
    // This will include all the definitions within the module.
    logMessage(LMI, "INFO", "Parser", "Parsing module declaration...");

    consume(__LINE__, lexer, TOKEN_KW_MODULE, "Expected `module` keyword.",
            "parseModuleDeclaration", table, arena, state, typeTable, context);

    // Get the module name
    Token moduleNameToken = lexer->currentToken;
    char *moduleName = strndup(moduleNameToken.start, moduleNameToken.length);
    logMessage(LMI, "INFO", "Parser", "Module name: %s", moduleName);

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

    // Handle the module parsing
    int moduleParsingResult = handleModuleParsing(fileArray, fileCount, state, globalTable);
    if (moduleParsingResult == 0)
    {
        logMessage(LMI, "INFO", "Parser", "Module parsing successful.");
    }
    else
    {
        logMessage(LMI, "ERROR", "Parser", "Module parsing failed.");
    }

    DEBUG_BREAKPOINT;
}

int handleModuleParsing(const char *fileArray[], int fileCount, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    logMessage(LMI, "INFO", "Parser", "Handling module parsing...");

    for (int i = 0; i < fileCount; i++)
    {
        const char *filePath = fileArray[i];
        logMessage(LMI, "INFO", "Parser", "File path: %s", filePath);
        DEBUG_BREAKPOINT;
        SymbolTable *moduleSymbolTable = compileToReapSymbols(filePath, "", state);
    }

    DEBUG_BREAKPOINT;
}

const char **getDirFileList(const char *dir)
{
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
                fileList[fileCount] = fileName;
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
    if (strstr(fileName, ".cryo") != NULL)
    {
        return true;
    }
    return false;
}