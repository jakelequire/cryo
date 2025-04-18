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
#include "codegen/codegenDebug.hpp"
#include "tools/logger/logger_config.h"

namespace Cryo
{

    void CodeGenDebug::printLLVMValue(llvm::Value *value)
    {
        if (value == nullptr)
        {
            std::cerr << "LLVM Value is null" << std::endl;
            return;
        }

        std::cout << "\n";
        std::cout << ">>===-----------<LLVM Value Node>-----------===<<" << std::endl;
        std::string valueStr;
        llvm::raw_string_ostream rso(valueStr);

        // This prints out the line of IR that was generated
        value->print(rso);
        std::cout << "Inst:" << "" << rso.str() << std::endl;

        // Print out the type of the value
        std::string tyName = LLVMTypeIDToString(value->getType());
        std::cout << "Type: " << tyName << std::endl;

        // Print out the name of the value
        std::string valName = value->getName().str();
        std::cout << "Name: " << valName << std::endl;

        // Print out the address of the value
        std::cout << "Address: " << value << std::endl;

        std::cout << ">>===--------------------------------------===<<" << std::endl;
        std::cout << "\n";
    }

    void CodeGenDebug::printLLVMType(llvm::Type *type)
    {
        if (type == nullptr)
        {
            std::cerr << "LLVM Type is null" << std::endl;
            return;
        }

        std::cout << "\n";
        std::cout << ">>===-----------<LLVM Type Node>-----------===<<" << std::endl;

        // Print out the name of the type
        std::string typeName = LLVMTypeIDToString(type);
        std::cout << "Name: " << typeName << std::endl;

        // Print out the instruction of the type
        std::string typeInst;
        llvm::raw_string_ostream rso(typeInst);
        type->print(rso);
        std::cout << "Inst: " << rso.str() << std::endl;

        // Print out the address of the type
        std::cout << "Address: " << type << std::endl;
        std::cout << ">>===--------------------------------------===<<" << std::endl;
    }

    void CodeGenDebug::printLLVMFunction(llvm::Function *function)
    {
        if (function == nullptr)
        {
            std::cerr << "LLVM Function is null" << std::endl;
            return;
        }

        std::cout << "\n";
        std::cout << ">>===-----------<LLVM Function Node>-----------===<<" << std::endl;

        // Print out the name of the function
        std::string funcName = function->getName().str();
        std::cout << "Name: " << funcName << std::endl;

        // Print out the instruction of the function
        std::string funcInst;
        llvm::raw_string_ostream rso(funcInst);
        function->print(rso);
        std::cout << "Inst: " << rso.str() << std::endl;

        // Print out the address of the function
        std::cout << "Address: " << function << std::endl;
        std::cout << ">>===-------------------------------------------===<<" << std::endl;
    }

    void CodeGenDebug::printLLVMStruct(llvm::StructType *structType)
    {
        if (structType == nullptr)
        {
            return;
        }

        std::cout << "\n";
        std::cout << ">>===-----------<LLVM Struct Type Node>-----------===<<" << std::endl;

        // Print out the name of the struct
        std::string structName = structType->getName().str();
        std::cout << "Name: " << structName << std::endl;

        // Print out the instruction of the struct
        std::string structInst;
        llvm::raw_string_ostream rso(structInst);
        structType->print(rso);
        std::cout << "Inst: " << rso.str() << std::endl;

        // Print out the address of the struct
        std::cout << "Address: " << structType << std::endl;
        std::cout << ">>===-------------------------------------------===<<" << std::endl;
    }

    void CodeGenDebug::printLLVMInstruction(llvm::Instruction *inst)
    {
        if (inst == nullptr)
        {
            std::cerr << "LLVM Instruction is null" << std::endl;
            return;
        }

        std::cout << "\n";
        std::cout << ">>===-----------<LLVM Instruction Node>-----------===<<" << std::endl;

        // Print out the instruction of the instruction
        std::string instStr;
        llvm::raw_string_ostream rso(instStr);
        inst->print(rso);
        std::cout << "Inst: " << rso.str() << std::endl;

        // Print out the address of the instruction
        std::cout << "Address: " << inst << std::endl;
        std::cout << ">>===---------------------------------------------===<<" << std::endl;
    }

    void CodeGenDebug::printLLVMAllocaInst(llvm::AllocaInst *allocaInst)
    {
        if (allocaInst == nullptr)
        {
            std::cerr << "LLVM Alloca Instruction is null" << std::endl;
            return;
        }

        std::cout << "\n";
        std::cout << ">>===-----------<LLVM Alloca Instruction Node>-----------===<<" << std::endl;

        // Print out the instruction of the alloca instruction
        std::string allocaInstStr;
        llvm::raw_string_ostream rso(allocaInstStr);
        allocaInst->print(rso);
        std::cout << "Inst: " << rso.str() << std::endl;

        // Print out the address of the alloca instruction
        std::cout << "Address: " << allocaInst << std::endl;
        std::cout << ">>===---------------------------------------------------===<<" << std::endl;
    }

    std::string CodeGenDebug::LLVMTypeIDToString(llvm::Type *type)
    {
        if (!type || type == nullptr)
        {
            std::cerr << "LLVM Type is null" << std::endl;
            return "Unknown";
        }

        llvm::Type::TypeID tyID = type->getTypeID();
        switch (tyID)
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
        case llvm::Type::TypeID::X86_MMXTyID:
            return "X86_MMXTyID";
        case llvm::Type::TypeID::TokenTyID:
            return "TokenTyID";
        case llvm::Type::TypeID::IntegerTyID:
            return "IntegerTyID";
        case llvm::Type::TypeID::FunctionTyID:
            return "FunctionTyID";
        case llvm::Type::TypeID::StructTyID:
            return "StructTyID";
        case llvm::Type::TypeID::ArrayTyID:
            return "ArrayTyID";
        case llvm::Type::TypeID::FixedVectorTyID:
            return "FixedVectorTyID";
        case llvm::Type::TypeID::ScalableVectorTyID:
            return "ScalableVectorTyID";
        case llvm::Type::TypeID::VoidTyID:
            return "VoidTyID";
        case llvm::Type::TypeID::LabelTyID:
            return "LabelTyID";
        case llvm::Type::TypeID::MetadataTyID:
            return "MetadataTyID";
        case llvm::Type::TypeID::X86_AMXTyID:
            return "X86_AMXTyID";
        case llvm::Type::TypeID::PointerTyID:
            return "PointerTyID";
        default:
            return "Unknown: " + std::to_string(tyID);
        }
    }

} // namespace Cryo
