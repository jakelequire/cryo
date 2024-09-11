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
#ifndef BACKEND_SYMTABLE_H
#define BACKEND_SYMTABLE_H
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "cpp/codegen.h"
#include "cpp/debugger.h"
#include "compiler/ast.h"

namespace Cryo
{
    class BackendSymTable;
    class CryoCompiler;
    class CryoContext;
    class CryoDebugger;

    // -----------------------------------------------------------------------------------------------

    // For each file, we will have a SymTable that contains all the variables and functions
    typedef struct SymTableNode
    {
        std::string namespaceName;
        std::unordered_map<std::string, CryoVariableNode> variables;
        std::unordered_map<std::string, FunctionDeclNode> functions;
        std::unordered_map<std::string, ExternFunctionNode> externFunctions;
    } SymTableNode;

    // This will contain all the namespaces for the entire program
    typedef struct SymTable
    {
        std::unordered_map<std::string, SymTableNode> namespaces;
    } SymTable;

    // -----------------------------------------------------------------------------------------------

    class BackendSymTable
    {
    public:
        BackendSymTable() {}
        ~BackendSymTable() {}

        // Properties
        SymTable symTable;
        CryoDebugger &getDebugger() { return *debugger; }

        // Prototypes
        void initSymTable(void);
        void initModule(ASTNode *root, std::string namespaceName);
        SymTableNode getSymNode(std::string namespaceName);
        void printTable(std::string namespaceName);

    private:
        CryoDebugger *debugger;
        SymTableNode traverseModule(ASTNode *root, std::string namespaceName);
    };
}
#endif // BACKEND_SYMTABLE_H
