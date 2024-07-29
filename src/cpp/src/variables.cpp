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


// <getVariable>
llvm::Value* CodeGen::getVariable(char* name) {
    std::cout << "[CPP] Getting variable: " << name << "\n";
    auto it = namedValues.find(name);
    if (it != namedValues.end()) {
        llvm::Value* var = it->second;
        if (!var) {
            std::cerr << "[CPP] Error: Variable value is null\n";
            return nullptr;
        }
        return var;
    } else {
        std::cerr << "[CPP] <getVariable> Error: Variable not found\n";
        return nullptr;
    }
}
// </getVariable>



// <lookupVariable>
llvm::Value* CodeGen::lookupVariable(char* name) {
    llvm::Value* var = getVariable(name);
    if (!var) {
        std::cerr << "[CPP] <lookupVariable> Error: Variable not found: " << name << "\n";
        return nullptr;
    }
    return var;
}
// </lookupVariable>


// <createGlobalVariable>
llvm::GlobalVariable* CodeGen::createGlobalVariable(llvm::Type* varType, llvm::Constant* initialValue, char* varName) {
    std::cout << "[CPP] Creating global variable\n";
    std::cout << "[CPP] Variable Name: " << varName << "\n";
    llvm::GlobalVariable* globalVar = new llvm::GlobalVariable(
        *module,
        varType,
        false,
        llvm::GlobalValue::ExternalLinkage,
        initialValue,
        llvm::Twine(varName)
    );

    if (!globalVar) {
        std::cerr << "[CPP] Error: Failed to create global variable: " << varName << "\n";
        return nullptr;
    }

    std::cout << "[CPP]! Created global variable: " << varName << "\n";
    return globalVar;
}
// </createGlobalVariable>


