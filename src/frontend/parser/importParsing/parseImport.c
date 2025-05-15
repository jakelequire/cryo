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
#include "frontend/parser.h"
#include "tools/logger/logger_config.h"
#include "diagnostics/diagnostics.h"
#include <dirent.h>
#include <libgen.h> // For dirname
#include <limits.h> // For PATH_MAX
#include <unistd.h> // For realpath

/*
The import keyword can have the following syntax:
```
import "./module.cryo" // Import relative to the current file
*/
ASTNode *handleImportParsing(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    consume(__LINE__, lexer, TOKEN_KW_IMPORT, "Expected `import` keyword", "Import", arena, state, context);

    // Get the next token
    Token token = lexer->currentToken;

    // Check if the next token
    switch (token.type)
    {
    case TOKEN_STRING_LITERAL:
    {
        // Get the module path
        const char *modulePath = strndup(token.start, token.length);
        logMessage(LMI, "INFO", "Import", "Importing module: <%s>", modulePath);
        getNextToken(lexer, arena, state); // Consume the string literal token

        return handleRelativeImport(modulePath, lexer, context, arena, state);
    }
    default:
    {
        logMessage(LMI, "ERROR", "Import", "Expected a string literal after the `import` keyword");
        break;
    }
    }

    DEBUG_BREAKPOINT;
}

const char *resolveRelativePath(const char *currentDir, const char *relativePath)
{
    // Sanitize the relative path by removing surrounding quotes if present
    char sanitizedPath[PATH_MAX];
    size_t len = strlen(relativePath);

    if (relativePath[0] == '"' && relativePath[len - 1] == '"')
    {
        snprintf(sanitizedPath, sizeof(sanitizedPath), "%.*s", (int)(len - 2), relativePath + 1);
    }
    else
    {
        snprintf(sanitizedPath, sizeof(sanitizedPath), "%s", relativePath);
    }

    // Buffer to hold the combined path
    char combinedPath[PATH_MAX];

    // Combine the current directory with the sanitized relative path
    snprintf(combinedPath, sizeof(combinedPath), "%s/%s", currentDir, sanitizedPath);

    // Buffer to hold the normalized absolute path
    char *absolutePath = realpath(combinedPath, NULL);
    if (!absolutePath)
    {
        logMessage(LMI, "ERROR", "Import", "Failed to resolve path: %s", combinedPath);
        return NULL;
    }

    return absolutePath; // Caller must free this memory
}

ASTNode *handleRelativeImport(const char *modulePath,
                              Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    logMessage(LMI, "INFO", "Import", "Handling relative import");
    printf("Module Path: %s\n", modulePath);

    const char *currentFile = state->getFilePath(state);
    logMessage(LMI, "INFO", "Import", "Current File: %s", currentFile);
    char *currentFileCpy = strdup(currentFile);
    if (!currentFileCpy)
    {
        logMessage(LMI, "ERROR", "Import", "Failed to copy current file path");
        CONDITION_FAILED;
    }

    const char *currentFileDir = fs->getDirectoryPath(currentFile);
    logMessage(LMI, "INFO", "Import", "Current File Directory: %s", currentFileDir);

    // Resolve the relative path
    const char *resolvedPath = resolveRelativePath(currentFileDir, modulePath);
    if (resolvedPath)
    {
        logMessage(LMI, "INFO", "Import", "Resolved Path: %s", resolvedPath);
    }
    else
    {
        logMessage(LMI, "ERROR", "Import", "Failed to resolve module path: %s", modulePath);
        CONDITION_FAILED;
    }

    // Now that we have the resolved path, we can parse the module
    // and get the AST tree for the module.

    ASTNode *moduleNode = compileForASTNode(resolvedPath, state);
    if (!moduleNode)
    {
        logMessage(LMI, "ERROR", "Import", "Failed to compile module: %s", resolvedPath);
        CONDITION_FAILED;
    }

    DEBUG_PRINT_FILTER({
        DTM->symbolTable->printTable(DTM->symbolTable);
        moduleNode->print(moduleNode);
    });

    ASTNode *programNodePtr = context->programNodePtr;
    if (!programNodePtr)
    {
        logMessage(LMI, "ERROR", "Import", "Program node is NULL");
        CONDITION_FAILED;
    }

    programNodePtr->data.program->importAST(programNodePtr, moduleNode);
    programNodePtr->print(programNodePtr);

    // Set the context back to the original file
    state->setFilePath(state, strdup(currentFileCpy));
    logMessage(LMI, "INFO", "Import", "Set file path back to: %s", currentFileCpy);

    return createDiscardNode(arena, state, lexer);
}
