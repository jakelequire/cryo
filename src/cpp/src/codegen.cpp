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

/*
To compile the generated LLVM IR code to an object file, run the following commands:
llvm-as ./output.ll -o ./output.bc
llc -filetype=obj ./output.bc -o ./output.o
*/

std::unordered_map<std::string, llvm::Value*> namedValues;


// <codegen> 
void codegen(ASTNode* root) {
    llvm::LLVMContext context;
    llvm::IRBuilder<> builder(context);
    std::unique_ptr<llvm::Module> module = std::make_unique<llvm::Module>("CryoModule", context);
    // create non function block
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "entry", module->getFunction("main"));
    builder.SetInsertPoint(entry);

    generateCode(root, builder, *module);

    if (llvm::verifyModule(*module, &llvm::errs())) {
        module->print(llvm::outs(), nullptr);
        std::cerr << "Error: LLVM module verification failed\n";
    } else {
        // Output the generated LLVM IR code to a .ll file
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
}
// </codegen>


// <generateCode>
void generateCode(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::cout << "[CPP] Generating code for AST node\n" << std::endl;
    if (!node) {
        std::cerr << "[CPP] Error generating code: AST node is null\n";
        return;
    };
    std::cout << "[CPP] Generating code for node: " << std::endl;
    logASTNode(node, 2);

    switch(node->type) {
        case CryoNodeType::NODE_PROGRAM:
            std::cout << "[CPP] Generating code for program\n";
            generateProgram(node, builder, module);
            break;

        case CryoNodeType::NODE_FUNCTION_DECLARATION:
            std::cout << "[CPP] Generating code for function\n";
            generateFunction(node, builder, module);
            break;

        case CryoNodeType::NODE_VAR_DECLARATION:
            std::cout << "[CPP] Generating code for variable declaration\n";
            generateVarDeclaration(node, builder, module);
            break;

        case CryoNodeType::NODE_STATEMENT:
            std::cout << "[CPP] Generating code for statement\n";
            generateStatement(node, builder, module);
            break;

        case CryoNodeType::NODE_EXPRESSION:
            std::cout << "[CPP] Generating code for expression\n";
            generateExpression(node, builder, module);
            break;

        case CryoNodeType::NODE_BINARY_EXPR:
            std::cout << "[CPP] Generating code for binary expression\n";
            generateBinaryOperation(node, builder, module);
            break;        

        case CryoNodeType::NODE_UNARY_EXPR:
            std::cout << "[CPP] Generating code for unary expression\n";
            std::cout << "[CPP] Unary Expression UNIMPLEMENTED\n";
            // TODO: Implement unary expression code generation
            break;
        
        case CryoNodeType::NODE_LITERAL_EXPR:
            std::cout << "[CPP] Generating code for literal expression\n";
            std::cout << "[CPP] Literal Expression UNIMPLEMENTED\n";
            // TODO: Implement literal expression code generation
            break;
        
        case CryoNodeType::NODE_VAR_NAME:
            std::cout << "[CPP] Generating code for variable name\n";
            std::cout << "[CPP] Variable Name UNIMPLEMENTED\n";
            // TODO: Implement variable name code generation
            break;

        case CryoNodeType::NODE_FUNCTION_CALL:
            std::cout << "[CPP] Generating code for function call\n";
            std::cout << "[CPP] Function Call UNIMPLEMENTED\n";
            // TODO: Implement function call code generation

        case CryoNodeType::NODE_RETURN_STATEMENT:
            std::cout << "[CPP] Generating code for return statement\n";
            generateReturnStatement(node, builder, module);
            break;

        case CryoNodeType::NODE_BLOCK:
            std::cout << "[CPP] Generating code for block\n";
            generateBlock(node, builder, module);
            break;

        case CryoNodeType::NODE_FUNCTION_BLOCK:
            std::cout << "[CPP] Generating code for function block\n";
            generateFunctionBlock(node, builder, module);
            break;

        default:
            std::cerr << "[CPP] Unknown node type\n";
            break;
    }
}
// </generateCode>


// <generateProgram>
void generateProgram(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::cout << "[CPP] Generating code for program\n";
    if (node->data.program.stmtCount == 0) {
        std::cerr << "[CPP] Error generating code for program: No statements found\n";
        return;
    }
    for (int i = 0; i < node->data.program.stmtCount; ++i) {
        std::cout << "[CPP] Generating code for program statement " << i << "\n";
        generateCode(node->data.program.statements[i], builder, module);
        std::cout << "[CPP] Moving to next statement\n";
    }
}
// </generateProgram>


