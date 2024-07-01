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
            // TODO: Implement unary expression code generation
            break;
        
        case CryoNodeType::NODE_LITERAL_EXPR:
            std::cout << "[CPP] Generating code for literal expression\n";
            // TODO: Implement literal expression code generation
            break;
        
        case CryoNodeType::NODE_VAR_NAME:
            std::cout << "[CPP] Generating code for variable name\n";
            // TODO: Implement variable name code generation
            break;

        case CryoNodeType::NODE_FUNCTION_CALL:
            std::cout << "[CPP] Generating code for function call\n";
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


llvm::Value* getVariableValue(const std::string& name, llvm::IRBuilder<>& builder) {
    auto it = namedValues.find(name);
    if (it != namedValues.end()) {
        llvm::Value* var = it->second;
        return builder.CreateLoad(var->getType(), var, llvm::Twine(name));
    }
    return nullptr;
}

// <generateVarDeclaration>
void generateVarDeclaration(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::cout << "[CPP] Generating code for variable declaration\n";

    llvm::Type* varType = nullptr;
    llvm::Value* initializer = nullptr;

    std::cout << "[CPP DEBUG] Variable name: <" << node->data.varDecl.name << ">\n";
    std::cout << "[CPP DEBUG] Variable data type:" << "\n";
    logCryoDataType(node->data.varDecl.dataType);
    std::cout << "[CPP DEBUG] Variable initializer: <" << node->data.varDecl.initializer << ">\n";

    switch (node->data.varDecl.dataType) {
        case DATA_TYPE_INT:
            std::cout << "[CPP] Variable type: int\n";
            varType = llvm::Type::getInt32Ty(module.getContext());
            initializer = generateExpression(node->data.varDecl.initializer, builder, module);
            break;
        case DATA_TYPE_STRING: {
            std::cout << "[CPP] Variable type: string\n";
            llvm::Value* string = createString(builder, module, node->data.varDecl.initializer->data.literalExpression.stringValue);
            varType = string->getType();
            initializer = string;
            break;
        }
        case DATA_TYPE_BOOLEAN:
            std::cout << "[CPP] Variable type: boolean\n";
            varType = llvm::Type::getInt1Ty(module.getContext());
            initializer = llvm::ConstantInt::get(varType, node->data.varDecl.initializer->data.literalExpression.booleanValue);
            break;
        case DATA_TYPE_FLOAT:
            std::cout << "[CPP] Variable type: float\n";
            varType = llvm::Type::getFloatTy(module.getContext());
            initializer = llvm::ConstantFP::get(varType, node->data.varDecl.initializer->data.literalExpression.floatValue);
            break;
        case DATA_TYPE_VOID:
            std::cerr << "[CPP] Void type cannot be used as a variable\n";
            return;
        default:
            std::cerr << "[CPP] Unknown data type\n";
            return;
    }

    if (!initializer) {
        std::cerr << "[CPP] Error: Initializer for variable is null\n";
        return;
    }

    // Allocate the variable in the function's entry block
    llvm::Function* function = builder.GetInsertBlock()->getParent();
    llvm::IRBuilder<> tmpB(&function->getEntryBlock(), function->getEntryBlock().begin());
    llvm::AllocaInst* alloca = tmpB.CreateAlloca(varType, 0, node->data.varDecl.name);

    // Store the initializer in the allocated variable
    builder.CreateStore(initializer, alloca);

    // Add the variable to the named values map
    namedValues[node->data.varDecl.name] = alloca;

    std::cout << "[CPP] Variable registered in function's local scope\n";
    std::cout << "[CPP] Variable name: " << node->data.varDecl.name << "\n";
}
// </generateVarDeclaration>


// <generateReturnStatement>
void generateReturnStatement(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    if (!node) {
        std::cerr << "[CPP] Error: Return statement node is null\n";
        return;
    }
    llvm::Value* retValue = nullptr;
    if (node->data.returnStmt.returnValue) {
        std::cout << "[CPP] Return statement node has return value of type: " << node->data.returnStmt.returnValue->type << "\n";
        retValue = generateExpression(node->data.returnStmt.returnValue, builder, module);
    }

    if (!retValue) {
        std::cerr << "[CPP] Error: Return statement value is null\n";
        builder.CreateRetVoid();
        return;
    }

    // Check if the return value is a variable name and load its value
    if (node->data.returnStmt.returnValue->type == CryoNodeType::NODE_VAR_NAME) {
        retValue = getVariableValue(node->data.returnStmt.returnValue->data.varName.varName, builder);
    }

    if (retValue) {
        std::cout << "[CPP] Return value: " << retValue << "\n";
        builder.CreateRet(retValue);
    } else {
        std::cerr << "[CPP] Error: Failed to generate return value\n";
        builder.CreateRetVoid();
    }
}
// </generateReturnStatement>


