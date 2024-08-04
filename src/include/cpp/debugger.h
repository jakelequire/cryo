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
#ifndef CRYO_DEBUGGER
#define CRYO_DEBUGGER
#include "codegen.h"

namespace Cryo
{
    class CryoContext;

#define VALIDATE_ASTNODE(node) checkNode(node)
#define DEBUG_PANIC debugPanic()
#define DEBUG_ASSERT nullptr // Temp value

    class CryoDebugger
    {
    public:
        CryoDebugger(CryoContext &context) : context(context) {}

        void logNode(ASTNode *node);
        // Macro Implementations
        void checkNode(ASTNode *node);
        void debugPanic(std::string funcName);

    private:
        CryoContext &context;
        bool isNodeTypeValid(ASTNode *node);

    protected:
    };

}

#endif // CRYO_DEBUGGER