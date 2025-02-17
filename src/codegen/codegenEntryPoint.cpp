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
#include "codegen/oldCodeGen.hpp"
#include "tools/logger/logger_config.h"

int generateIRFromAST(CompilationUnit *unit,
                      CompilerState *state, CryoLinker *cLinker, CryoGlobalSymbolTable *globalTable)
{
    DEBUG_PRINT_FILTER({
        std::cout << "\n";
        std::cout << "<!> ======================= @generateIRFromAST ======================= <!>" << std::endl;
    });

    if (!unit->isVerified)
    {
        logMessage(LMI, "ERROR", "Compiler", "CompilationUnit is not verified");
        return 1;
    }

    Cryo::CryoCompiler compiler;
    compiler.setCompilerState(state);
    compiler.setCompilerSettings(state->settings);

    bool isCompilingMain = unit->type == CRYO_MAIN;
    if (isCompilingMain)
    {
        compiler.preInitMain();
    }

    ASTNode *rootNode = unit->ast;
    if (!rootNode)
    {
        logMessage(LMI, "ERROR", "Compiler", "ASTNode is null");
        return 1;
    }
    rootNode->print(rootNode);

    std::string moduleName = unit->dir.src_fileName;
    compiler.setModuleIdentifier(moduleName);

    // Compile the ASTNode
    compiler.compile(rootNode);

    logMessage(LMI, "INFO", "Compiler", "ASTNode compiled successfully");

    // Generate the IR code
    llvm::Module *mod = compiler.getContext().module.get();
    compiler.getLinker()->compileModule(unit, mod);

    logMessage(LMI, "INFO", "Compiler", "CompilationUnit compiled successfully");

    return 0;
}

namespace Cryo
{

} // namespace Cryo
