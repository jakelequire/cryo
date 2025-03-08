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
#include "tools/logger/logger_config.h"
#include "diagnostics/diagnostics.h"
#include <dirent.h>

/*
The import keyword can have the following syntax:
```
import "./module.cryo" // Import relative to the current file
*/
ASTNode *handleImportParsing(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
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

        return handleRelativeImport(modulePath, lexer, context, arena, state, globalTable);
    }
    default:
    {
        logMessage(LMI, "ERROR", "Import", "Expected a string literal after the `import` keyword");
        break;
    }
    }

    DEBUG_BREAKPOINT;
}

ASTNode *handleRelativeImport(const char *modulePath,
                              Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    logMessage(LMI, "INFO", "Import", "Handling relative import");
    printf("Module Path: %s\n", modulePath);

    // This is the current file being parsed and the relative path to the imported module file
    const char *currentFile = state->getFilePath(state); 
    logMessage(LMI, "INFO", "Import", "Current File: %s", currentFile); 

    DEBUG_BREAKPOINT;
}