// <generateStatement>
void generateStatement(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::cout << "[CPP] Generating code for statement\n";
    switch (node->type) {
        case CryoNodeType::NODE_EXPRESSION_STATEMENT:
            std::cout << "[CPP] Generating code for expression statement!\n" << std::endl;
            generateExpression(node->data.stmt.stmt, builder, module);
            break;
        case CryoNodeType::NODE_VAR_DECLARATION:
            std::cout << "[CPP] Generating code for variable declaration!\n" << std::endl;
            generateVarDeclaration(node, builder, module);
            break;
        default:
            std::cerr << "[CPP] Unknown statement type\n";
            break;
    }
}
// </generateStatement>


// <generateBlock>
void generateBlock(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::cout << "[CPP] Generating code for block\n";
    if (!node || node->data.block.stmtCount == 0) {
        std::cerr << "[CPP] Error generating code for block: No statements found\n";
        return;
    }
    for (int i = 0; i < node->data.block.stmtCount; ++i) {
        std::cout << "[CPP] Generating code for block statement " << i << "\n";
        generateCode(node->data.block.statements[i], builder, module);
        std::cout << "[CPP] Moving to next statement\n";
    }
}
// </generateBlock>


// <generateExpression>
llvm::Value* generateExpression(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::cout << "[CPP] Generating code for expression\n";
    if (!node) {
        std::cerr << "[CPP] Error: Expression node is null\n";
        return nullptr;
    }

    std::cout << "[CPP - DEBUG] Expression type: " << node->type << "\n";
    std::cout << "[CPP - DEBUG] Expression data type: " << node->data.literalExpression.dataType << "\n";

    switch (node->type) {
        case CryoNodeType::NODE_LITERAL_EXPR:
            std::cout << "[CPP] Generating code for literal expression\n";
            switch (node->data.literalExpression.dataType) {
                case DATA_TYPE_INT:
                    std::cout << "[CPP] Generating int literal: " << node->data.literalExpression.intValue << "\n";
                    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(module.getContext()), node->data.literalExpression.intValue);

                case DATA_TYPE_FLOAT:
                    std::cout << "[CPP] Generating float literal: " << node->data.literalExpression.floatValue << "\n";
                    return llvm::ConstantFP::get(llvm::Type::getFloatTy(module.getContext()), node->data.literalExpression.floatValue);

                case DATA_TYPE_STRING:
                    std::cout << "[CPP] Generating string literal: " << node->data.literalExpression.stringValue << "\n";
                    return createString(builder, module, node->data.literalExpression.stringValue);

                case DATA_TYPE_BOOLEAN:
                    std::cout << "[CPP] Generating boolean literal: " << node->data.literalExpression.booleanValue << "\n";
                    return llvm::ConstantInt::get(llvm::Type::getInt1Ty(module.getContext()), node->data.literalExpression.booleanValue);

                case DATA_TYPE_VOID:
                    std::cerr << "[CPP] Error: Void type cannot be used as an expression\n";
                    return nullptr;

                case DATA_TYPE_UNKNOWN:
                    std::cerr << "[CPP] Error: Unknown data type\n";
                    return nullptr;

                default:
                    std::cerr << "[CPP] Unknown literal expression type\n";
                    return nullptr;
            }
            
        case CryoNodeType::NODE_BINARY_EXPR: {
            llvm::Value* left = generateExpression(node->data.bin_op.left, builder, module);
            llvm::Value* right = generateExpression(node->data.bin_op.right, builder, module);
            if (!left || !right) {
                std::cerr << "[CPP] Error generating binary operation: operands are null\n";
                return nullptr;
            }
            std::cout << "[CPP] Binary operation operands generated\n";
        
            // Check and cast the operands to integer types if necessary
            if (left->getType()->isPointerTy()) {
                left = builder.CreatePtrToInt(left, llvm::Type::getInt32Ty(module.getContext()));
            }
            if (right->getType()->isPointerTy()) {
                right = builder.CreatePtrToInt(right, llvm::Type::getInt32Ty(module.getContext()));
            }
        
            if (!left->getType()->isIntegerTy() || !right->getType()->isIntegerTy()) {
                std::cerr << "[CPP] Error: Binary operation requires integer operands\n";
                return nullptr;
            }
        
            switch (node->data.bin_op.op) {
                case TOKEN_OP_PLUS:
                    return builder.CreateAdd(left, right, "addtmp");
                case TOKEN_OP_MINUS:
                    return builder.CreateSub(left, right, "subtmp");
                case TOKEN_OP_STAR:
                    return builder.CreateMul(left, right, "multmp");
                case TOKEN_OP_SLASH:
                    return builder.CreateSDiv(left, right, "divtmp");
                default:
                    std::cerr << "[CPP] Unknown binary operator\n";
                    return nullptr;
            }
            break;
        }

        case CryoNodeType::NODE_VAR_NAME: {
            llvm::Value* var = getVariableValue(node->data.varName.varName, builder);
            if (!var) {
                std::cerr << "[CPP] Error: Variable not found: " << node->data.varName.varName << "\n";
                return nullptr;
            }
            // Return the loaded value instead of the pointer
            return var;
        }
        default:
            std::cerr << "[CPP] Unknown expression type\n";
            return nullptr;
    }
    std::cout << "[CPP] Expression generated\n";
}
// </generateExpression>