// <loadGlobalVariable>
llvm::Value* CodeGen::loadGlobalVariable(llvm::GlobalVariable* globalVar, char* name) {
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


// <loadPointerVariable>
llvm::Value* CodeGen::loadPointerVariable(llvm::Value* var, char* name) {
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
// <loadPointerVariable>


// <getVariableValue>
llvm::Value* CodeGen::getVariableValue(char* name) {
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
        std::cerr << "[CPP] Error <getVariableValue>: Variable not found\n";
        return nullptr;
    }
}
// </getVariableValue>


// <generateVarDeclaration>
void CodeGen::generateVarDeclaration(ASTNode* node) {
    std::cout << "[CPP] Generating code for variable declaration\n";

    llvm::Type* varType = nullptr;
    llvm::Value* initialValue = nullptr;
    llvm::Value* computedValue = nullptr; // For non-literal initializers
    char* varName = node->data.varDecl.name;

    std::cout << "[CPP]! Variable Name: " << node->data.varDecl.name << "\n";
    std::cout << "[CPP]! Variable Type: " << CryoDataTypeToString(node->data.varDecl.dataType) << "\n";

    switch (node->data.varDecl.dataType) {
        case DATA_TYPE_STRING:
            std::cout << "[CPP] Generating code for string variable\n";
            if (node->data.varDecl.initializer) {
                initialValue = createString(node->data.varDecl.initializer->data.literalExpression.stringValue);
            } else {
                initialValue = llvm::ConstantPointerNull::get(llvm::PointerType::get(builder.getInt8Ty(), 0));
            }
            varType = llvm::PointerType::get(builder.getInt8Ty(), 0);
            break;
        
        case DATA_TYPE_INT:
            std::cout << "[CPP] Generating code for int variable\n";
            varType = builder.getInt32Ty();
            if(node->data.varDecl.isReference) {
                std::cout << "[CPP] Generating code for reference variable\n";
                llvm::Value * refVar = lookupVariable(node->data.varDecl.name);
                if (!refVar) {
                    std::cerr << "[CPP] Error: Referenced variable not found: " << node->data.varDecl.name << "\n";
                    // try to find from symbol table
                    refVar = getVariableValue(node->data.varDecl.name);
                    if (!refVar) {
                        std::cerr << "[CPP] Error: Referenced variable not found in symbol table: " << node->data.varDecl.name << "\n";
                        return;
                    }
                    return;
                }
            } else if (node->data.varDecl.initializer->type == NODE_LITERAL_EXPR) {
                std::cout << "[CPP] Initializer is a literal expression\n";
                initialValue = createConstantInt(node->data.varDecl.initializer->data.literalExpression.intValue);
            } else if (node->data.varDecl.initializer->type == NODE_VAR_NAME) {
                std::cout << "[CPP] Initializer is a variable name\n";
                llvm::GlobalVariable* refVar = module->getGlobalVariable(node->data.varDecl.initializer->data.varName.varName);
                if (refVar) {
                    initialValue = llvm::cast<llvm::Constant>(refVar->getInitializer());
                } else {
                    std::cerr << "[CPP] Error: Referenced variable not found: " << node->data.varDecl.initializer->data.varName.varName << "\n";
                    return;
                }
            } else if (node->data.varDecl.initializer->type == NODE_BINARY_EXPR) {
                std::cout << "[CPP] Initializer is a binary expression\n";
                computedValue = generateBinaryOperation(node->data.varDecl.initializer);
                if (!computedValue) {
                    std::cerr << "[CPP] Error: Failed to generate code for binary expression\n";
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

        case DATA_TYPE_INT_ARRAY:
        case DATA_TYPE_FLOAT_ARRAY:
        case DATA_TYPE_STRING_ARRAY:
        case DATA_TYPE_BOOLEAN_ARRAY:
        case DATA_TYPE_VOID:
            std::cout << "[CPP] Generating code for array variable\n";
            varType = llvm::ArrayType::get(builder.getInt32Ty(), node->data.varDecl.initializer->data.arrayLiteral.elementCount);
            initialValue = llvm::ConstantArray::get(
                llvm::cast<llvm::ArrayType>(varType),
                llvm::ArrayRef<llvm::Constant*>(generateArrayElements(node->data.varDecl.initializer))
            );
            break;
        
        case DATA_TYPE_UNKNOWN:
            std::cerr << "[CPP] Error: Cannot declare variable of type unknown\n";
            return;
        default:
            std::cerr << "[CPP - ERROR] Invalid data type for variable: " << varName << "\n";
            return;
    }

    llvm::Value* finalValue = initialValue ? initialValue : computedValue;

    if (!finalValue) {
        std::cerr << "[CPP] Error: Initial value is null for variable: " << varName << "\n";
        finalValue = llvm::Constant::getNullValue(varType);
    }

    std::cout << "[CPP] Varname is: " << varName << "\n";
if (node->data.varDecl.isGlobal) {
    llvm::GlobalVariable* globalVar = new llvm::GlobalVariable(
        *module,
        varType,
        false,
        llvm::GlobalValue::ExternalLinkage,
        llvm::Constant::getNullValue(varType),  // Initialize with null value
        llvm::Twine(varName)
    );
    if (globalVar) {
        std::cout << "[CPP] Created global variable: " << varName << "\n";
        namedValues[varName] = globalVar;

        std::cout << "[CPP] Loading global variable value\n";

        // If there's an initial value, store it
        if (finalValue) {
            std::cout << "[CPP] Storing initial value for global variable: " << varName << "\n";
            std::cout << "[CPP] <LLVM VALUE> Final value type: " << LLVMTypeToString(finalValue->getType()) << "\n";
            std::cout << "[CPP] <LLVM VALUE> Global variable type: " << LLVMTypeToString(globalVar->getValueType()) << "\n";
            
            if (finalValue->getType() != globalVar->getValueType()) {
                std::cout << "[CPP] Type mismatch, attempting to cast\n";
                finalValue = builder.CreateBitCast(finalValue, globalVar->getValueType());
            }
            
            if (llvm::isa<llvm::Constant>(finalValue)) {
                globalVar->setInitializer(llvm::cast<llvm::Constant>(finalValue));
            } else {
                builder.CreateStore(finalValue, globalVar);
            }
        }
    } else {
        std::cerr << "[CPP] Error: Failed to create global variable: " << varName << "\n";
    }
} else {
    // ... existing local variable code ...
}

    std::cout << "[CPP] Completed code generation for variable declaration\n";
}
// </generateVarDeclaration>


// <createVariableDeclaration>
llvm::Value* CodeGen::createVariableDeclaration(ASTNode* node) {
    std::cout << "[CPP] Creating variable declaration\n";
    llvm::Type* varType = getLLVMType(node->data.varDecl.dataType, false);
    llvm::Value* varAlloca = builder.CreateAlloca(varType, nullptr, node->data.varDecl.name);
    llvm::Value* initValue = nullptr;

    if (node->data.varDecl.initializer) {
        switch (node->data.varDecl.dataType) {
            case DATA_TYPE_INT:
                std::cout << "[CPP] Creating int variable\n";
                initValue = createNumber(node->data.varDecl.initializer->data.literalExpression.intValue);
                builder.CreateStore(initValue, varAlloca);
                break;
            case DATA_TYPE_STRING:
                std::cout << "[CPP] Creating string variable\n";
                initValue = createString(node->data.varDecl.initializer->data.literalExpression.stringValue);
                builder.CreateStore(initValue, varAlloca);
                break;
            default:
                std::cerr << "[CPP] Error: Unsupported data type for variable declaration\n";
                return nullptr;
        }
        builder.CreateStore(initValue, varAlloca);
    }

    namedValues[node->data.varDecl.name] = varAlloca;
    return varAlloca;
}



// <generateArrayElements>
std::vector<llvm::Constant*> CodeGen::generateArrayElements(ASTNode* arrayLiteral) {
    std::vector<llvm::Constant*> elements;
    for (int i = 0; i < arrayLiteral->data.arrayLiteral.elementCount; i++) {
        ASTNode* element = arrayLiteral->data.arrayLiteral.elements[i];
        if (element->type == NODE_LITERAL_EXPR && element->data.literalExpression.dataType == DATA_TYPE_INT) {
            elements.push_back(llvm::ConstantInt::get(builder.getInt32Ty(), element->data.literalExpression.intValue));
        }
    }
    return elements;
}
// </generateArrayElements>
