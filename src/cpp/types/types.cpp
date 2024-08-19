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
        CryoDebugger &cryoDebugger = compiler.getDebugger();

        char *typeStr = CryoDataTypeToString(type);
        cryoDebugger.logMessage("INFO", __LINE__, "Types", "Getting LLVM type for " + std::string(typeStr));
        // Test if the context exists at all:

        CryoContext &cryoContext = compiler.getContext();

        switch (type)
        {
        case DATA_TYPE_INT:
            cryoDebugger.logMessage("INFO", __LINE__, "Types", "Returning int type");
            return llvm::Type::getInt32Ty(cryoContext.context);
        case DATA_TYPE_FLOAT:
            cryoDebugger.logMessage("INFO", __LINE__, "Types", "Returning float type");
            return llvm::Type::getFloatTy(cryoContext.context);
        case DATA_TYPE_STRING:
            cryoDebugger.logMessage("INFO", __LINE__, "Types", "Returning string type");
            return llvm::Type::getInt8Ty(cryoContext.context);
        case DATA_TYPE_BOOLEAN:
            cryoDebugger.logMessage("INFO", __LINE__, "Types", "Returning boolean type");
            return llvm::Type::getInt1Ty(cryoContext.context);
        case DATA_TYPE_VOID:
            cryoDebugger.logMessage("INFO", __LINE__, "Types", "Returning void type");
            return llvm::Type::getVoidTy(cryoContext.context);
        case DATA_TYPE_ARRAY:
            cryoDebugger.logMessage("INFO", __LINE__, "Types", "Returning array type");
            return llvm::ArrayType::get(llvm::Type::getInt32Ty(cryoContext.context), 0);
        case DATA_TYPE_UNKNOWN:
            cryoDebugger.logMessage("ERROR", __LINE__, "Types", "Unknown data type");
            return nullptr;

        default:
            cryoDebugger.logMessage("ERROR", __LINE__, "Types", "Unsupported data type");
            return nullptr;
        }
    }

    llvm::Type *CryoTypes::createLLVMConstantType(CryoDataType type)
    {
        CryoDebugger &cryoDebugger = compiler.getDebugger();
        if (type == DATA_TYPE_UNKNOWN)
        {
            cryoDebugger.logMessage("ERROR", __LINE__, "Types", "Unknown data type");
            return nullptr;
        }

        CryoContext &cryoContext = compiler.getContext();

        switch (type)
        {
        case DATA_TYPE_INT:
            cryoDebugger.logMessage("INFO", __LINE__, "Types", "Returning int type");
            return llvm::Type::getInt32Ty(cryoContext.context);
        case DATA_TYPE_FLOAT:
            cryoDebugger.logMessage("INFO", __LINE__, "Types", "Returning float type");
            return llvm::Type::getFloatTy(cryoContext.context);
        case DATA_TYPE_STRING:
            cryoDebugger.logMessage("INFO", __LINE__, "Types", "Returning string type");
            return llvm::Type::getInt8Ty(cryoContext.context); // String is represented as i8
        case DATA_TYPE_BOOLEAN:
            cryoDebugger.logMessage("INFO", __LINE__, "Types", "Returning boolean type");
            return llvm::Type::getInt1Ty(cryoContext.context);
        case DATA_TYPE_VOID:
            cryoDebugger.logMessage("INFO", __LINE__, "Types", "Returning void type");
            return llvm::Type::getVoidTy(cryoContext.context);
        default:
            cryoDebugger.logMessage("ERROR", __LINE__, "Types", "Unsupported data type");
            return nullptr;
        }
    }

    llvm::PointerType *CryoTypes::createLLVMPointerType(CryoDataType type)
    {
        CryoDebugger &cryoDebugger = compiler.getDebugger();
        llvm::Type *baseType = createLLVMConstantType(type);
        if (!baseType)
        {
            return nullptr;
        }

        std::cout << "[INFO] \t@" << __LINE__ << "\t{Types}\t\t Creating pointer type for " << CryoDataTypeToString(type) << std::endl;
        return llvm::PointerType::get(baseType, 0);
    }

    llvm::ArrayType *CryoTypes::createLLVMArrayType(CryoDataType elementType, unsigned int size)
    {
        CryoDebugger &cryoDebugger = compiler.getDebugger();
        llvm::Type *baseType = createLLVMConstantType(elementType);
        if (!baseType)
        {
            return nullptr;
        }

        std::cout << "[INFO] \t@" << __LINE__ << "\t{Types}\t\t Creating array type for " << CryoDataTypeToString(elementType) << " with size " << size << std::endl;
        return llvm::ArrayType::get(baseType, size);
    }

    llvm::StructType *CryoTypes::createLLVMStructType(const std::vector<CryoDataType> &memberTypes, const std::string &name)
    {
        CryoContext &cryoContext = compiler.getContext();
        CryoDebugger &cryoDebugger = compiler.getDebugger();

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
            cryoDebugger.logMessage("INFO", __LINE__, "Types", "Creating struct type");
            return llvm::StructType::get(cryoContext.context, llvmTypes);
        }
        else
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Types", "Creating named struct type");
            return llvm::StructType::create(cryoContext.context, llvmTypes, name);
        }
    }

    llvm::FunctionType *CryoTypes::createLLVMFunctionType(CryoDataType returnType, const std::vector<CryoDataType> &paramTypes, bool isVarArg)
    {
        CryoDebugger &cryoDebugger = compiler.getDebugger();
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

        std::cout << "[INFO] \t@" << __LINE__ << "\t{Types}\t\t Creating function type\n";
        return llvm::FunctionType::get(llvmReturnType, llvmParamTypes, isVarArg);
    }

    llvm::Constant *CryoTypes::convertLLVMPtrToConstant(llvm::Value *ptr)
    {
        CryoDebugger &cryoDebugger = compiler.getDebugger();
        llvm::Constant *constant = llvm::dyn_cast<llvm::Constant>(ptr);
        if (!constant)
        {
            cryoDebugger.logMessage("ERROR", __LINE__, "Types", "Failed to convert pointer to constant");
            return nullptr;
        }
        return constant;
    }

    llvm::Constant *CryoTypes::createLLVMConstant(CryoDataType type, const std::string &value)
    {
        CryoContext &cryoContext = compiler.getContext();
        CryoDebugger &cryoDebugger = compiler.getDebugger();

        switch (type)
        {
        case DATA_TYPE_INT:
            cryoDebugger.logMessage("INFO", __LINE__, "Types", "Creating int constant");
            return llvm::ConstantInt::get(cryoContext.context, llvm::APInt(32, value, 10));
        case DATA_TYPE_FLOAT:
            cryoDebugger.logMessage("INFO", __LINE__, "Types", "Creating float constant");
            return llvm::ConstantFP::get(cryoContext.context, llvm::APFloat(std::stof(value)));
        case DATA_TYPE_STRING:
            cryoDebugger.logMessage("INFO", __LINE__, "Types", "Creating string constant");
            return llvm::ConstantDataArray::getString(cryoContext.context, value);
        case DATA_TYPE_BOOLEAN:
            cryoDebugger.logMessage("INFO", __LINE__, "Types", "Creating boolean constant");
            return llvm::ConstantInt::get(cryoContext.context, llvm::APInt(1, value, 10));
        case DATA_TYPE_VOID:
            cryoDebugger.logMessage("INFO", __LINE__, "Types", "Creating void constant");
            return nullptr;
        default:
            cryoDebugger.logMessage("ERROR", __LINE__, "Types", "Unsupported data type");
            return nullptr;
        }
    }

} // namespace Cryo