// <generateBinaryOperation>
llvm::Value* generateBinaryOperation(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::cout << "[CPP] Generating code for binary operation\n";

    llvm::Value* left = generateExpression(node->data.bin_op.left, builder, module);
    llvm::Value* right = generateExpression(node->data.bin_op.right, builder, module);

    if (!left || !right) {
        std::cerr << "[CPP] Error generating binary operation: operands are null\n";
        return nullptr;
    }

    std::cout << "[CPP] Binary operation operands generated\n";

    switch (node->data.bin_op.op) {
        case CryoTokenType::TOKEN_OP_PLUS:
            std::cout << "[CPP] Generating code for binary operation: ADD\n";
            return builder.CreateAdd(left, right, "addtmp");
        case CryoTokenType::TOKEN_OP_MINUS:
            std::cout << "[CPP] Generating code for binary operation: SUB\n";
            return builder.CreateSub(left, right, "subtmp");
        case CryoTokenType::TOKEN_OP_STAR:
            std::cout << "[CPP] Generating code for binary operation: MUL\n";
            return builder.CreateMul(left, right, "multmp");
        case CryoTokenType::TOKEN_OP_SLASH:
            std::cout << "[CPP] Generating code for binary operation: DIV\n";
            return builder.CreateSDiv(left, right, "divtmp");
        default:
            std::cerr << "[CPP] Unknown binary operator\n";
            return nullptr;
    }
}
// </generateBinaryOperation>



// <createString>
llvm::Value *createString(llvm::IRBuilder<> &builder, llvm::Module &module, const std::string &str) {
    std::cout << "[CPP_DEBUG - createString] Creating string\n - Value: " << str << "\n";
    // Set up the string type & struct
    llvm::LLVMContext &context = module.getContext();
    std::cout << "[CPP_DEBUG - createString] Setting uqp string type & struct\n";
    
    llvm::StructType *stringType = createStringType(context, builder);
    std::cout << "[CPP_DEBUG - createString] String type set up\n";

    // Use the ConstantDataArray to create a buffer
    llvm::Constant *buffer = llvm::ConstantDataArray::getString(context, str);
    std::cout << "[CPP_DEBUG - createString] Buffer created\n";

    // Create a global variable for the buffer
    llvm::GlobalVariable *globalBuffer = new llvm::GlobalVariable(module, buffer->getType(), true, llvm::GlobalValue::PrivateLinkage, buffer);
    std::cout << "[CPP_DEBUG - createString] Global buffer created\n";

    // Create a constant struct with the buffer, length, maxlen, and factor
    llvm::Constant *length = llvm::ConstantInt::get(builder.getInt32Ty(), str.size());
    llvm::Constant *maxlen = llvm::ConstantInt::get(builder.getInt32Ty(), str.size());
    llvm::Constant *factor = llvm::ConstantInt::get(builder.getInt32Ty(), 16);
    std::cout << "[CPP_DEBUG - createString] Constants created\n";

    // Create the struct
    std::vector<llvm::Constant *> elements = {
        llvm::ConstantExpr::getPointerCast(globalBuffer, builder.getInt8Ty()->getPointerTo()),
        length,
        maxlen,
        factor
    };
    std::cout << "[CPP_DEBUG - createString] Elements created\n";

    // Return the constant struct
    return llvm::ConstantStruct::get(stringType, elements);
}
// </createString>
