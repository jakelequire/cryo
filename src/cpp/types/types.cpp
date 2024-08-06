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

namespace Cryo
{

    std::string CryoTypes::LLVMTypeIdToString(llvm::Type::TypeID type)
    {
        switch (type)
        {
        case llvm::Type::TypeID::HalfTyID:
            return "HalfTyID";
        case llvm::Type::TypeID::BFloatTyID:
            return "BFloatTyID";
        case llvm::Type::TypeID::FloatTyID:
            return "FloatTyID";
        case llvm::Type::TypeID::DoubleTyID:
            return "DoubleTyID";
        case llvm::Type::TypeID::X86_FP80TyID:
            return "X86_FP80TyID";
        case llvm::Type::TypeID::FP128TyID:
            return "FP128TyID";
        case llvm::Type::TypeID::PPC_FP128TyID:
            return "PPC_FP128TyID";
        case llvm::Type::TypeID::VoidTyID:
            return "VoidTyID";
        case llvm::Type::TypeID::LabelTyID:
            return "LabelTyID";
        case llvm::Type::TypeID::MetadataTyID:
            return "MetadataTyID";
        case llvm::Type::TypeID::X86_MMXTyID:
            return "X86_MMXTyID";
        case llvm::Type::TypeID::X86_AMXTyID:
            return "X86_AMXTyID";
        case llvm::Type::TypeID::TokenTyID:
            return "TokenTyID";
        case llvm::Type::TypeID::IntegerTyID:
            return "IntegerTyID";
        case llvm::Type::TypeID::FunctionTyID:
            return "FunctionTyID";
        case llvm::Type::TypeID::PointerTyID:
            return "PointerTyID";
        case llvm::Type::TypeID::StructTyID:
            return "StructTyID";
        case llvm::Type::TypeID::ArrayTyID:
            return "ArrayTyID";
        case llvm::Type::TypeID::FixedVectorTyID:
            return "FixedVectorTyID";
        case llvm::Type::TypeID::ScalableVectorTyID:
            return "ScalableVectorTyID";
        case llvm::Type::TypeID::TypedPointerTyID:
            return "TypedPointerTyID";
        case llvm::Type::TypeID::TargetExtTyID:
            return "TargetExtTyID";
        default:
            return "Unknown";
        }
    }

    std::string CryoTypes::LLVMTypeToString(llvm::Type *type)
    {
        std::string typeStr;
        llvm::raw_string_ostream rso(typeStr);
        type->print(rso);
        return rso.str();
    }

    llvm::Type *CryoTypes::getLLVMType(CryoDataType type)
    {
        std::cout << "[Types] Getting LLVM Type for " << CryoDataTypeToString(type) << std::endl;
        // Test if the context exists at all:

        CryoContext &cryoContext = compiler.getContext();

        switch (type)
        {
        case DATA_TYPE_INT:
            std::cout << "[Types] Returning int type\n";
            return llvm::Type::getInt32Ty(cryoContext.context);
        case DATA_TYPE_FLOAT:
            std::cout << "[Types] Returning float type\n";
            return llvm::Type::getFloatTy(cryoContext.context);
        case DATA_TYPE_STRING:
            std::cout << "[Types] Returning string type\n";
            return llvm::Type::getInt8Ty(cryoContext.context);
        case DATA_TYPE_BOOLEAN:
            std::cout << "[Types] Returning boolean type\n";
            return llvm::Type::getInt1Ty(cryoContext.context);
        case DATA_TYPE_VOID:
            std::cout << "[Types] Returning void type\n";
            return llvm::Type::getVoidTy(cryoContext.context);
        case DATA_TYPE_ARRAY:
            std::cout << "[Types] Returning array type\n";
            return llvm::ArrayType::get(llvm::Type::getInt32Ty(cryoContext.context), 0);
        case DATA_TYPE_UNKNOWN:
            std::cerr << "[Types] Error: Unknown type\n";
            return nullptr;

        default:
            std::cerr << "[Types] Error: Unsupported type\n";
            return nullptr;
        }
    }

    llvm::Type *CryoTypes::createLLVMConstantType(CryoDataType type)
    {
        if (type == DATA_TYPE_UNKNOWN)
        {
            std::cerr << "[Types] Error: Unknown type\n";
            return nullptr;
        }

        CryoContext &cryoContext = compiler.getContext();

        switch (type)
        {
        case DATA_TYPE_INT:
            return llvm::Type::getInt32Ty(cryoContext.context);
        case DATA_TYPE_FLOAT:
            return llvm::Type::getFloatTy(cryoContext.context);
        case DATA_TYPE_STRING:
            return llvm::Type::getInt8Ty(cryoContext.context); // String is represented as i8
        case DATA_TYPE_BOOLEAN:
            return llvm::Type::getInt1Ty(cryoContext.context);
        case DATA_TYPE_VOID:
            return llvm::Type::getVoidTy(cryoContext.context);
        default:
            std::cerr << "[Types] Error: Unsupported type\n";
            return nullptr;
        }
    }

    llvm::PointerType *CryoTypes::createLLVMPointerType(CryoDataType type)
    {
        if(!type) {
            std::cerr << "[Types] Error: Unknown type\n";
            return nullptr;
        }

        switch(type) {
            case DATA_TYPE_INT:
                return llvm::Type::getInt32PtrTy(compiler.getContext().context);
            case DATA_TYPE_FLOAT:
                return llvm::Type::getFloatPtrTy(compiler.getContext().context);
            case DATA_TYPE_STRING:
                return llvm::Type::getInt8PtrTy(compiler.getContext().context);
            case DATA_TYPE_BOOLEAN:
                return llvm::Type::getInt1PtrTy(compiler.getContext().context);
            case DATA_TYPE_VOID:
                return llvm::Type::getVoidPtrTy(compiler.getContext().context);
            default:
                std::cerr << "[Types] Error: Unsupported type\n";
                return nullptr;
        }
    }

    llvm::ArrayType *CryoTypes::createLLVMArrayType(CryoDataType elementType, unsigned int size)
    {
        llvm::Type *baseType = createLLVMConstantType(elementType);
        if (!baseType)
        {
            return nullptr;
        }
        return llvm::ArrayType::get(baseType, size);
    }

    llvm::StructType *CryoTypes::createLLVMStructType(const std::vector<CryoDataType> &memberTypes, const std::string &name)
    {
        CryoContext &cryoContext = compiler.getContext();

        std::vector<llvm::Type *> llvmTypes;
        for (const auto &type : memberTypes)
        {
            llvm::Type *memberType = createLLVMConstantType(type);
            if (!memberType)
            {
                return nullptr;
            }
            llvmTypes.push_back(memberType);
        }

        if (name.empty())
        {
            return llvm::StructType::get(cryoContext.context, llvmTypes);
        }
        else
        {
            return llvm::StructType::create(cryoContext.context, llvmTypes, name);
        }
    }

    llvm::FunctionType *CryoTypes::createLLVMFunctionType(CryoDataType returnType, const std::vector<CryoDataType> &paramTypes, bool isVarArg)
    {
        llvm::Type *llvmReturnType = createLLVMConstantType(returnType);
        if (!llvmReturnType)
        {
            return nullptr;
        }

        std::vector<llvm::Type *> llvmParamTypes;
        for (const auto &type : paramTypes)
        {
            llvm::Type *paramType = createLLVMConstantType(type);
            if (!paramType)
            {
                return nullptr;
            }
            llvmParamTypes.push_back(paramType);
        }

        return llvm::FunctionType::get(llvmReturnType, llvmParamTypes, isVarArg);
    }

} // namespace Cryo
