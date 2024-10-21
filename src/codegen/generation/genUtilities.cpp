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
#include "codegen/generation/genUtilities.hpp"

namespace Cryo
{

    std::string GenUtilities::getNamespaceFromAST(ASTNode *node)
    {
        DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Getting Namespace");

        std::string namespaceName = "";

        int count = node->data.program->statementCount;
        for (int i = 0; i < count; i++)
        {
            CryoNodeType nodeType = node->data.program->statements[i]->metaData->type;
            if (nodeType == NODE_NAMESPACE)
            {
                namespaceName = node->data.program->statements[i]->data.cryoNamespace->name;
                break;
            }
        }
        std::cout << "Namespace: " << namespaceName << std::endl;

        return namespaceName;
    }

    std::string GenUtilities::formatString(std::string str)
    {
        std::string formattedString = "";
        for (int i = 0; i < str.length(); i++)
        {
            if (str[i] == '\\')
            {
                if (str[i + 1] == 'n')
                {
                    formattedString += '\n';
                    i++;
                }
                else if (str[i + 1] == 't')
                {
                    formattedString += '\t';
                    i++;
                }
                else if (str[i + 1] == 'r')
                {
                    formattedString += '\r';
                    i++;
                }
                else if (str[i + 1] == '0')
                {
                    formattedString += '\0';
                    i++;
                }
                else
                {
                    formattedString += str[i];
                }
            }
            else
            {
                formattedString += str[i];
            }
        }

        return formattedString;
    }

} // namespace Cryo
