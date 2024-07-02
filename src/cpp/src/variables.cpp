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

extern std::unordered_map<std::string, llvm::Value*> namedValues;


// Function to retrieve a variable from the namedValues map
// <getVariable>
llvm::Value* getVariable(const std::string& name) {
    auto it = namedValues.find(name);
    if (it != namedValues.end()) {
        llvm::Value* var = it->second;
        if (!var) {
            std::cerr << "[CPP] Error: Variable value is null\n";
            return nullptr;
        }
        return var;
    } else {
        std::cerr << "[CPP] Error: Variable not found\n";
        return nullptr;
    }
}
// </getVariable>


// Function to load a global variable value
// <loadGlobalVariable>
llvm::Value* loadGlobalVariable(llvm::GlobalVariable* globalVar, llvm::IRBuilder<>& builder, const std::string& name) {
    std::cout << "[CPP] Loading global variable value\n";
    llvm::Type* globalVarType = globalVar->getValueType();
    std::cout << "[CPP] Global Variable Type: " << globalVarType << "\n";

    if (!globalVar) {
        std::cerr << "[CPP] Error: globalVar is null\n";
        return nullptr;
    }
    if (!globalVarType) {
        std::cerr << "[CPP] Error: globalVarType is null\n";
        return nullptr;
    }

    std::cout << "[CPP] Creating load instruction\n";
    llvm::LoadInst* load = builder.CreateLoad(globalVarType, globalVar, llvm::Twine(name));
    std::cout << "[CPP] Created load instruction\n";
    assert(load && "[CPP] Error: Failed to load global variable value");

    std::cout << "[CPP] Loaded global variable value\n";
    return load;
}
// </loadGlobalVariable>


// Function to load a pointer variable value
// <loadPointerVariable>
llvm::Value* loadPointerVariable(llvm::Value* var, llvm::IRBuilder<>& builder, const std::string& name) {
    std::cout << "[CPP] Loading pointer variable value\n";
    llvm::Type* pointedType = var->getType();
    std::cout << "[CPP] Variable pointer type: " << pointedType << "\n";
    llvm::LoadInst* load = builder.CreateLoad(pointedType, var, llvm::Twine(name));
    if (!load) {
        std::cerr << "[CPP] Error: Failed to load pointer variable value\n";
        return nullptr;
    }
    std::cout << "[CPP] Loaded pointer variable value\n";
    return load;
}
// </loadPointerVariable>


// Function to get the variable value
// <getVariableValue>
llvm::Value* getVariableValue(const std::string& name, llvm::IRBuilder<>& builder) {
    auto it = namedValues.find(name);
    if (it != namedValues.end()) {
        llvm::Value* var = it->second;
        if (!var) {
            std::cerr << "[CPP] Error: Variable value is null\n";
            return nullptr;
        }
        std::cout << "[CPP] Found variable: " << name << " of type " << (var->getType()) << "\n";

        if (llvm::GlobalVariable* globalVar = llvm::dyn_cast<llvm::GlobalVariable>(var)) {
            std::cout << "[CPP] Loading global variable value\n";
            llvm::Type* globalVarType = globalVar->getValueType();
            std::cout << "[CPP] Global Variable Type: " << globalVarType << "\n";
            llvm::LoadInst* load = builder.CreateLoad(globalVarType, globalVar, llvm::Twine(name));
            assert(load && "[CPP] Error: Failed to load global variable value");
            std::cout << "[CPP] Loaded global variable value\n";
            return load;
        } else if (var->getType()->isPointerTy()) {
            std::cout << "[CPP] Loading pointer variable value\n";
            llvm::LoadInst* load = builder.CreateLoad(var->getType(), var, llvm::Twine(name));
            assert(load && "[CPP] Error: Failed to load pointer variable value");
            std::cout << "[CPP] Loaded pointer variable value\n";
            return load;
        } else {
            std::cerr << "[CPP] Error: Variable is neither a pointer nor a global variable\n";
            return nullptr;
        }
    } else {
        std::cerr << "[CPP] Error: Variable not found\n";
        return nullptr;
    }
}

// </getVariableValue>



// <generateVarDeclaration>
void generateVarDeclaration(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::cout << "[CPP] Generating code for variable declaration\n";

    llvm::Type* varType = nullptr;

    std::cout << "[CPP DEBUG] Variable name: <" << node->data.varDecl.name << ">\n";
    std::cout << "[CPP DEBUG] Variable data type: " << node->data.varDecl.dataType << "\n";
    logCryoDataType(node->data.varDecl.dataType);
    std::cout << "[CPP DEBUG] Variable initializer: <" << node->data.varDecl.initializer << ">\n";

    llvm::Value* initializer = nullptr;
    switch (node->data.varDecl.dataType) {
        case DATA_TYPE_INT:
            std::cout << "[CPP] Variable type: int\n";
            varType = llvm::Type::getInt32Ty(module.getContext());
            initializer = generateExpression(node->data.varDecl.initializer, builder, module);
            break;
        case DATA_TYPE_STRING:
            std::cout << "[CPP] Variable type: string\n";
            {
                llvm::Value* string = createString(builder, module, node->data.varDecl.initializer->data.literalExpression.stringValue);
                varType = string->getType();
                initializer = string;
            }
            break;
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

    // This was the error in the previous code
    // Fixed by adding the initializer to the GlobalVariable constructor
    // Allocate the variable as a global variable
    std::cout << "[CPP] Allocating variable as global variable\n";
    // std::string varName = node->data.varDecl.name;
    // llvm::GlobalVariable* globalVar = new llvm::GlobalVariable(
    //     module,
    //     varType,
    //     false,
    //     llvm::GlobalValue::ExternalLinkage,
    //     static_cast<llvm::Constant*>(initializer),
    //     node->data.varDecl.name
    // );
    llvm::AllocaInst* alloca = builder.CreateAlloca(varType, 0, node->data.varDecl.name);
    builder.CreateStore(initializer, alloca);
    // if (!globalVar) {
    //     std::cerr << "[CPP] Error: Allocation failed\n";
    //     return;
    // }

    // std::cout << "[CPP] Global Variable: " << globalVar << "\n";

    // Add the variable to the named values map
    namedValues[node->data.varDecl.name] = alloca;

    std::cout << "[CPP] Variable registered in global scope\n";
    std::cout << "[CPP] Variable name: " << node->data.varDecl.name << "\n";
    std::cout << "[CPP] Variable name: " << node->data.varDecl.name << "\n";
}
// </generateVarDeclaration>

