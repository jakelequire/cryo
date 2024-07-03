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
    llvm::Value* initialValue = nullptr;
    std::string varName = node->data.varDecl.name;

    switch (node->data.varDecl.dataType) {
        case DATA_TYPE_STRING:
            varType = builder.getInt8Ty()->getPointerTo();
            initialValue = createString(builder, module, node->data.varDecl.initializer->data.literalExpression.stringValue);
            break;
        
        case DATA_TYPE_INT:
            varType = builder.getInt32Ty();
            break;

        case DATA_TYPE_FLOAT:
            varType = builder.getFloatTy();
            break;

        case DATA_TYPE_BOOLEAN:
            varType = builder.getInt1Ty();
            break;
        // Handle other types...
    }

    if (!initialValue) {
        initialValue = generateExpression(node->data.varDecl.initializer, builder, module);
        if (!initialValue) {
            std::cerr << "[CPP] Error: Failed to generate initializer for variable: " << varName << "\n";
            return;
        }
    }

    llvm::AllocaInst* alloca = builder.CreateAlloca(varType, nullptr, varName);
    builder.CreateStore(initialValue, alloca);
    namedValues[varName] = alloca;
}

// </generateVarDeclaration>

