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
#include "codegen/oldCodeGen.hpp"

namespace Cryo
{
    llvm::Type *OldTypes::getType(DataType *type, int length)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Types", "Getting type");
        if (type->container->baseType == STRUCT_TYPE)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Struct type");
            return this->getStructType(type);
        }
        if (length == 0)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Converting simple type to LLVM type");
            return this->convertSimpleType(type);
        }
        else
        {
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Converting complex type to LLVM type");
            return this->convertComplexType(type, length);
        }
    }

    llvm::Type *OldTypes::getLiteralType(LiteralNode *literal)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Types", "Getting literal type");

        if (literal->type->container->baseType == UNKNOWN_TYPE)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Unknown type");
            CONDITION_FAILED;
        }

        DataType *type = literal->type;
        int len = literal->length;
        switch (type->container->primitive)
        {
        case PRIM_INT:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Returning int type");
            return llvm::Type::getInt32Ty(CryoContext::getInstance().context);
        }
        case PRIM_STRING:
        {
            // [i8 x len]
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Returning string type");
            return llvm::ArrayType::get(llvm::Type::getInt8Ty(CryoContext::getInstance().context), len);
        }
        case PRIM_FLOAT:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Returning float type");
            return llvm::Type::getFloatTy(CryoContext::getInstance().context);
        }
        case PRIM_BOOLEAN:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Returning boolean type");
            return llvm::Type::getInt1Ty(CryoContext::getInstance().context);
        }
        default:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Unknown type");
            return nullptr;
        }
        }
    }

    llvm::Type *OldTypes::getReturnType(DataType *type)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Types", "Getting return type");

        if (type->container->baseType == UNKNOWN_TYPE)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Unknown type");
            return nullptr;
        }

        switch (type->container->primitive)
        {
        case PRIM_INT:
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Returning int type");
            return llvm::Type::getInt32Ty(CryoContext::getInstance().context);
        case PRIM_STRING:
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Returning string type");
            return llvm::Type::getInt8Ty(CryoContext::getInstance().context)->getPointerTo();
        case PRIM_FLOAT:
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Returning float type");
            return llvm::Type::getFloatTy(CryoContext::getInstance().context);
        case PRIM_BOOLEAN:
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Returning boolean type");
            return llvm::Type::getInt1Ty(CryoContext::getInstance().context);
        case PRIM_VOID:
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Returning void type");
            return llvm::Type::getVoidTy(CryoContext::getInstance().context);
        case PRIM_ANY:
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Returning any type");
            return llvm::Type::getInt8Ty(CryoContext::getInstance().context)->getPointerTo();
            // case DATA_TYPE_INT_ARRAY:
            //     DevDebugger::logMessage("INFO", __LINE__, "Types", "Returning int array type");
            //     return llvm::ArrayType::get(llvm::Type::getInt32Ty(CryoContext::getInstance().context), 0);
            // case DATA_TYPE_STRING_ARRAY:
            //     DevDebugger::logMessage("INFO", __LINE__, "Types", "Returning string array type");
            //     return llvm::ArrayType::get(llvm::Type::getInt8Ty(CryoContext::getInstance().context), 0);
            // case DATA_TYPE_ARRAY:
            //    DevDebugger::logMessage("INFO", __LINE__, "Types", "Returning array type");
            //    return llvm::ArrayType::get(llvm::Type::getInt32Ty(CryoContext::getInstance().context), 0);
        default:
        {
            std::string typeStr = DataTypeToString(type);
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Unknown type, received: " + typeStr);
            return nullptr;
        }
        }
    }

    // -----------------------------------------------------------------------------------------------

    llvm::Type *OldTypes::convertSimpleType(DataType *type)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Types", "Converting simple type to LLVM type");

        switch (type->container->primitive)
        {
        case PRIM_INT: // 32-bit Default Integer
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Returning 32-bit integer type (default)");
            return llvm::Type::getInt32Ty(CryoContext::getInstance().context);
        case PRIM_I8:
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Returning 8-bit integer type");
            return llvm::Type::getInt8Ty(CryoContext::getInstance().context);
        case PRIM_I16:
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Returning 16-bit integer type");
            return llvm::Type::getInt16Ty(CryoContext::getInstance().context);
        case PRIM_I32:
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Returning 32-bit integer type");
            return llvm::Type::getInt32Ty(CryoContext::getInstance().context);
        case PRIM_I64:
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Returning 64-bit integer type");
            return llvm::Type::getInt64Ty(CryoContext::getInstance().context);
        case PRIM_I128:
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Returning 128-bit integer type");
            return llvm::Type::getInt128Ty(CryoContext::getInstance().context);

        case PRIM_STRING:
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Converting string to LLVM type");
            return llvm::Type::getInt8Ty(CryoContext::getInstance().context)->getPointerTo();

        case PRIM_FLOAT:
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Converting float to LLVM type");
            return llvm::Type::getFloatTy(CryoContext::getInstance().context);

        case PRIM_BOOLEAN:
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Converting boolean to LLVM type");
            return llvm::Type::getInt1Ty(CryoContext::getInstance().context);

        case PRIM_VOID:
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Converting void to LLVM type");
            return llvm::Type::getVoidTy(CryoContext::getInstance().context);

            // case DATA_TYPE_INT_ARRAY:
            //     DevDebugger::logMessage("INFO", __LINE__, "Types", "Converting int array to LLVM type");
            //     return llvm::ArrayType::get(llvm::Type::getInt32Ty(CryoContext::getInstance().context), 0);

            // case DATA_TYPE_ANY:
            //     DevDebugger::logMessage("INFO", __LINE__, "Types", "Converting any to LLVM type");
            //     return llvm::Type::getInt8Ty(CryoContext::getInstance().context)->getPointerTo();

        case PRIM_CUSTOM:
        {
            if (type->container->baseType == FUNCTION_TYPE)
            {
                DevDebugger::logMessage("INFO", __LINE__, "Types", "Converting function to LLVM type");
                DataType *returntype = type->container->custom.funcDef->returnType;

                return getFunctionReturnType(returntype);
            }
            if (type->container->baseType == STRUCT_TYPE)
            {
                DevDebugger::logMessage("INFO", __LINE__, "Types", "Converting struct to LLVM type");
                return getStructType(type);
            }
        }

        default:
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Unknown type");
            std::cout << "Received: " << DataTypeToString(type) << std::endl;
            CONDITION_FAILED;
        }
    }

    llvm::Type *OldTypes::getIntegerTypeFromPrimitive(PrimitiveDataType type)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Types", "Getting integer type from primitive");

        switch (type)
        {
        case PRIM_INT: // 32-bit Default Integer
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Returning 32-bit integer type (default)");
            return llvm::Type::getInt32Ty(CryoContext::getInstance().context);
        case PRIM_I8:
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Returning 8-bit integer type");
            return llvm::Type::getInt8Ty(CryoContext::getInstance().context);
        case PRIM_I16:
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Returning 16-bit integer type");
            return llvm::Type::getInt16Ty(CryoContext::getInstance().context);
        case PRIM_I32:
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Returning 32-bit integer type");
            return llvm::Type::getInt32Ty(CryoContext::getInstance().context);
        case PRIM_I64:
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Returning 64-bit integer type");
            return llvm::Type::getInt64Ty(CryoContext::getInstance().context);
        case PRIM_I128:
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Returning 128-bit integer type");
            return llvm::Type::getInt128Ty(CryoContext::getInstance().context);
        }
        return nullptr;
    }

    // -----------------------------------------------------------------------------------------------

    llvm::Type *OldTypes::convertComplexType(DataType *types, int length)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Types", "Converting complex type to LLVM type");
        switch (types->container->primitive)
        {
        case PRIM_STRING:
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Converting string to LLVM type");
            return llvm::ArrayType::get(llvm::Type::getInt8Ty(CryoContext::getInstance().context), length);

            // case DATA_TYPE_ARRAY:
            //     DevDebugger::logMessage("INFO", __LINE__, "Types", "Converting array to LLVM type");
            //     return llvm::ArrayType::get(llvm::Type::getInt32Ty(CryoContext::getInstance().context), length);
            //
            // case DATA_TYPE_INT_ARRAY:
            //     DevDebugger::logMessage("INFO", __LINE__, "Types", "Converting int array to LLVM type");
            //     return llvm::ArrayType::get(llvm::Type::getInt32Ty(CryoContext::getInstance().context), length);
            //
            // case DATA_TYPE_STRING_ARRAY:
            //    DevDebugger::logMessage("INFO", __LINE__, "Types", "Converting string array to LLVM type");
            //    return llvm::ArrayType::get(llvm::Type::getInt8Ty(CryoContext::getInstance().context), length);

        default:
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Unknown type");
            return nullptr;
        }
    }

    llvm::Type *OldTypes::getFunctionReturnType(DataType *returnType)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Types", "Getting function return type");
        logDataType(returnType);
        std::cout << "Primitive: " << PrimitiveDataTypeToString(returnType->container->primitive) << std::endl;
        std::cout << "Typeof DataType: " << TypeofDataTypeToString(returnType->container->baseType) << std::endl;

        TypeofDataType baseType = returnType->container->baseType;
        if (baseType == STRUCT_TYPE)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Returning struct type");
            return getStructType(returnType);
        }

        if (baseType == PRIMITIVE_TYPE)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Returning primitive type");
            return convertSimpleType(returnType);
        }

        DEBUG_BREAKPOINT; // Safeguard
        return nullptr;
    }

    // -----------------------------------------------------------------------------------------------

    int OldTypes::getLiteralValLength(ASTNode *node)
    {
        Arrays &arrays = compiler.getArrays();
        if (node->metaData->type == NODE_STRING_LITERAL)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Getting length of string literal");
            return strlen(strdup(node->data.literal->value.stringValue));
        }

        if (node->metaData->type == NODE_LITERAL_EXPR)
        {
            if (node->data.literal->type->container->primitive == PRIM_STRING)
            {
                DevDebugger::logMessage("INFO", __LINE__, "Types", "Getting length of string literal");
                int _len = strlen(strdup(node->data.literal->value.stringValue));
                std::cout << "<TYPES> String Length: " << _len << std::endl;
                return _len;
            }

            if (node->data.literal->type->container->primitive == PRIM_INT)
            {
                return 0;
            }

            if (node->data.literal->type->container->primitive == PRIM_FLOAT)
            {
                return 0;
            }

            if (node->data.literal->type->container->primitive == PRIM_BOOLEAN)
            {
                return 0;
            }

            // if (node->data.literal->dataType == DATA_TYPE_INT_ARRAY)
            // {
            //     DevDebugger::logMessage("INFO", __LINE__, "Types", "Getting length of int array literal");
            //     return node->data.array->elementCount;
            // }
        }
        if (node->metaData->type == NODE_ARRAY_LITERAL)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Getting length of array literal");
            return arrays.getArrayLength(node);
        }

        if (node->metaData->type == NODE_VAR_DECLARATION)
        {
            if (node->data.varDecl->type->container->baseType == PRIM_STRING)
            {
                DevDebugger::logMessage("INFO", __LINE__, "Types", "Getting length of string variable");
                return strlen(strdup(node->data.varDecl->initializer->data.literal->value.stringValue));
            }
            else
            {
                return 0;
            }
        }

        if (node->metaData->type == NODE_INDEX_EXPR)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Getting length of index expression");
            return 0;
        }

        char *typeNode = CryoNodeTypeToString(node->metaData->type);

        DevDebugger::logMessage("ERROR", __LINE__, "Types", "Unknown node type");
        DevDebugger::logMessage("ERROR", __LINE__, "Types", "Type: " + std::string(typeNode));
        return 0;
    }

    int OldTypes::getLiteralValLength(LiteralNode *node)
    {
        if (node->type->container->primitive == PRIM_STRING)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Getting length of string literal");
            return strlen(strdup(node->value.stringValue));
        }
        return 0;
    }

    // -----------------------------------------------------------------------------------------------

    int OldTypes::getLiteralIntValue(LiteralNode *node)
    {
        if (node->type->container->baseType != PRIM_INT)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Types", "Invalid data type");
            return 0;
        }
        return node->value.intValue;
    }

    llvm::ConstantInt *OldTypes::getLiteralIntValue(int value)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Types", "Getting integer literal value");
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(CryoContext::getInstance().context), value);
    }

    // getLiteralStringValue
    llvm::Constant *OldTypes::getLiteralStringValue(std::string value)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Types", "Getting string literal value");
        return llvm::ConstantDataArray::getString(CryoContext::getInstance().context, value);
    }

    llvm::Value *OldTypes::ptrToExplicitType(llvm::Value *value)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Types", "Getting pointer to explicit type");
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
            DevDebugger::logMessage("ERROR", __LINE__, "Types", "Value is not a pointer");
            return nullptr;
        }
    }

    llvm::Value *OldTypes::explicitTypeToPtr(llvm::Value *value)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Types", "Getting explicit type to pointer");
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
            DevDebugger::logMessage("ERROR", __LINE__, "Types", "Value is already a pointer");
            return nullptr;
        }
    }

    llvm::Value *OldTypes::castTyToVal(llvm::Value *val, llvm::Type *ty)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Types", "Casting type to value");

        if (val->getType()->getTypeID() == ty->getTypeID())
        {
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Value is already the expected type");
            return val;
        }

        llvm::Instruction *inst = llvm::dyn_cast<llvm::Instruction>(val);
        if (inst)
        {
            llvm::Value *op = inst->getOperand(0);
            if (!op)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "Types", "Operand is null");
                CONDITION_FAILED;
                return nullptr;
            }
            if (op->getType()->getTypeID() == ty->getTypeID())
            {
                return op;
            }
        }

        DevDebugger::logMessage("ERROR", __LINE__, "Types", "Failed to cast type to value");
        return nullptr;
    }

    llvm::Type *OldTypes::getInstType(llvm::Value *val)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Types", "Getting instruction type");

        llvm::Instruction *inst = llvm::dyn_cast<llvm::Instruction>(val);
        if (inst)
        {
            DevDebugger::logLLVMInst(inst);
            return parseInstForType(inst);
        }

        DevDebugger::logMessage("ERROR", __LINE__, "Types", "Failed to get instruction type");
        return nullptr;
    }

    llvm::Type *OldTypes::parseInstForType(llvm::Instruction *inst)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Types", "Parsing instruction for type");

        if (!inst)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Types", "Instruction is null");
            return nullptr;
        }

        DevDebugger::logMessage("INFO", __LINE__, "Types", "Parsing instruction");

        llvm::Value *op = inst->getOperand(0);
        if (!op)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Types", "Operand is null");
            return nullptr;
        }

        DevDebugger::logMessage("INFO", __LINE__, "Types", "Operand");

        llvm::Type *ty = op->getType();
        if (!ty)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Types", "Type is null");
            return nullptr;
        }

        return ty;
    }

    // -----------------------------------------------------------------------------------------------

    std::string OldTypes::trimStrQuotes(std::string str)
    {
        // Check to see if the string even has the quotes
        if (str[0] == '\"' && str[str.length() - 1] == '\"')
        {
            // Remove the first and last characters
            str = str.substr(1, str.length() - 2);
        }

        return str;
    }

    // -----------------------------------------------------------------------------------------------

    llvm::Type *OldTypes::getStructType(DataType *type)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Types", "Getting struct type");

        if (type->container->baseType != STRUCT_TYPE)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Types", "Invalid data type");
            return nullptr;
        }

        // Check if the struct already exists
        std::string structName = type->container->custom.structDef->name;
        llvm::Type *structType = doesStructExist(type->container->custom.structDef->name);
        if (structType)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Struct already exists: " + structName);
            return structType;
        }

        std::vector<llvm::Type *> structTypes;
        for (int i = 0; i < type->container->custom.structDef->propertyCount; i++)
        {
            ASTNode *property = type->container->custom.structDef->properties[i];
            DataType *propertyType = property->data.property->type;
            llvm::Type *llvmType = this->getType(propertyType, 0);
            structTypes.push_back(llvmType);
        }
        DevDebugger::logMessage("INFO", __LINE__, "Types", "Creating struct type");

        DevDebugger::logMessage("INFO", __LINE__, "Types", "Struct Name: " + structName);

        return llvm::StructType::create(CryoContext::getInstance().context, structTypes);
    }

    llvm::Type *OldTypes::doesStructExist(std::string structName)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Types", "Checking if struct exists");

        if (CryoContext::getInstance().structTypes.find(structName) != CryoContext::getInstance().structTypes.end())
        {
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Struct exists");
            return CryoContext::getInstance().structTypes[structName];
        }

        DevDebugger::logMessage("INFO", __LINE__, "Types", "Struct does not exist");
        return nullptr;
    }

    llvm::Value *OldTypes::createAllocaFromStructProps(llvm::StructType *structType, llvm::Value *thisPtr)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Types", "Creating alloca from struct properties");

        std::string structName = structType->getName().str();
        std::string propName = structName + ".prop";

        llvm::IRBuilder<> builder(CryoContext::getInstance().context);
        llvm::Value *alloca = builder.CreateAlloca(structType, nullptr, propName);

        for (int i = 0; i < structType->getNumElements(); i++)
        {
            llvm::Value *fieldPtr = builder.CreateStructGEP(structType, thisPtr, i, "field" + std::to_string(i));
            builder.CreateStore(fieldPtr, alloca);
        }

        return alloca;
    }

    bool OldTypes::isCustomType(DataType *type)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Types", "Checking if type is custom");

        if (type->container->baseType == STRUCT_TYPE)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Types", "Type is custom");
            return true;
        }

        DevDebugger::logMessage("INFO", __LINE__, "Types", "Type is not custom");
        return false;
    }

    bool OldTypes::isLLVMStringType(llvm::Value *val)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Types", "Checking if value is LLVM string type");

        if (val->getType()->isPointerTy())
        {
            llvm::Instruction *inst = llvm::dyn_cast<llvm::Instruction>(val);
            if (!inst)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "Types", "Instruction is null");
                return false;
            }
            DevDebugger::logLLVMInst(inst);
            llvm::Type *ty = parseInstForType(inst);
            if (ty->isPointerTy())
            {
                llvm::Type *ptrTy = ty;
                if (ptrTy->isIntegerTy(8))
                {
                    DevDebugger::logMessage("INFO", __LINE__, "Types", "Value is an LLVM string type");
                    return true;
                }
            }

            DevDebugger::logMessage("INFO", __LINE__, "Types", "Value is not an LLVM string type");
            DevDebugger::logLLVMType(ty);
            return false;
        }

        DevDebugger::logMessage("INFO", __LINE__, "Types", "Value is not a pointer");
        DevDebugger::logLLVMValue(val);
        return false;
    }
} // namespace Cryo
