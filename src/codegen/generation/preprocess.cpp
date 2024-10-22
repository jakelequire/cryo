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
#include "codegen/generation/codegen.hpp"

namespace Cryo
{
    /// @private
    int CodeGen::preprocess(ASTNode *root)
    {
        DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Preprocessing Code Generation");

        // Get / Set the symbol table for the module and its state
        std::string namespaceName = getNamespaceFromAST(root);
        getIRSymTable().initModule(root, namespaceName);
        currentNamespace = namespaceName;

        DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Linting Tree");

        bool validateTree = DevDebugger::lintTree(root);
        if (validateTree == false)
        {
            std::cerr << "[CPP] Tree is invalid!" << std::endl;
            CONDITION_FAILED;
        }

        // Declare all functions in the AST tree
        // declarations.preprocessDeclare(root); <- TODO: Implement this function
        DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Preprocessing Complete");
        return 0;
    }

}