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


// <codegen> 
void codegen(ASTNode* root) {
    llvm::LLVMContext context;
    llvm::IRBuilder<> builder(context);
    std::unique_ptr<llvm::Module> module = std::make_unique<llvm::Module>("CryoModule", context);

    generateCode(root, builder, *module);

    if (llvm::verifyModule(*module, &llvm::errs())) {
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
    logNode(node);

    switch (node->type) {
        case CryoNodeType::NODE_PROGRAM:
            std::cout << "[CPP] Starting code generation for program...\n" << std::endl;
            generateProgram(node, builder, module);
            break;
        case CryoNodeType::NODE_STATEMENT:
            std::cout << "[CPP] Starting code generation for <statement>\n" << std::endl;
            generateStatement(node, builder, module);
            break;
        case CryoNodeType::NODE_EXPRESSION:
            std::cout << "[CPP] Starting code generation for <expression>\n" << std::endl;
            generateExpression(node, builder, module);
            break;
        case CryoNodeType::NODE_VAR_DECLARATION:
            std::cout << "[CPP] Starting code generation for <var declaration>\n" << std::endl;
            generateVarDeclaration(node, builder, module);
            break;
        // Add cases for other node types
        default:
            std::cerr << "[CPP] Unknown AST node type\n";
            break;
    }
}
// </generateCode>


// <generateProgram>
void generateProgram(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::cout << "[CPP] Generating code for program\n";
    ASTNode* stmt = *node->data.program.statements;
    if(!stmt) {
        std::cerr << "[CPP] Error generating code for program: No statements found\n";
        return;
    }
    std::cout << "[CPP] Generating code for program statements\n";
    while (stmt) {
        std::cout << "[CPP] Starting process for generating code...\n";
        generateCode(stmt, builder, module);
        std::cout << "[CPP] Moving to next statement\n" << std::endl;
        stmt = stmt->nextSibling;
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


// <generateVarDeclaration>
void generateVarDeclaration(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::cout << "[CPP] Generating code for variable declaration\n";

    llvm::Type* varType = nullptr;
    llvm::Constant* initializer = nullptr;

    switch (node->data.varDecl.dataType) {
        case DATA_TYPE_INT:
            std::cout << "[CPP] Variable type: int\n";
            varType = llvm::Type::getInt32Ty(module.getContext());
            initializer = llvm::ConstantInt::get(varType, node->data.varDecl.initializer->data.literalExpression.intValue);
            break;
        case DATA_TYPE_STRING: {
            std::cout << "[CPP] Variable type: string\n";
            
            // using the createString function to create a string
            llvm::Value *string = createString(builder, module, node->data.varDecl.initializer->data.literalExpression.stringValue);
            llvm::Type *varType = string->getType();
            // Convert to LLVM::Constant

            initializer = llvm::ConstantExpr::getPointerCast((llvm::Constant *)string, varType);

            // Add the string to the module
            module.getOrInsertGlobal(node->data.varDecl.name, varType);

            // Create a global variable
            llvm::GlobalVariable *gVar = new llvm::GlobalVariable(
                module,
                varType,
                false,
                llvm::GlobalValue::ExternalLinkage,
                initializer,
                node->data.varDecl.name
            );

            std::cout << "[CPP] Variable registered in module's global scope\n";
            std::cout << "[CPP] Variable name: " << node->data.varDecl.name << "\n";
            break;
        }
        default:
            std::cerr << "[CPP] Unknown variable type\n";
            return;
    }

    llvm::GlobalVariable* gVar = new llvm::GlobalVariable(
        module,
        varType,
        false,
        llvm::GlobalValue::ExternalLinkage,
        initializer,
        node->data.varDecl.name
    );

    std::cout << "[CPP] Variable registered in module's global scope\n";
    std::cout << "[CPP] Variable name: " << node->data.varDecl.name << "\n";
    std::cout << "[CPP] Variable type: " << varType << "\n";
    std::cout << "[CPP] Variable initializer: " << initializer << std::endl;
}
// </generateVarDeclaration>


// <generateExpression>
llvm::Value* generateExpression(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::cout << "[CPP] Generating code for expression\n";
    switch (node->type) {
        case CryoNodeType::NODE_LITERAL_EXPR:
            std::cout << "[CPP] Generating code for literal expression\n";
            switch (node->data.literalExpression.dataType) {
                case DATA_TYPE_INT:
                    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(module.getContext()), node->data.literalExpression.intValue);
                case DATA_TYPE_STRING:
                    return builder.CreateGlobalStringPtr(node->data.literalExpression.stringValue);
                default:
                    std::cerr << "[CPP] Unknown literal expression type\n";
                    return nullptr;
            }
        case CryoNodeType::NODE_BINARY_EXPR:
            std::cout << "[CPP] Generating code for binary operation\n";
            return generateBinaryOperation(node, builder, module);
        default:
            std::cerr << "[CPP] Unknown expression type\n";
            return nullptr;
    }
}
// </generateExpression>


// <generateBinaryOperation>
llvm::Value* generateBinaryOperation(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::cout << "[CPP] Generating code for binary operation\n";
    llvm::Value* left = generateExpression(node->data.bin_op.left, builder, module);
    llvm::Value* right = generateExpression(node->data.bin_op.right, builder, module);

    if (!left || !right) {
        std::cerr << "[CPP] Error generating binary operation\n";
        return nullptr;
    }

    std::cout << "[CPP] Binary operation operands generated\n";

    switch (node->data.bin_op.op) {
        case CryoTokenType::TOKEN_PLUS:
            std::cout << "[CPP] Generating code for binary operation: ADD\n";
            return builder.CreateAdd(left, right);
        case CryoTokenType::TOKEN_MINUS:
            std::cout << "[CPP] Generating code for binary operation: SUB\n";
            return builder.CreateSub(left, right);
        case CryoTokenType::TOKEN_STAR:
            std::cout << "[CPP] Generating code for binary operation: MUL\n";
            return builder.CreateMul(left, right);
        case CryoTokenType::TOKEN_SLASH:
            std::cout << "[CPP] Generating code for binary operation: DIV\n";
            return builder.CreateSDiv(left, right);
        // Add cases for other binary operators
        default:
            std::cerr << "[CPP] Unknown binary operator\n";
            return nullptr;
    }
}
// </generateBinaryOperation>


// <createStringStruct>
llvm::StructType *createStringStruct(llvm::LLVMContext &context) {
    return llvm::StructType::create(context, "String");
}
// </createStringStruct>


// <createStringType>
llvm::StructType *createStringType(llvm::LLVMContext &context, llvm::IRBuilder<> &builder) {
    llvm::StructType *stringType = createStringStruct(context);
    std::vector<llvm::Type *> elements = {
        builder.getInt8Ty(),        // buffer
        builder.getInt32Ty(),       // length
        builder.getInt32Ty(),       // maxlen
        builder.getInt32Ty()        // factor
    };
    stringType->setBody(elements);
    return stringType;
}
// </createStringType>


// <createString>
llvm::Value *createString(llvm::IRBuilder<> &builder, llvm::Module &module, const std::string &str) {
    // Set up the string type & struct
    llvm::LLVMContext &context = module.getContext();
    llvm::StructType *stringType = createStringType(context, builder);
    // Use the ConstantDataArray to create a buffer
    llvm::Constant *buffer = llvm::ConstantDataArray::getString(context, str);
    // Create a global variable for the buffer
    llvm::GlobalVariable *globalBuffer = new llvm::GlobalVariable(module, buffer->getType(), true, llvm::GlobalValue::PrivateLinkage, buffer);
    // Create a constant struct with the buffer, length, maxlen, and factor
    llvm::Constant *length = llvm::ConstantInt::get(builder.getInt32Ty(), str.size());
    llvm::Constant *maxlen = llvm::ConstantInt::get(builder.getInt32Ty(), str.size());
    llvm::Constant *factor = llvm::ConstantInt::get(builder.getInt32Ty(), 16);
    // Create the struct
    std::vector<llvm::Constant *> elements = {
        llvm::ConstantExpr::getPointerCast(globalBuffer, builder.getInt8Ty()->getPointerTo()),
        length,
        maxlen,
        factor
    };
    // Return the constant struct
    return llvm::ConstantStruct::get(stringType, elements);
}
// </createString>
