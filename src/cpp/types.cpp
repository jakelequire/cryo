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

        char *typeNode = CryoNodeTypeToString(node->metaData->type);

        debugger.logMessage("ERROR", __LINE__, "Types", "Unknown node type");
        debugger.logMessage("ERROR", __LINE__, "Types", "Type: " + std::string(typeNode));
        return 0;
    }

    CryoBoolean *Types::evalBooleanExpression(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Types", "Evaluating boolean expression");

        CryoBoolean *result = new CryoBoolean();
        CryoNodeType nodeType = node->metaData->type;
        CryoDataType nodeDataType;

        switch (nodeType)
        {
        case NODE_LITERAL_EXPR:
        {
            nodeDataType = node->data.literal->dataType;
            assert(nodeDataType != DATA_TYPE_UNKNOWN);
            switch (nodeDataType)
            {
            case DATA_TYPE_BOOLEAN:
            {
                bool boolValue = node->data.literal->value.booleanValue ? true : false;
                debugger.logMessage("INFO", __LINE__, "Types", "Boolean value: " + std::to_string(boolValue));
                result->result = boolValue;
                if (boolValue)
                {
                    result->truthy = TRUTHY_TRUE;
                }
                else
                {
                    result->falsey = FALSEY_FALSE;
                }
                return result;
            }
            }
        }
        }

        debugger.logMessage("ERROR", __LINE__, "Types", "Unknown node type");
        return nullptr;
    }
} // namespace Cryo
