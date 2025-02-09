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
#include "linker/linker.hpp"
#include "diagnostics/diagnostics.h"

CompilationUnit *CompilationUnit_Create(CompilationUnitType type, CompilationUnitDir dir)
{
    CompilationUnit *unit = (CompilationUnit *)malloc(sizeof(CompilationUnit));
    if (!unit)
    {
        logMessage(LMI, "ERROR", "Linker", "Failed to allocate memory for CompilationUnit");
        return nullptr;
    }

    unit->type = type;
    unit->dir = dir;
    unit->ast = nullptr;
    unit->isASTSet = false;
    unit->module = nullptr;
    unit->isModuleSet = false;

    unit->setAST = CompilationUnit_SetAST;
    unit->setModule = CompilationUnit_SetModule;

    return unit;
}
void CompilationUnit_SetAST(CompilationUnit *self, ASTNode *ast)
{
    if (!self)
    {
        logMessage(LMI, "ERROR", "Linker", "CompilationUnit is null");
        return;
    }

    self->ast = ast;
    self->isASTSet = true;
    return;
}
void CompilationUnit_SetModule(CompilationUnit *self, llvm::Module *module)
{
    if (!self)
    {
        logMessage(LMI, "ERROR", "Linker", "CompilationUnit is null");
        return;
    }

    self->module = module;
    self->isModuleSet = true;
    return;
}

CompilationUnitDir createCompilationUnitDir(const char *inputFile, CompilationUnitType type)
{
    CompilationUnitDir dir;
}

namespace Cryo
{

} // namespace Cryo
