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

/*

LLVM Type System Reference
---

Structs IR Representation:
```
%T1 = type { <type list> }     ; Identified normal struct type
%T2 = type <{ <type list> }>   ; Identified packed struct type
```
Examples of struct types:
{ i32, i32, i32 }   - A triple of three i32 values
{ float, ptr }      - A pair, where the first element is a float and the second element is a pointer.
<{ i8, i32 }>       - A packed struct known to be 5 bytes in size.

~~~~~~~~~~


Integer Types:
<4 x i32> Vector of 4 32-bit integer values.
<8 x float> Vector of 8 32-bit floating-point values.
<2 x i64> Vector of 2 64-bit integer values.
<4 x ptr> Vector of 4 pointers
<vscale x 4 x i32> Vector with a multiple of 4 32-bit integer values.


~~~~~~~~~~


Floating-Point Types:

Type        - Description
--------------------------------
half        - 16-bit floating-point value
bfloat      - 16-bit “brain” floating-point value (7-bit significand). Provides the same number of exponent bits as float,
            so that it matches its dynamic range, but with greatly reduced precision.
            Used in Intel’s AVX-512 BF16 extensions and Arm’s ARMv8.6-A extensions, among others.
float       - 32-bit floating-point value
double      - 64-bit floating-point value
fp128       - 128-bit floating-point value (113-bit significand)
x86_fp80    - 80-bit floating-point value (X87)
ppc_fp128   - 128-bit floating-point value (two 64-bits)


~~~~~~~~~~


Bitcasting:
%val = bitcast <4 x i4> <i4 1, i4 2, i4 3, i4 5> to i16

; Bitcasting from a vector to an integral type can be seen as
; concatenating the values:
;   %val now has the hexadecimal value 0x5321.

store i16 %val, ptr %ptr

; In memory the content will be (8-bit addressing):
;
;    [%ptr + 0]: 00100001  (0x21)
;    [%ptr + 1]: 01010011  (0x53)

*/
