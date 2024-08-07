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

namespace Cryo {

/**
 * The public constructor for the CodeGen class. This is the main entry point to the program.
 * The frontend passes the parsed AST Tree and starts the process for code generation.
 */
CodeGen::CodeGen(ASTNode* root) : 
context(), builder(context), module(std::make_unique<llvm::Module>("CryoModule", context)) 
{
    namedValues = std::unordered_map<std::string, llvm::Value*>();
    executeCodeGeneration(root);
}


/**
 * The Entry Point to the generation process. 
 */
void CodeGen::executeCodeGeneration(ASTNode* root) {
    std::cout << "\nStarting Code Generation...\n";

    // First Pass: Declare all functions
    std::cout << "\nFirst Pass: Declaring all functions\n";
    bool mainFunctionExists = CryoModules->declareFunctions(root);
    
    if (!mainFunctionExists) {
        std::cout << "[CPP] Main function not found, creating default main function\n";
        CryoSyntax->createDefaultMainFunction();
    }

    // Second Pass: Generate code for the entire program
    std::cout << "\nSecond Pass: Generate code for the entire program\n";
    identifyNodeExpression(root);

    llvm::Function* defaultMain = module->getFunction("_defaulted");
    if (defaultMain) {
        llvm::BasicBlock& entryBlock = defaultMain->getEntryBlock();
        if (!entryBlock.getTerminator()) {
            builder.SetInsertPoint(&entryBlock);
            builder.CreateRetVoid();
            std::cout << "[CPP] Added return statement to default main function in entry block\n";
        }
    }

        if (llvm::verifyModule(*module, &llvm::errs())) {
        std::cout << "\n>===------- Error: LLVM module verification failed -------===<\n";
        module->print(llvm::errs(), nullptr);
        std::cout << "\n>===----------------- End Error -----------------===<\n";
        std::cerr << "Error: LLVM module verification failed\n";
        exit(1);
    } else {
        std::error_code EC;
        llvm::raw_fd_ostream dest("output.ll", EC, llvm::sys::fs::OF_None);

        if (EC) {
            std::cerr << "Could not open file: " << EC.message() << std::endl;
        } else {
            std::cout << "\n>===------- LLVM IR Code -------===<\n" << std::endl;
            module->print(dest, nullptr);
            module->print(llvm::outs(), nullptr);
            std::cout << "\n>===------- End IR Code ------===<\n" << std::endl;
            std::cout << "LLVM IR written to output.ll" << std::endl;
        }
    }

    std::cout << "\nCode generation complete\n";
} // <executeCodeGeneration>



/**
 * Identify the type of node and call the appropriate function to generate code.
 */
void CodeGen::identifyNodeExpression(ASTNode* node) {
    if(!node) {
        std::cerr << "[CodeGen] Error: Code Generation Failed, ASTNode is null. @<identifyExpression>";
        exit(0);
        return;
    }
    std::cout << "[CodeGen] Starting Code Generation...\n";

    switch(node->type) {
        case NODE_PROGRAM: {
            std::cout << "[CodeGen] Identified NODE_PROGRAM.\n";
            CryoModules->generateProgram(node);
        }
        
        case NODE_FUNCTION_DECLARATION: {
            std::cout << "[CodeGen] Identified NODE_FUNCTION_DECLARATION.\n";
            CryoSyntax->generateFunction(node);
        }
        
        case NODE_VAR_DECLARATION: {
            std::cout << "[CodeGen] Identified NODE_VAR_DECLARATION.\n";
            CryoSyntax->generateVarDeclaration(node);
        }
        
        case NODE_STATEMENT: {
            std::cout << "[CodeGen] Identified NODE_STATEMENT.\n";
            CryoSyntax->generateStatement(node);
        }
        
        case NODE_EXPRESSION: {
            std::cout << "[CodeGen] Identified NODE_EXPRESSION.\n";
            CryoSyntax->generateExpression(node);
        }
        
        case NODE_BINARY_EXPR: {
            std::cout << "[CodeGen] Identified NODE_BINARY_EXPR.\n";
            CryoSyntax->generateBinaryOperation(node);
        }
        
        case NODE_UNARY_EXPR: {
            std::cout << "[CodeGen] Identified NODE_UNARY_EXPR.\n";
            // Todo
        }
        
        case NODE_LITERAL_EXPR: {
            std::cout << "[CodeGen] Identified NODE_LITERAL_EXPR.\n";
            // Todo
        }
        
        case NODE_VAR_NAME: {
            std::cout << "[CodeGen] Identified NODE_VAR_NAME.\n";
            // Todo
        }
        
        case NODE_FUNCTION_CALL: {
            std::cout << "[CodeGen] Identified NODE_FUNCTION_CALL.\n";
            CryoSyntax->generateFunctionCall(node);
        }
        
        case NODE_IF_STATEMENT: {
            std::cout << "[CodeGen] Identified NODE_IF_STATEMENT.\n";
            // Todo
        }
        
        case NODE_WHILE_STATEMENT: {
            std::cout << "[CodeGen] Identified NODE_WHILE_STATEMENT.\n";
            // Todo
        }
        
        case NODE_FOR_STATEMENT: {
            std::cout << "[CodeGen] Identified NODE_FOR_STATEMENT.\n";
            // Todo
        }
        
        case NODE_RETURN_STATEMENT: {
            std::cout << "[CodeGen] Identified NODE_RETURN_STATEMENT.\n";
            CryoSyntax->generateReturnStatement(node);
        }
        
        case NODE_BLOCK: {
            std::cout << "[CodeGen] Identified NODE_BLOCK.\n";
            CryoModules->generateBlock(node);
        }
        
        case NODE_FUNCTION_BLOCK: {
            std::cout << "[CodeGen] Identified NODE_FUNCTION_BLOCK.\n";
            CryoSyntax->generateFunctionBlock(node);
        }
        
        case NODE_EXPRESSION_STATEMENT: {
            std::cout << "[CodeGen] Identified NODE_EXPRESSION_STATEMENT.\n";
            CryoSyntax->generateExpression(node->data.stmt.stmt);
        }
        
        case NODE_ASSIGN: {
            std::cout << "[CodeGen] Identified NODE_ASSIGN.\n";
            // Todo
        }
        
        case NODE_PARAM_LIST: {
            std::cout << "[CodeGen] Identified NODE_PARAM_LIST.\n";
            // Todo
        }
        
        case NODE_TYPE: {
            std::cout << "[CodeGen] Identified NODE_TYPE.\n";
            // Todo
        }
        
        case NODE_STRING_LITERAL: {
            std::cout << "[CodeGen] Identified NODE_STRING_LITERAL.\n";
            // Todo
        }
        
        case NODE_STRING_EXPRESSION: {
            std::cout << "[CodeGen] Identified NODE_STRING_EXPRESSION.\n";
            // Todo
        }
        
        case NODE_BOOLEAN_LITERAL: {
            std::cout << "[CodeGen] Identified NODE_BOOLEAN_LITERAL.\n";
            // Todo
        }
        
        case NODE_ARRAY_LITERAL: {
            std::cout << "[CodeGen] Identified NODE_ARRAY_LITERAL.\n";
            CryoSyntax->generateArrayLiteral(node);
        }
        
        case NODE_IMPORT_STATEMENT: {
            std::cout << "[CodeGen] Identified NODE_IMPORT_STATEMENT.\n";
            // Todo
        }
        
        case NODE_EXTERN_STATEMENT: {
            std::cout << "[CodeGen] Identified NODE_EXTERN_STATEMENT.\n";
            // Todo
        }
        
        case NODE_EXTERN_FUNCTION: {
            std::cout << "[CodeGen] Identified NODE_EXTERN_FUNCTION.\n";
            // Todo
        }
        
        case NODE_ARG_LIST: {
            std::cout << "[CodeGen] Identified NODE_ARG_LIST.\n";
            // Todo
        }
        
        case NODE_UNKNOWN: {
            std::cerr << "[CodeGen] Error: `NODE_UNKNOWN` found. @<identifyNodeExpression>.\n";
            break;
        }

        default: {
            std::cerr << "[CodeGen] Error: Unknown Node Type. @<identifyNodeExpression>\n";
            break;
        }
    }
    return;
} // <identifyNodeExpression>



} // namespace Cryo
