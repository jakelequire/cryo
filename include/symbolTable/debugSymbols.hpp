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
#ifndef DEBUG_SYMBOLS_H
#define DEBUG_SYMBOLS_H

#ifdef __cplusplus
extern "C"
{
#endif

    // Opaque pointer type for C
    typedef struct CryoSymbolTableDebugger_t *CryoSymbolTableDebugger;

#ifdef __cplusplus
} // C API ----------------------------------------------------------

#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <functional>

#include "symbolTable/symdefs.h"
#include "tools/cxx/logger.hpp"

namespace Cryo
{

    class SymbolTableDebugger
    {
    public:
        static void logScopeBlock(ScopeBlock *block);
        static void logVariableSymbol(VariableSymbol *symbol);
        static void logFunctionSymbol(FunctionSymbol *symbol);
        static void logExternSymbol(ExternSymbol *symbol);
        static void logTypeSymbol(TypeSymbol *symbol);
        static void logPropertySymbol(PropertySymbol *symbol);
        static void logMethodSymbol(MethodSymbol *symbol);

        static void logSymbol(Symbol *symbol);
        static void logSymbolTable(SymbolTable *table);
        static void logTypeTable(TypesTable *table);

        static const char *getTypeofSymbol(TypeOfSymbol symbolType);
        static void logTypeofSymbol(TypeOfSymbol symbolType);

    private:
        static Logger *logger;

        static constexpr const char *ANSI_RESET = "\033[0m";
        static constexpr const char *ANSI_BOLD = "\033[1m";

        // Colors for different symbol types
        struct SymbolColors
        {
            static const char *VARIABLE() { return "\033[38;5;45m"; }  // Blue
            static const char *FUNCTION() { return "\033[38;5;214m"; } // Orange
            static const char *EXTERN() { return "\033[38;5;147m"; }   // Purple
            static const char *TYPE() { return "\033[38;5;84m"; }      // Green
            static const char *PROPERTY() { return "\033[38;5;219m"; } // Pink
            static const char *METHOD() { return "\033[38;5;203m"; }   // Red
            static const char *SCOPE() { return "\033[38;5;226m"; }    // Yellow
            static const char *TABLE() { return "\033[38;5;123m"; }    // Cyan
        };

        static std::string getColoredSymbolType(TypeOfSymbol symbolType);
        static std::string getColoredTableType(const char *tableType);
        static std::string getColoredScopeType();
    };
}

#endif // C API -----------------------------------------------------
#endif // DEBUG_SYMBOLS_H
