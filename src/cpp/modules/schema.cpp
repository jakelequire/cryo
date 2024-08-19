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
        CryoDebugger &cryoDebugger = compiler.getDebugger();
        if (!node)
        {
            cryoDebugger.logMessage("ERROR", __LINE__, "Schema", "Node is null in declareFunctions");
            return false;
        }

        CryoSyntax &cryoSyntaxInstance = compiler.getSyntax();
        bool mainFunctionExists = false;
        std::unordered_set<std::string> declaredFunctions;

        for (int i = 0; i < node->data.program->statementCount; ++i)
        {
            ASTNode *statement = node->data.program->statements[i];
            if (!statement)
            {
                cryoDebugger.logMessage("ERROR", __LINE__, "Schema", "Statement is null in declareFunctions");
                continue;
            }

            switch (statement->metaData->type)
            {
            case NODE_FUNCTION_DECLARATION:
            {
                FunctionDeclNode *functionNode = statement->data.functionDecl;
                if (functionNode)
                {
                    if (strcmp(strdup(functionNode->name), "main") == 0)
                    {
                        cryoDebugger.logMessage("INFO", __LINE__, "Schema", "Main function exists");
                        mainFunctionExists = true;
                        continue;
                    }
                    else if (declaredFunctions.find(functionNode->name) == declaredFunctions.end())
                    {
                        cryoSyntaxInstance.generateFunctionPrototype(statement);
                        declaredFunctions.insert(functionNode->name);
                        cryoDebugger.logMessage("INFO", __LINE__, "Schema", "Function declared: " + std::string(functionNode->name));
                    }
                }
                break;
            }

            case NODE_EXTERN_FUNCTION:
            {
                ExternFunctionNode *externFunctionNode = statement->data.externFunction;
                char *externFunctionName = strdup(externFunctionNode->name);
                if (externFunctionNode && declaredFunctions.find(externFunctionName) == declaredFunctions.end())
                {
                    cryoSyntaxInstance.generateExternalPrototype(statement);
                    declaredFunctions.insert(externFunctionNode->name);
                    cryoDebugger.logMessage("INFO", __LINE__, "Schema", "Extern function declared: " + std::string(externFunctionNode->name));
                }
                else
                {
                    cryoDebugger.logMessage("ERROR", __LINE__, "Schema", "Extern function already declared: " + std::string(externFunctionNode->name));
                }
                break;
            }

            default:
                cryoDebugger.logMessage("ERROR", __LINE__, "Schema", "Unsupported node type in declareFunctions");
                break;
            }
        }

        cryoDebugger.logMessage("INFO", __LINE__, "Schema", "Function declaration complete");
        return mainFunctionExists;
    }

} // namespace Cryo
