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
#include "codegen/instances.hpp"

namespace Cryo
{
    // Create an instance of a class or struct
    llvm::Value *Instance::createInstance(ASTNode *node)
    {
        ASSERT_NODE_NULLPTR_RET(node);

        // Check if the node is a struct or class
        if (node->metaData->type == NODE_STRUCT_DECLARATION)
        {
            return createStructInstance(node);
        }

        // If the node is not a struct or class, return nullptr
        return nullptr;
    }

    // Create an instance of a struct
    llvm::Value *Instance::createStructInstance(ASTNode *node)
    {
        ASSERT_NODE_NULLPTR_RET(node);

        // Get the struct type
        llvm::StructType *structType = llvm::StructType::create(context.context, node->data.structNode->name);

        // Create an instance of the struct
        llvm::Value *instance = new llvm::AllocaInst(structType, 0, node->data.structNode->name, context.builder.GetInsertBlock());

        return instance;
    }

} // namespace Cryo
