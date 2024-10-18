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
#include "cpp/codegen.hpp"

namespace Cryo
{
    llvm::Type *Types::getType(CryoDataType type, int length)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        if (length == 0)
        {
            debugger.logMessage("INFO", __LINE__, "Types", "Converting simple type to LLVM type");
            return this->convertSimpleType(type);
        }
        else
        {
            debugger.logMessage("INFO", __LINE__, "Types", "Converting complex type to LLVM type");
            return this->convertComplexType(type, length);
        }
    }

    llvm::Type *Types::getLiteralType(LiteralNode *literal)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Types", "Getting literal type");

        if (literal->dataType == DATA_TYPE_UNKNOWN)
        {
            debugger.logMessage("INFO", __LINE__, "Types", "Unknown type");
            CONDITION_FAILED;
        }

        CryoDataType type = literal->dataType;
        int len = literal->length;
        switch (type)
        {
        case DATA_TYPE_INT:
        {
            debugger.logMessage("INFO", __LINE__, "Types", "Returning int type");
            return llvm::Type::getInt32Ty(CryoContext::getInstance().context);
        }
        case DATA_TYPE_STRING:
        {
            // [i8 x len]
            debugger.logMessage("INFO", __LINE__, "Types", "Returning string type");
            return llvm::ArrayType::get(llvm::Type::getInt8Ty(CryoContext::getInstance().context), len);
        }
        case DATA_TYPE_FLOAT:
        {
            debugger.logMessage("INFO", __LINE__, "Types", "Returning float type");
            return llvm::Type::getFloatTy(CryoContext::getInstance().context);
        }
        case DATA_TYPE_BOOLEAN:
        {
            debugger.logMessage("INFO", __LINE__, "Types", "Returning boolean type");
            return llvm::Type::getInt1Ty(CryoContext::getInstance().context);
        }
        default:
        {
            debugger.logMessage("INFO", __LINE__, "Types", "Unknown type");
            return nullptr;
        }
        }
    }

    llvm::Type *Types::getReturnType(CryoDataType type)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Types", "Getting return type");

        if (type == DATA_TYPE_UNKNOWN)
        {
            debugger.logMessage("INFO", __LINE__, "Types", "Unknown type");
            return nullptr;
        }

        switch (type)
        {
        case DATA_TYPE_INT:
            debugger.logMessage("INFO", __LINE__, "Types", "Returning int type");
            return llvm::Type::getInt32Ty(CryoContext::getInstance().context);
        case DATA_TYPE_STRING:
            debugger.logMessage("INFO", __LINE__, "Types", "Returning string type");
            return llvm::Type::getInt8Ty(CryoContext::getInstance().context);
        case DATA_TYPE_FLOAT:
            debugger.logMessage("INFO", __LINE__, "Types", "Returning float type");
            return llvm::Type::getFloatTy(CryoContext::getInstance().context);
        case DATA_TYPE_BOOLEAN:
            debugger.logMessage("INFO", __LINE__, "Types", "Returning boolean type");
            return llvm::Type::getInt1Ty(CryoContext::getInstance().context);
        case DATA_TYPE_VOID:
            debugger.logMessage("INFO", __LINE__, "Types", "Returning void type");
            return llvm::Type::getVoidTy(CryoContext::getInstance().context);
        case DATA_TYPE_INT_ARRAY:
            debugger.logMessage("INFO", __LINE__, "Types", "Returning int array type");
            return llvm::ArrayType::get(llvm::Type::getInt32Ty(CryoContext::getInstance().context), 0);
        case DATA_TYPE_STRING_ARRAY:
            debugger.logMessage("INFO", __LINE__, "Types", "Returning string array type");
            return llvm::ArrayType::get(llvm::Type::getInt8Ty(CryoContext::getInstance().context), 0);
        case DATA_TYPE_ARRAY:
            debugger.logMessage("INFO", __LINE__, "Types", "Returning array type");
            return llvm::ArrayType::get(llvm::Type::getInt32Ty(CryoContext::getInstance().context), 0);
        }
    }

    // -----------------------------------------------------------------------------------------------

    llvm::Type *Types::convertSimpleType(CryoDataType type)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Types", "Converting simple type to LLVM type");
        switch (type)
        {
        case DATA_TYPE_INT:
            debugger.logMessage("INFO", __LINE__, "Types", "Converting int to LLVM type");
            return llvm::Type::getInt32Ty(CryoContext::getInstance().context);

        case DATA_TYPE_STRING:
            debugger.logMessage("INFO", __LINE__, "Types", "Converting string to LLVM type");
            return llvm::Type::getInt8Ty(CryoContext::getInstance().context)->getPointerTo();

        case DATA_TYPE_FLOAT:
            debugger.logMessage("INFO", __LINE__, "Types", "Converting float to LLVM type");
            return llvm::Type::getFloatTy(CryoContext::getInstance().context);

        case DATA_TYPE_BOOLEAN:
            debugger.logMessage("INFO", __LINE__, "Types", "Converting boolean to LLVM type");
            return llvm::Type::getInt1Ty(CryoContext::getInstance().context);

        case DATA_TYPE_VOID:
            debugger.logMessage("INFO", __LINE__, "Types", "Converting void to LLVM type");
            return llvm::Type::getVoidTy(CryoContext::getInstance().context);

        case DATA_TYPE_INT_ARRAY:
            debugger.logMessage("INFO", __LINE__, "Types", "Converting int array to LLVM type");
            return llvm::ArrayType::get(llvm::Type::getInt32Ty(CryoContext::getInstance().context), 0);

        case DATA_TYPE_ANY:
            debugger.logMessage("INFO", __LINE__, "Types", "Converting any to LLVM type");
            return llvm::Type::getInt8Ty(CryoContext::getInstance().context)->getPointerTo();

        // ================================
        // New Types

        // Integer Types
        case DATA_TYPE_SINT8:
        case DATA_TYPE_SINT16:
        case DATA_TYPE_SINT32:
        case DATA_TYPE_SINT64:
        case DATA_TYPE_UINT8:
        case DATA_TYPE_UINT16:
        case DATA_TYPE_UINT32:
        case DATA_TYPE_UINT64:

        // Arrays
        case DATA_TYPE_INT8_ARRAY:
        case DATA_TYPE_INT16_ARRAY:
        case DATA_TYPE_INT32_ARRAY:
        case DATA_TYPE_INT64_ARRAY:

        // Vectors
        case DATA_TYPE_DYN_VEC:

        default:
            debugger.logMessage("INFO", __LINE__, "Types", "Unknown type");
            return nullptr;
        }
    }

    // -----------------------------------------------------------------------------------------------

    llvm::Type *Types::convertComplexType(CryoDataType types, int length)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Types", "Converting complex type to LLVM type");
        switch (types)
        {
        case DATA_TYPE_STRING:
            debugger.logMessage("INFO", __LINE__, "Types", "Converting string to LLVM type");
            return llvm::ArrayType::get(llvm::Type::getInt8Ty(CryoContext::getInstance().context), length);

        case DATA_TYPE_ARRAY:
            debugger.logMessage("INFO", __LINE__, "Types", "Converting array to LLVM type");
            return llvm::ArrayType::get(llvm::Type::getInt32Ty(CryoContext::getInstance().context), length);

        case DATA_TYPE_INT_ARRAY:
            debugger.logMessage("INFO", __LINE__, "Types", "Converting int array to LLVM type");
            return llvm::ArrayType::get(llvm::Type::getInt32Ty(CryoContext::getInstance().context), length);

        case DATA_TYPE_STRING_ARRAY:
            debugger.logMessage("INFO", __LINE__, "Types", "Converting string array to LLVM type");
            return llvm::ArrayType::get(llvm::Type::getInt8Ty(CryoContext::getInstance().context), length);

        default:
            debugger.logMessage("INFO", __LINE__, "Types", "Unknown type");
            return nullptr;
        }
    }

    // -----------------------------------------------------------------------------------------------

    int Types::getLiteralValLength(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Arrays &arrays = compiler.getArrays();
        if (node->metaData->type == NODE_STRING_LITERAL)
        {
            debugger.logMessage("INFO", __LINE__, "Types", "Getting length of string literal");
            return strlen(node->data.literal->value.stringValue);
        }

        if (node->metaData->type == NODE_LITERAL_EXPR)
        {
            if (node->data.literal->dataType == DATA_TYPE_STRING)
            {
                debugger.logMessage("INFO", __LINE__, "Types", "Getting length of string literal");
                int _len = strlen(node->data.literal->value.stringValue);
                std::cout << "<TYPES> String Length: " << _len << std::endl;
                return _len;
            }

            if (node->data.literal->dataType == DATA_TYPE_INT)
            {
                return 0;
            }

            if (node->data.literal->dataType == DATA_TYPE_FLOAT)
            {
                return 0;
            }

            if (node->data.literal->dataType == DATA_TYPE_BOOLEAN)
            {
                return 0;
            }

            if (node->data.literal->dataType == DATA_TYPE_INT_ARRAY)
            {
                debugger.logMessage("INFO", __LINE__, "Types", "Getting length of int array literal");
                return node->data.array->elementCount;
            }
        }
        if (node->metaData->type == NODE_ARRAY_LITERAL)
        {
            debugger.logMessage("INFO", __LINE__, "Types", "Getting length of array literal");
            return arrays.getArrayLength(node);
        }

        if (node->metaData->type == NODE_VAR_DECLARATION)
        {
            if (node->data.varDecl->type == DATA_TYPE_STRING)
            {
                debugger.logMessage("INFO", __LINE__, "Types", "Getting length of string variable");
                return strlen(node->data.varDecl->initializer->data.literal->value.stringValue);
            }
            else
            {
                return 0;
            }
        }

        if (node->metaData->type == NODE_INDEX_EXPR)
        {
            debugger.logMessage("INFO", __LINE__, "Types", "Getting length of index expression");
            return 0;
        }

        char *typeNode = CryoNodeTypeToString(node->metaData->type);

        debugger.logMessage("ERROR", __LINE__, "Types", "Unknown node type");
        debugger.logMessage("ERROR", __LINE__, "Types", "Type: " + std::string(typeNode));
        return 0;
    }

    int Types::getLiteralValLength(LiteralNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        if (node->dataType == DATA_TYPE_STRING)
        {
            debugger.logMessage("INFO", __LINE__, "Types", "Getting length of string literal");
            return strlen(node->value.stringValue);
        }
        return 0;
    }

    // -----------------------------------------------------------------------------------------------

    int Types::getLiteralIntValue(LiteralNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        if (node->dataType != DATA_TYPE_INT)
        {
            debugger.logMessage("ERROR", __LINE__, "Types", "Invalid data type");
            return 0;
        }
        return node->value.intValue;
    }

    llvm::ConstantInt *Types::getLiteralIntValue(int value)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Types", "Getting integer literal value");
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(CryoContext::getInstance().context), value);
    }

    // getLiteralStringValue
    llvm::Constant *Types::getLiteralStringValue(std::string value)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Types", "Getting string literal value");
        return llvm::ConstantDataArray::getString(CryoContext::getInstance().context, value);
    }

    llvm::Value *Types::ptrToExplicitType(llvm::Value *value)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Types", "Getting pointer to explicit type");
        if (value->getType()->isPointerTy())
        {
            std::string valueName = value->getName().str();
            std::cout << "Value Name: " << valueName << std::endl;
            llvm::Instruction *getPtr = llvm::dyn_cast<llvm::Instruction>(value);
            llvm::Type *ptrType = getPtr->getOperand(0)->getType();
            llvm::Value *ptrValue = getPtr->getOperand(0);

            value->mutateType(ptrType);

            return value;
        }
        else
        {
            debugger.logMessage("ERROR", __LINE__, "Types", "Value is not a pointer");
            return nullptr;
        }
    }

    llvm::Value *Types::explicitTypeToPtr(llvm::Value *value)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Types", "Getting explicit type to pointer");
        if (!value->getType()->isPointerTy())
        {
            std::string valueName = value->getName().str();
            std::cout << "Value Name: " << valueName << std::endl;
            llvm::Instruction *getPtr = llvm::dyn_cast<llvm::Instruction>(value);
            llvm::Value *ptrValue = getPtr->getOperand(0);

            value->mutateType(ptrValue->getType()->getPointerTo());

            return value;
        }
        else
        {
            debugger.logMessage("ERROR", __LINE__, "Types", "Value is already a pointer");
            return nullptr;
        }
    }

    llvm::Value *Types::castTyToVal(llvm::Value *val, llvm::Type *ty)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Types", "Casting type to value");

        if (val->getType()->getTypeID() == ty->getTypeID())
        {
            debugger.logMessage("INFO", __LINE__, "Types", "Value is already the expected type");
            return val;
        }

        llvm::Instruction *inst = llvm::dyn_cast<llvm::Instruction>(val);
        if (inst)
        {
            llvm::Value *op = inst->getOperand(0);
            if (!op)
            {
                debugger.logMessage("ERROR", __LINE__, "Types", "Operand is null");
                CONDITION_FAILED;
                return nullptr;
            }
            if (op->getType()->getTypeID() == ty->getTypeID())
            {
                return op;
            }
        }

        debugger.logMessage("ERROR", __LINE__, "Types", "Failed to cast type to value");
        return nullptr;
    }

    llvm::Type *Types::getInstType(llvm::Value *val)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Types", "Getting instruction type");

        llvm::Instruction *inst = llvm::dyn_cast<llvm::Instruction>(val);
        if (inst)
        {
            debugger.logLLVMInst(inst);
            return parseInstForType(inst);
        }

        debugger.logMessage("ERROR", __LINE__, "Types", "Failed to get instruction type");
        return nullptr;
    }

    llvm::Type *Types::parseInstForType(llvm::Instruction *inst)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Types", "Parsing instruction for type");

        if (!inst)
        {
            debugger.logMessage("ERROR", __LINE__, "Types", "Instruction is null");
            return nullptr;
        }

        llvm::Value *op = inst->getOperand(0);
        if (!op)
        {
            debugger.logMessage("ERROR", __LINE__, "Types", "Operand is null");
            return nullptr;
        }

        llvm::Type *ty = op->getType();
        if (!ty)
        {
            debugger.logMessage("ERROR", __LINE__, "Types", "Type is null");
            return nullptr;
        }

        return ty;
    }

    // -----------------------------------------------------------------------------------------------

    std::string Types::trimStrQuotes(std::string str)
    {
        // Check to see if the string even has the quotes
        if (str[0] == '\"' && str[str.length() - 1] == '\"')
        {
            // Remove the first and last characters
            str = str.substr(1, str.length() - 2);
        }

        return str;
    }

} // namespace Cryo
