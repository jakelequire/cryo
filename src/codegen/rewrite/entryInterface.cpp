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
#include "codegen/rewrite/codegen.hpp"

int DONOTUSEYET_generateIRFromAST(CompilationUnit *unit, CompilerState *state,
                                  CryoLinker *cLinker, CryoGlobalSymbolTable *globalTable)
{
    if (!unit->isVerified)
    {
        logMessage(LMI, "ERROR", "Compiler", "CompilationUnit is not verified");
        return 1;
    }

    bool isCompilingMain = unit->type == CRYO_MAIN;
    if (isCompilingMain)
    {
        // compiler.preInitMain();
    }

    // Initialize the context
    Cryo::CodegenContext &context = Cryo::CodegenContext::getInstance();
    // context.initializeContext();
    // context.setModuleIdentifier(unit->dir.src_fileName);

    // Initialize symbol table with the current module
    // context.symbolTable = std::make_unique<IRSymbolTable>(context.module.get());
    // context.symbolTable->pushScope(); // Push global scope

    // Create IR Generator and Visitor
    Cryo::IRGeneration irGen(context);
    Cryo::CodeGenVisitor visitor;

    // Begin code generation
    try
    {
        irGen.generateIR(unit->ast);
        return 0;
    }
    catch (const std::exception &e)
    {
        logMessage(LMI, "ERROR", "Compiler", e.what());
        return 1;
    }
}

/*
// The new code generation process will need to emulate the old code generation process.
// The old code generation process is as follows:

    Cryo::CryoCompiler compiler;
    compiler.setCompilerState(state);
    compiler.setCompilerSettings(state->settings);



    ASTNode *rootNode = unit->ast;
    if (!rootNode)
    {
        logMessage(LMI, "ERROR", "Compiler", "ASTNode is null");
        return 1;
    }
    rootNode->print(rootNode);

    std::string moduleName = unit->dir.src_fileName;
    compiler.setModuleIdentifier(moduleName);

    // Initialize the symbol table
    compiler.getContext().initializeSymbolTable();

    // Compile the ASTNode
    compiler.compile(rootNode);

    logMessage(LMI, "INFO", "Compiler", "ASTNode compiled successfully");

    // Generate the IR code
    llvm::Module *mod = compiler.getContext().module.get();
    compiler.getLinker()->compileModule(unit, mod);

    logMessage(LMI, "INFO", "Compiler", "CompilationUnit compiled successfully");

*/
