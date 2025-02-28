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
#include "codegen_rewrite/codegen.hpp"

int UNFINISHED_generateIRFromAST(CompilationUnit *unit, CompilerState *state,
                                 CryoLinker *cLinker, CryoGlobalSymbolTable *globalTable)
{
    if (!unit->isVerified)
    {
        logMessage(LMI, "ERROR", "Compiler", "CompilationUnit is not verified");
        return 1;
    }

    logMessage(LMI, "INFO", "Compiler", "[new_codegen] Generating IR from AST...");

    // Initialize the context
    Cryo::CodegenContext &context = Cryo::CodegenContext::getInstance();
    context.initializeCodegenContext();
    context.setModuleIdentifier(unit->dir.src_fileName);

    bool isCompilingMain = unit->type == CRYO_MAIN;
    if (isCompilingMain)
    {
        logMessage(LMI, "INFO", "Compiler", "Compiling main file...");
        context.preInitMain();
    }

    logMessage(LMI, "INFO", "Compiler", "Generating IR from AST...");
    // Create IR Generator and Visitor
    Cryo::IRGeneration irGen(context);
    irGen.setBuildDir(unit->dir.out_filePath);

    // Begin code generation
    try
    {
        // [1]: Step 1. Generate IR from the AST
        logMessage(LMI, "INFO", "Compiler", "Generating IR...");
        irGen.generateIR(unit->ast);
        return 0;
    }
    catch (const std::exception &e)
    {
        logMessage(LMI, "ERROR", "Compiler", e.what());
        return 1;
    }
}