// <generateFunction>
void generateFunction(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::cout << "[CPP] Generating code for function: " << node->data.functionDecl.name << "\n";

    // Retrieve and handle the function return type
    llvm::Type* returnType = nullptr;
    switch (node->data.functionDecl.returnType) {
        case DATA_TYPE_INT:
            returnType = llvm::Type::getInt32Ty(module.getContext());
            break;
        case DATA_TYPE_FLOAT:
            returnType = llvm::Type::getFloatTy(module.getContext());
            break;
        case DATA_TYPE_BOOLEAN:
            returnType = llvm::Type::getInt1Ty(module.getContext());
            break;
        case DATA_TYPE_VOID:
            returnType = builder.getVoidTy();
            break;
        default:
            std::cerr << "[CPP] Error: Unknown function return type: " << node->data.functionDecl.returnType << "\n";
            return;
    }
    std::cout << "[CPP] Function return type: " << node->data.functionDecl.returnType << " mapped to LLVM type: " << returnType << "\n";

    // Define function parameters
    std::vector<llvm::Type*> paramTypes;
    if (node->data.functionDecl.params) {
        for (int i = 0; i < node->data.functionDecl.paramCount; ++i) {
            ASTNode* paramNode = node->data.functionDecl.params->data.paramList.params[i];
            switch (paramNode->data.varDecl.dataType) {
                case DATA_TYPE_INT:
                    paramTypes.push_back(llvm::Type::getInt32Ty(module.getContext()));
                    break;
                case DATA_TYPE_FLOAT:
                    paramTypes.push_back(llvm::Type::getFloatTy(module.getContext()));
                    break;
                case DATA_TYPE_BOOLEAN:
                    paramTypes.push_back(llvm::Type::getInt1Ty(module.getContext()));
                    break;
                default:
                    std::cerr << "[CPP] Error: Unknown parameter type\n";
                    return;
            }
        }
    }

    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramTypes, false);
    llvm::Function::LinkageTypes linkageType = node->data.functionDecl.visibility == CryoVisibilityType::VISIBILITY_PRIVATE ? llvm::Function::PrivateLinkage : llvm::Function::ExternalLinkage;
    llvm::Function* function = llvm::Function::Create(funcType, linkageType, node->data.functionDecl.name, module);

    // Assign names to the parameters
    auto paramIter = function->arg_begin();
    if (node->data.functionDecl.params) {
        for (int i = 0; i < node->data.functionDecl.paramCount; ++i, ++paramIter) {
            ASTNode* paramNode = node->data.functionDecl.params->data.paramList.params[i];
            paramIter->setName(paramNode->data.varDecl.name);
        }
    }

    llvm::BasicBlock* BB = llvm::BasicBlock::Create(module.getContext(), "entry", function);
    builder.SetInsertPoint(BB);

    // Check for a valid function body
    if (node->data.functionDecl.body) {
        generateCode(node->data.functionDecl.body, builder, module);
    } else {
        std::cerr << "[CPP] Error: Function body is null\n";
        return;
    }

    // Ensure proper function termination
    if (returnType->isVoidTy()) {
        builder.CreateRetVoid();
    } else {
        if (BB->getTerminator() == nullptr) {
            std::cerr << "[CPP] Error: Missing return statement\n";
            builder.CreateRet(llvm::Constant::getNullValue(returnType));
        }
    }
}
// </generateFunction>


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


// <generateFunctionBlock>
void generateFunctionBlock(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::cout << "[CPP] Generating code for function block\n";
    if (!node || !node->data.functionBlock.block) {
        std::cerr << "[CPP] Error generating code for function block: Invalid function block\n";
        return;
    }
    std::cout << "[CPP] Function block contains " << node->data.functionBlock.block->data.block.stmtCount << " statements\n";

    // Generate code for each statement in the block
    for (int i = 0; i < node->data.functionBlock.block->data.block.stmtCount; ++i) {
        std::cout << "[CPP] Generating code for block statement " << i << "\n";
        generateCode(node->data.functionBlock.block->data.block.statements[i], builder, module);
        std::cout << "[CPP] Moving to next statement\n";
    }
}
// </generateFunctionBlock>


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


// <createStringStruct>
llvm::StructType *createStringStruct(llvm::LLVMContext &context) {
    return llvm::StructType::create(context, "String");
}
// </createStringStruct>


// <createStringType>
llvm::StructType *createStringType(llvm::LLVMContext &context, llvm::IRBuilder<> &builder) {
    llvm::StructType *stringType = createStringStruct(context);
    std::vector<llvm::Type *> elements = {
        builder.getInt8Ty()->getPointerTo(),    // buffer
        builder.getInt32Ty(),                   // length
        builder.getInt32Ty(),                   // maxlen
        builder.getInt32Ty()                    // factor
    };
    stringType->setBody(elements);
    return stringType;
}
// </createStringType>


// <createString>
llvm::Value *createString(llvm::IRBuilder<> &builder, llvm::Module &module, const std::string &str) {
    std::cout << "[CPP_DEBUG - createString] Creating string\n - Value: " << str << "\n";
    // Set up the string type & struct
    llvm::LLVMContext &context = module.getContext();
    std::cout << "[CPP_DEBUG - createString] Setting up string type & struct\n";
    
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
