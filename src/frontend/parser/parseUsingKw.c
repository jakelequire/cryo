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
void parseUsingKeyword(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable, CryoGlobalSymbolTable *globalTable)
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

// Store module chain
#define MAX_MODULE_CHAIN 32
    struct
    {
        char *name;
        size_t length;
    } moduleChain[MAX_MODULE_CHAIN];
    size_t chainLength = 0;

    // Parse the chain of scope/function identifiers
    while (true)
    {
        Token current = lexer->currentToken;

        // Add current identifier to chain
        moduleChain[chainLength].name = strndup(current.start, current.length);
        moduleChain[chainLength].length = current.length;
        chainLength++;

        if (chainLength >= MAX_MODULE_CHAIN)
        {
            logMessage("ERROR", __LINE__, "Parser", "Module chain exceeds maximum length");
            // Clean up allocated memory
            for (size_t i = 0; i < chainLength; i++)
            {
                free(moduleChain[i].name);
            }
            free(primaryModule);
            return;
        }

        consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.",
                "parseUsingKeyword", table, arena, state, typeTable, context);

        // Check if chain continues
        if (lexer->currentToken.type != TOKEN_DOUBLE_COLON)
        {
            break;
        }

        consume(__LINE__, lexer, TOKEN_DOUBLE_COLON, "Expected `::` after identifier.",
                "parseUsingKeyword", table, arena, state, typeTable, context);
    }

    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected `;` to end using statement.",
            "parseUsingKeyword", table, arena, state, typeTable, context);

    // Import the module chain
    importUsingModule(primaryModule, moduleChain, chainLength);

    free(primaryModule);
}

void importUsingModule(const char *primaryModule, const char *moduleChain[], size_t moduleCount)
{
    // Import the module and scope or function
    // This will allow the module to be used in the current scope.
    DEBUG_BREAKPOINT;
}
