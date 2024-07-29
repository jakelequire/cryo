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

std::string CryoTypes::LLVMTypeToString(llvm::Type* type) {
    std::string typeStr;
    llvm::raw_string_ostream rso(typeStr);
    type->print(rso);
    return rso.str();
}

llvm::Type* CryoTypes::getLLVMType(CryoDataType type) {
    switch(type) {
        case DATA_TYPE_INT:
            return llvm::Type::getInt32Ty(context);
        case DATA_TYPE_FLOAT:
            return llvm::Type::getFloatTy(context);
        case DATA_TYPE_STRING:
            return llvm::Type::getInt8Ty(context);
        case DATA_TYPE_BOOLEAN:
            return llvm::Type::getInt1Ty(context);
        case DATA_TYPE_VOID:
            return llvm::Type::getVoidTy(context);
        default:
            return nullptr;
    }
}


llvm::Type* CryoTypes::createLLVMConstantType(CryoDataType type) {
    if (type == DATA_TYPE_UNKNOWN) {
        std::cerr << "[Types] Error: Unknown type\n";
        return nullptr;
    }

    switch(type) {
        case DATA_TYPE_INT:
            return llvm::Type::getInt32Ty(context);
        case DATA_TYPE_FLOAT:
            return llvm::Type::getFloatTy(context);
        case DATA_TYPE_STRING:
            return llvm::Type::getInt8Ty(context); // String is represented as i8
        case DATA_TYPE_BOOLEAN:
            return llvm::Type::getInt1Ty(context);
        case DATA_TYPE_VOID:
            return llvm::Type::getVoidTy(context);
        default:
            std::cerr << "[Types] Error: Unsupported type\n";
            return nullptr;
    }
}

llvm::PointerType* CryoTypes::createLLVMPointerType(CryoDataType type) {
    llvm::Type* baseType = createLLVMConstantType(type);
    if (!baseType) {
        return nullptr;
    }
    return llvm::PointerType::getUnqual(baseType);
}

llvm::ArrayType* CryoTypes::createLLVMArrayType(CryoDataType elementType, unsigned int size) {
    llvm::Type* baseType = createLLVMConstantType(elementType);
    if (!baseType) {
        return nullptr;
    }
    return llvm::ArrayType::get(baseType, size);
}

llvm::StructType* CryoTypes::createLLVMStructType(const std::vector<CryoDataType>& memberTypes, const std::string& name) {
    std::vector<llvm::Type*> llvmTypes;
    for (const auto& type : memberTypes) {
        llvm::Type* memberType = createLLVMConstantType(type);
        if (!memberType) {
            return nullptr;
        }
        llvmTypes.push_back(memberType);
    }
    
    if (name.empty()) {
        return llvm::StructType::get(context, llvmTypes);
    } else {
        return llvm::StructType::create(context, llvmTypes, name);
    }
}


llvm::FunctionType* CryoTypes::createLLVMFunctionType(CryoDataType returnType, const std::vector<CryoDataType>& paramTypes, bool isVarArg) {
    llvm::Type* llvmReturnType = createLLVMConstantType(returnType);
    if (!llvmReturnType) {
        return nullptr;
    }

    std::vector<llvm::Type*> llvmParamTypes;
    for (const auto& type : paramTypes) {
        llvm::Type* paramType = createLLVMConstantType(type);
        if (!paramType) {
            return nullptr;
        }
        llvmParamTypes.push_back(paramType);
    }

    return llvm::FunctionType::get(llvmReturnType, llvmParamTypes, isVarArg);
}

} // namespace Cryo

