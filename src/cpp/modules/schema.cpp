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
#include "cpp/codegen.h"

namespace Cryo
{

    /*
    Refactor for this file, notes:
    In the logs, it seems to be defining function declarations more than once.
    There seems to be an overlap or multiple calls somehow to prototyping.

    The idea for the refactor, loop over the AST Tree and find the function nodes (extern or normal).
    */

    bool CryoModules::declareFunctions(ASTNode *node)
    {
        if (!node)
        {
            std::cerr << "Error: node is null." << std::endl;
            return false;
        }

        CryoSyntax &cryoSyntaxInstance = compiler.getSyntax();
        bool mainFunctionExists = false;
        std::unordered_set<std::string> declaredFunctions;

        for (int i = 0; i < node->data.program.stmtCount; ++i)
        {
            ASTNode *statement = node->data.program.statements[i];
            if (!statement)
            {
                std::cerr << "Error: statement is null at index " << i << "." << std::endl;
                continue;
            }

            switch (statement->type)
            {
            case NODE_FUNCTION_DECLARATION:
            {
                FunctionDeclNode *functionNode = statement->data.functionDecl.function;
                if (functionNode)
                {
                    if (functionNode->name == "main")
                    {
                        std::cout << "[Schema] `main` Function Found, not generating prototype." << std::endl;
                        mainFunctionExists = true;
                        continue;
                    }
                    else if (declaredFunctions.find(functionNode->name) == declaredFunctions.end())
                    {
                        cryoSyntaxInstance.generateFunctionPrototype(statement);
                        declaredFunctions.insert(functionNode->name);
                    }
                }
                break;
            }

            case NODE_EXTERN_FUNCTION:
            {
                FunctionDeclNode *externFunctionNode = statement->data.externNode.decl.function;
                if (externFunctionNode && declaredFunctions.find(externFunctionNode->name) == declaredFunctions.end())
                {
                    cryoSyntaxInstance.generateExternalPrototype(statement);
                    declaredFunctions.insert(externFunctionNode->name);
                }
                else
                {
                    std::cerr << "Error: extern function node is null or already declared." << std::endl;
                }
                break;
            }

            default:
                std::cout << "[Schema] Non-Function Declaration in @declareFunctions. Skipping..." << std::endl;
                break;
            }
        }

        std::cout << "[Schema] Function Declarations Complete." << std::endl;
        return mainFunctionExists;
    }

} // namespace Cryo
