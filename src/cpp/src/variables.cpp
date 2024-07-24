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

    std::cout << "[CPP] Created global variable: " << varName << "\n";
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
    llvm::Constant* initialValue = nullptr;
    llvm::Value* computedValue = nullptr; // For non-literal initializers
    char* varName = node->data.varDecl.name;

    std::cout << "[CPP]! Variable Name: " << node->data.varDecl.name << "\n";
    std::cout << "[CPP]! Variable Type: " << CryoDataTypeToString(node->data.varDecl.dataType) << "\n";

    switch (node->data.varDecl.dataType) {
        case DATA_TYPE_STRING:
            std::cout << "[CPP] Generating code for string variable\n";
            if (node->data.varDecl.initializer->type == NODE_LITERAL_EXPR) {
                std::cout << "[CPP] Initializer is a literal expression\n";
                
                initialValue = llvm::cast<llvm::Constant>(createString(node->data.varDecl.initializer->data.literalExpression.stringValue));
                varType = builder.getInt8Ty()->getPointerTo();
            } else if (node->data.varDecl.initializer->type == NODE_VAR_NAME) {
                std::cout << "[CPP] Initializer is a variable name\n";
                llvm::GlobalVariable* refVar = module->getGlobalVariable(node->data.varDecl.initializer->data.varName.varName);
                if (refVar) {
                    initialValue = llvm::ConstantExpr::getPointerCast(refVar, refVar->getType());
                    varType = refVar->getValueType();
                } else {
                    std::cerr << "[CPP] Error: Referenced variable not found: " << node->data.varDecl.initializer->data.varName.varName << "\n";
                    return;
                }
            } else if(node->data.varDecl.initializer->type == NODE_STRING_LITERAL) {
                std::cout << "[CPP] Initializer is a string literal\n";

                char* stringValue = node->data.varDecl.initializer->data.literalExpression.stringValue;
                std::cout << "[CPP] String Value: " << stringValue << "\n";
                
                initialValue = llvm::cast<llvm::Constant>(createString(node->data.varDecl.initializer->data.literalExpression.stringValue));
                varType = builder.getInt8Ty()->getPointerTo();
            }
            else {
                std::cerr << "[CPP] Unknown expression type for string variable initializer\n";
                return;
            }
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
        return;
    }

    std::cout << "[CPP] Varname is: " << varName << "\n";
    if (node->data.varDecl.isGlobal) {
        std::cout << "[CPP] Creating global variable\n";
        llvm::GlobalVariable* globalVar = new llvm::GlobalVariable(
            *module,
            varType,
            false,
            llvm::GlobalValue::ExternalLinkage,
            llvm::cast<llvm::Constant>(finalValue),
            llvm::Twine(varName)
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
        builder.CreateStore(finalValue, alloca);
        std::cout << "[CPP] Stored initial value in alloca\n";
        namedValues[varName] = alloca;
        std::cout << "[CPP] Stored alloca in namedValues\n";
    }
}
// </generateVarDeclaration>



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


// <generateCodeForArrayLiteral>
void CodeGen::generateCodeForArrayLiteral(ASTNode* node) {
    if (node->type != NODE_ARRAY_LITERAL) {
        std::cerr << "[CodeGen] [ERROR] Expected array literal node, got " << node->type << "\n";
        return;
    }

    std::cout << "[CodeGen] Generating code for array literal\n";

    for (int i = 0; i < node->data.arrayLiteral.elementCount; i++) {
        generateCode(node->data.arrayLiteral.elements[i]);
    }

    std::cout << "[CodeGen] Completed code generation for array literal\n";
}
// </generateCodeForArrayLiteral>

} // namespace Cryo
