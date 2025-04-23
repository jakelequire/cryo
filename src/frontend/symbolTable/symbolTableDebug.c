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
#include "frontend/frontendSymbolTable.h"
#include "tools/logger/logger_config.h"
#include "diagnostics/diagnostics.h"

// Helper function to convert ScopeType enum to string
const char *getScopeTypeString(ScopeType type)
{
    switch (type)
    {
    case SCOPE_GLOBAL:
        return "GLOBAL";
    case SCOPE_NAMESPACE:
        return "NAMESPACE";
    case SCOPE_FUNCTION:
        return "FUNCTION";
    case SCOPE_BLOCK:
        return "BLOCK";
    case SCOPE_CLASS:
        return "CLASS";
    case SCOPE_STRUCT:
        return "STRUCT";
    case SCOPE_UNKNOWN:
        return "UNKNOWN";
    default:
        return "INVALID";
    }
}

// Helper function to print indentation based on depth
static void printIndent(size_t depth)
{
    for (size_t i = 0; i < depth; i++)
    {
        printf("  ");
    }
}

// Print details of a single symbol
static void printSymbol(FrontendSymbol *symbol, size_t depth)
{
    if (!symbol)
    {
        printf("(NULL SYMBOL)\n");
        return;
    }

    // Assuming the FrontendSymbol structure has at least these fields
    // Adjust according to your actual structure
    printIndent(depth);
    const char *dataTypeString = symbol->type->debug->toString(symbol->type);
    printf("|- Symbol: %s | %s\n",
           symbol->name ? symbol->name : "(unnamed)",
           dataTypeString ? dataTypeString : "(invalid type)");

    // Add more details as needed based on your FrontendSymbol structure
}

// Print details of a single scope
static void printScope(FrontendScope *scope, size_t depth)
{
    if (!scope)
    {
        printf("(NULL SCOPE)\n");
        return;
    }

    // Print scope header with safer access
    printIndent(depth);
    printf("+- Scope: %s (ID: %s, Type: %s, Depth: %zu)\n",
           scope->name ? scope->name : "(unnamed)",
           scope->id ? scope->id : "(no ID)",
           scope->type >= SCOPE_GLOBAL && scope->type <= SCOPE_UNKNOWN ? getScopeTypeString(scope->type) : "INVALID",
           scope->depth);

    // Print symbols in this scope - check if symbols array exists first
    printIndent(depth);
    printf("   |-- Symbols (%zu/%zu):\n",
           scope->symbolCount,
           scope->symbolCapacity);

    if (scope->symbolCount == 0 || !scope->symbols)
    {
        printIndent(depth);
        printf("   |   (No symbols in scope)\n");
    }
    else
    {
        for (size_t i = 0; i < scope->symbolCount; i++)
        {
            printIndent(depth);
            printf("   |   ");
            if (scope->symbols[i] == NULL)
            {
                printf("(NULL SYMBOL)\n");
                continue;
            }
            printSymbol(scope->symbols[i], 0);
        }
    }
}

void FrontendSymbolTable_printTable(FrontendSymbolTable *self)
{
    if (!self)
    {
        printf("Cannot print NULL symbol table\n");
        DEBUG_BREAKPOINT;
        return;
    }

    printf("\n=================================================================\n");
    printf("FRONTEND SYMBOL TABLE DUMP\n");
    printf("=================================================================\n");

    // Overall statistics
    printf("Total symbols: %zu/%zu\n", self->symbolCount, self->symbolCapacity);
    printf("Total scopes: %zu/%zu\n", self->scopeCount, self->scopeCapacity);
    printf("Current namespace: %s\n",
           self->currentNamespace ? self->currentNamespace : "(global)");
    printf("Scope stack depth: %zu/%zu\n", self->scopeStackSize, SCOPE_STACK_SIZE);

    // Current scope info
    printf("\nCURRENT SCOPE:\n");
    printf("-----------------------------------------------------------------\n");
    if (self->currentScope)
    {
        printScope(self->currentScope, 0);
    }
    else
    {
        printf("(No current scope)\n");
    }

    // Scope stack (nested scopes)
    printf("\nSCOPE STACK (most recent first):\n");
    printf("-----------------------------------------------------------------\n");
    if (self->scopeCount == 0)
    {
        printf("(Empty scope stack)\n");
    }
    else
    {
        for (size_t i = 0; i < self->scopeStackSize; i++)
        {
            // Print scope header with safer access
            printIndent(i);
            printf("Scope[%zu]: ", i);
            if (self->scopeStack[i] == NULL)
            {
                printf("(NULL SCOPE)\n");
                continue;
            }
            printScope(self->scopeStack[i], 0);
        }
    }

    // All scopes
    printf("\nALL SCOPES:\n");
    printf("-----------------------------------------------------------------\n");
    if (self->scopeCount == 0)
    {
        printf("(No scopes defined)\n");
    }
    else
    {
        for (size_t i = 0; i < self->scopeCount; i++)
        {
            printf("Scope[%zu] ", i);
            printScope(self->scopes[i], 1);
            printf("\n");
        }
    }

    printf("=================================================================\n");
    printf("END OF SYMBOL TABLE DUMP\n");
    printf("=================================================================\n\n");
}