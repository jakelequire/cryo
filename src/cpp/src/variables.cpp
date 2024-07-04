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


llvm::GlobalVariable* createGlobalVariable(llvm::Module& module, llvm::Type* varType, llvm::Constant* initialValue, const std::string& varName) {
    std::cout << "[CPP] Creating global variable\n";
    llvm::GlobalVariable* globalVar = new llvm::GlobalVariable(
        module,
        varType,
        false,
        llvm::GlobalValue::ExternalLinkage,
        initialValue,
        varName
    );

    if (!globalVar) {
        std::cerr << "[CPP] Error: Failed to create global variable: " << varName << "\n";
        return nullptr;
    }

    std::cout << "[CPP] Created global variable: " << varName << "\n";
    return globalVar;
}



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
    llvm::Constant* initialValue = nullptr;
    std::string varName = node->data.varDecl.name;

    switch (node->data.varDecl.dataType) {
        case DATA_TYPE_STRING:
            std::cout << "[CPP] Generating code for string variable\n";
            if (node->data.varDecl.initializer->type == NODE_LITERAL_EXPR) {
                std::cout << "[CPP] Initializer is a literal expression\n";
                initialValue = llvm::cast<llvm::Constant>(createString(builder, module, node->data.varDecl.initializer->data.literalExpression.stringValue));
                varType = builder.getInt8Ty()->getPointerTo();
            } else if (node->data.varDecl.initializer->type == NODE_VAR_NAME) {
                std::cout << "[CPP] Initializer is a variable name\n";
                llvm::GlobalVariable* refVar = module.getGlobalVariable(node->data.varDecl.initializer->data.varName.varName);
                if (refVar) {
                    initialValue = llvm::ConstantExpr::getPointerCast(refVar, refVar->getType());
                    varType = refVar->getValueType();
                } else {
                    std::cerr << "[CPP] Error: Referenced variable not found: " << node->data.varDecl.initializer->data.varName.varName << "\n";
                    return;
                }
            } else {
                std::cerr << "[CPP] Unknown expression type for string variable initializer\n";
                return;
            }
            break;
        
        case DATA_TYPE_INT:
            std::cout << "[CPP] Generating code for int variable\n";
            varType = builder.getInt32Ty();
            if (node->data.varDecl.initializer->type == NODE_LITERAL_EXPR) {
                std::cout << "[CPP] Initializer is a literal expression\n";
                initialValue = createConstantInt(builder, node->data.varDecl.initializer->data.literalExpression.intValue);
            } else if (node->data.varDecl.initializer->type == NODE_VAR_NAME) {
                std::cout << "[CPP] Initializer is a variable name\n";
                llvm::GlobalVariable* refVar = module.getGlobalVariable(node->data.varDecl.initializer->data.varName.varName);
                if (refVar) {
                    initialValue = llvm::cast<llvm::Constant>(refVar->getInitializer());
                } else {
                    std::cerr << "[CPP] Error: Referenced variable not found: " << node->data.varDecl.initializer->data.varName.varName << "\n";
                    return;
                }
            } else {
                std::cerr << "[CPP] Unknown expression type for int variable initializer\n";
                return;
            }
            break;

        case DATA_TYPE_FLOAT:
            std::cout << "[CPP] Generating code for float variable\n";
            varType = builder.getFloatTy()->getPointerTo();
            break;

        case DATA_TYPE_BOOLEAN:
            std::cout << "[CPP] Generating code for boolean variable\n";
            varType = builder.getInt1Ty()->getPointerTo();
            break;
        
        case DATA_TYPE_UNKNOWN:
        case DATA_TYPE_VOID:
            std::cerr << "[CPP] Error: Cannot declare variable of type void\n";
            return;
        // Handle other types...

        default:
            std::cout << "[CPP] Error: Invalid data type\n";
            std::cerr << "[CPP - ERROR] Invalid data type for variable: " << varName << "\n";
            return;
    }


    std::cout << "[CPP - DEBUG] varType: " << varType << "\n";
    std::cout << "[CPP - DEBUG] varType is pointer: " << varType->isPointerTy() << "\n";

    if (!initialValue) {
        std::cerr << "[CPP] Error: Initial value is null for variable: " << varName << "\n";
        return;
    }

    std::cout << "[CPP - DEBUG] varType: " << varType << "\n";
    if (varType) {
        std::cout << "[CPP - DEBUG] varType is pointer: " << varType->isPointerTy() << "\n";
    } else {
        std::cerr << "[CPP] Error: varType is null\n";
        return;
    }

    std::cout << "[CPP - DEBUG] initialValue type: " << initialValue->getType() << "\n";
    if (initialValue->getType()->isPointerTy()) {
        std::cout << "[CPP - DEBUG] initialValue is a pointer type\n";
    } else {
        std::cout << "[CPP - DEBUG] initialValue is not a pointer type\n";
    }

    // Determine if this is a global variable
    // Ensure initialValue is of type llvm::Constant* for global variables
    if (node->data.varDecl.isGlobal) {
        std::cout << "[CPP] Creating global variable\n";
        llvm::GlobalVariable* globalVar = new llvm::GlobalVariable(
            module,
            varType,
            false,
            llvm::GlobalValue::ExternalLinkage,
            initialValue,
            varName
        );
        if (globalVar) {
            std::cout << "[CPP] Created global variable: " << varName << "\n";
            namedValues[varName] = globalVar;
        } else {
            std::cerr << "[CPP] Error: Failed to create global variable: " << varName << "\n";
        }
    } else {
        std::cout << "[CPP] Creating alloca instruction\n";
        llvm::AllocaInst* alloca = builder.CreateAlloca(varType, nullptr, varName);
        std::cout << "[CPP] Created alloca instruction\n";
        builder.CreateStore(initialValue, alloca);
        std::cout << "[CPP] Stored initial value in alloca\n";
        namedValues[varName] = alloca;
        std::cout << "[CPP] Stored alloca in namedValues\n";
    }
}
// </generateVarDeclaration>

