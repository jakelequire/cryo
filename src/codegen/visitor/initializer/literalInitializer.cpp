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
#include "codegen/codegen.hpp"

namespace Cryo
{
    llvm::Value *Initializer::generateLiteralExpr(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating literal expression...");

        if (!validateLiteralNode(node))
            return nullptr;

        DataType *literalDataType = node->data.literal->type;
        logMessage(LMI, "INFO", "Initializer", "Literal data type: %s", DTM->debug->dataTypeToString(literalDataType));

        // If the literal is an object type, handle it separately
        if (literalDataType->container->typeOf == OBJECT_TYPE)
        {
            logMessage(LMI, "INFO", "Initializer", "Creating object from literal");
            return generateLiteralObject(node);
        }

        node->print(node);

        // Handle primitive types
        return generatePrimitiveValue(node, literalDataType->container->primitive);
    }

    bool Initializer::validateLiteralNode(ASTNode *node)
    {
        if (!node)
        {
            logMessage(LMI, "ERROR", "Initializer", "Node is null");
            return false;
        }

        if (node->metaData->type != NODE_LITERAL_EXPR)
        {
            logMessage(LMI, "ERROR", "Initializer", "Node is not a literal expression");
            return false;
        }

        return true;
    }

    llvm::Value *Initializer::generatePrimitiveValue(ASTNode *node, PrimitiveDataType primitiveType)
    {
        logMessage(LMI, "INFO", "Initializer", "Primitive type: %s",
                   DTM->debug->primitiveDataTypeToString(primitiveType));

        // Get the literal value
        LiteralNode *lit = node->data.literal;

        switch (primitiveType)
        {
        // Integer types
        case PRIM_INT:
            return createIntegerLiteral(32, lit->value.intValue, true);
        case PRIM_I8:
            return createIntegerLiteral(8, lit->value.intValue, true);
        case PRIM_I16:
            return createIntegerLiteral(16, lit->value.intValue, true);
        case PRIM_I32:
            return createIntegerLiteral(32, lit->value.intValue, true);
        case PRIM_I64:
            return createIntegerLiteral(64, lit->value.intValue, true);
        case PRIM_I128:
            return createIntegerLiteral(128, lit->value.intValue, true);

        // Floating point
        case PRIM_FLOAT:
            return llvm::ConstantFP::get(context.getInstance().context,
                                         llvm::APFloat(lit->value.floatValue));

        // Boolean
        case PRIM_BOOLEAN:
            return llvm::ConstantInt::get(context.getInstance().context,
                                          llvm::APInt(1, lit->value.booleanValue, true));

        // Strings
        case PRIM_STRING:
        case PRIM_STR:
            return generateStringLiteral(node);

        // Special types
        case PRIM_NULL:
            return generateNullLiteral();
        case PRIM_VOID:
            return llvm::UndefValue::get(llvm::Type::getVoidTy(context.getInstance().context));
        case PRIM_ANY:
            return generateNullLiteral();

        // Object (should be handled separately, but included for completeness)
        case PRIM_OBJECT:
            logMessage(LMI, "WARNING", "Initializer", "PRIM_OBJECT should be handled by generateLiteralObject");
            return generateLiteralObject(node);

        // Unsupported types
        case PRIM_AUTO:
        case PRIM_UNDEFINED:
        case PRIM_UNKNOWN:
        default:
            logMessage(LMI, "ERROR", "Initializer", "Unhandled literal type: %s",
                       DTM->debug->primitiveDataTypeToString(primitiveType));
            return nullptr;
        }
    }

    llvm::Value *Initializer::generateLiteralObject(ASTNode *node)
    {
        if (!validateLiteralNode(node))
            return nullptr;

        DataType *literalDataType = node->data.literal->type;

        if (literalDataType->container->typeOf != OBJECT_TYPE)
        {
            logMessage(LMI, "ERROR", "Initializer", "Expected object type but got %s",
                       DTM->debug->typeofDataTypeToString(literalDataType->container->typeOf));
            return nullptr;
        }

        LiteralType literalType = node->data.literal->literalType;

        switch (literalType)
        {
        case LITERAL_INT:
            return createIntObjectFromLiteral(node, literalDataType);
        case LITERAL_STRING:
            logMessage(LMI, "INFO", "Initializer", "Creating object from string literal");
            // Implementation would go here
            break;
        case LITERAL_FLOAT:
            logMessage(LMI, "INFO", "Initializer", "Creating object from float literal");
            // Implementation would go here
            break;
        case LITERAL_BOOLEAN:
            logMessage(LMI, "INFO", "Initializer", "Creating object from boolean literal");
            // Implementation would go here
            break;
        default:
            logMessage(LMI, "ERROR", "Initializer", "Unhandled literal type: %s",
                       DTM->debug->literalTypeToString(literalType));
            return nullptr;
        }

        return nullptr; // Default return if not handled
    }

    llvm::Value *Initializer::createIntObjectFromLiteral(ASTNode *node, DataType *literalDataType)
    {
        logMessage(LMI, "INFO", "Initializer", "Creating object from integer literal");
        node->print(node);

        // Get the object type name and LLVM type
        std::string typeName = literalDataType->typeName;
        logMessage(LMI, "INFO", "Initializer", "Creating object instance of type: %s", typeName.c_str());

        llvm::Type *objectType = context.getInstance().symbolTable->getLLVMType(literalDataType);
        if (!objectType)
        {
            logMessage(LMI, "ERROR", "Initializer", "Failed to get LLVM type for %s", typeName.c_str());
            CONDITION_FAILED;
            return nullptr;
        }

        // Allocate memory for the struct
        llvm::Value *objectInstance = context.getInstance().builder.CreateAlloca(
            objectType, nullptr, typeName + ".instance");

        // Create constructor argument based on the literal value
        llvm::Value *ctorArg = llvm::ConstantInt::get(
            context.getInstance().context,
            llvm::APInt(32, node->data.literal->value.intValue, true));
        std::vector<llvm::Value *> ctorArgs;
        ctorArgs.push_back(ctorArg);

        // Call constructor if available
        if (!callConstructor(ctorArgs, typeName))
            logMessage(LMI, "WARNING", "Initializer", "Constructor for %s not found", typeName.c_str());

        // Load the object instance
        llvm::Value *objectValue = context.getInstance().builder.CreateLoad(
            objectType, objectInstance, "load." + typeName);

        logMessage(LMI, "INFO", "Initializer", "Loaded object instance: %s",
                   objectValue->getName().str().c_str());

        return objectValue;
    }

    bool Initializer::callConstructor(std::vector<llvm::Value *> ctorArgs, const std::string &typeName)
    {
        std::string ctorName = "struct." + typeName + ".ctor";

        // Find the constructor function
        IRFunctionSymbol *ctorFunc = context.getInstance().symbolTable->findFunction(ctorName);
        if (!ctorFunc)
            return false;

        // Prepare constructor arguments
        std::vector<llvm::Value *> args;
        args.insert(args.end(), ctorArgs.begin(), ctorArgs.end());

        // Call the constructor
        context.getInstance().builder.CreateCall(ctorFunc->function, args, "call_ctor." + typeName);
        logMessage(LMI, "INFO", "Initializer", "Called constructor %s", ctorName.c_str());

        return true;
    }
}