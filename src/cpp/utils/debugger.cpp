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
#include "cpp/debugger.h"

namespace Cryo
{

    void CryoDebugger::logError(const std::string &message, const std::string &detail)
    {
        std::cerr << "Error: " << message << (detail.empty() ? "" : ": " + detail) << std::endl;
    }

    void CryoDebugger::logError(const std::string &message)
    {
        std::cerr << "Error: " << message << std::endl;
    }

    void CryoDebugger::logSuccess(const std::string &message, const std::string &detail)
    {
        std::cout << "[CPP] " << message << (detail.empty() ? "" : ": " + detail) << std::endl;
    }

    void CryoDebugger::logMessage(const char *type, int line, const std::string &category, const std::string &message)
    {
        std::cout << "[" << type << std::setw(6) << "] "
                  << "\t@" << std::setw(4) << line
                  << "\t{ " << std::setw(12) << std::left << category << "}"
                  << "\t" << message
                  << std::endl;
    }

    /**
     * @public
     * @brief Used to log a specific node.
     */
    void CryoDebugger::logNode(ASTNode *node)
    {
        VALIDATE_ASTNODE(node);

        if (!node)
        {
            std::cerr << "Node is null." << std::endl;
            return;
        }

        std::cout << "Node Type: " << CryoNodeTypeToString(node->metaData->type) << std::endl;

        switch (node->metaData->type)
        {
        case NODE_PROGRAM:
            std::cout << "\nProgram Node" << std::endl;
            std::cout << "Statements Count: " << node->data.program->statementCount << std::endl;
            for (int i = 0; i < node->data.program->statementCount; ++i)
            {
                logNode(node->data.program->statements[i]);
            }
            break;

        case NODE_FUNCTION_DECLARATION:
            std::cout << "\nFunction Declaration Node" << std::endl;
            std::cout << "Function Name: " << node->data.functionDecl->name << std::endl;
            std::cout << "Return Type: " << CryoDataTypeToString(node->data.functionDecl->returnType) << std::endl;
            std::cout << "Parameter Count: " << node->data.functionDecl->paramCount << std::endl;
            for (int i = 0; i < node->data.functionDecl->paramCount; ++i)
            {
                logNode(node->data.functionDecl->params[i]);
            }
            logNode(node->data.functionDecl->body);
            break;

        case NODE_FUNCTION_CALL:
            std::cout << "\nFunction Call Node" << std::endl;
            std::cout << "Function Name: " << node->data.functionCall->name << std::endl;
            std::cout << "Argument Count: " << node->data.functionCall->argCount << std::endl;
            for (int i = 0; i < node->data.functionCall->argCount; ++i)
            {
                logNode(node->data.functionCall->args[i]);
            }
            break;

        case NODE_VAR_DECLARATION:
            if (node == nullptr)
            {
                std::cerr << "Error: varDecl is null." << std::endl;
                break;
            }
            std::cout << "\nVariable Declaration Node" << std::endl;
            std::cout << "Variable Name: " << node->data.varDecl->name << std::endl;
            std::cout << "Data Type: " << CryoDataTypeToString(node->data.varDecl->type) << std::endl;
            std::cout << "Is Global: " << (node->data.varDecl->isGlobal ? 0 : 1) << std::endl;
            std::cout << "Is Reference: " << (node->data.varDecl->isReference ? "Yes" : "No") << std::endl;
            if (node->data.varDecl->initializer)
            {
                std::cout << "Initializer: " << std::endl;
                logNode(node->data.varDecl->initializer);
            }
            break;

        case NODE_LITERAL_EXPR:
            std::cout << "\nLiteral Expression Node" << std::endl;
            std::cout << "Data Type: " << CryoDataTypeToString(node->data.literal->dataType) << std::endl;
            switch (node->data.literal->dataType)
            {
            case DATA_TYPE_INT:
                std::cout << "Value: " << node->data.literal->value.intValue << std::endl;
                break;
            case DATA_TYPE_FLOAT:
                std::cout << "Value: " << node->data.literal->value.floatValue << std::endl;
                break;
            case DATA_TYPE_STRING:
                std::cout << "Value: " << node->data.literal->value.stringValue << std::endl;
                break;
            case DATA_TYPE_BOOLEAN:
                std::cout << "Value: " << (node->data.literal->value.booleanValue ? "True" : "False") << std::endl;
                break;
            }
            break;

        case NODE_BINARY_EXPR:
            std::cout << "\nBinary Expression Node" << std::endl;
            std::cout << "Operator: " << CryoOperatorTypeToString(node->data.bin_op->op) << std::endl;
            std::cout << "Left Operand: " << std::endl;
            logNode(node->data.bin_op->left);
            std::cout << "Right Operand: " << std::endl;
            logNode(node->data.bin_op->right);
            break;

        case NODE_UNARY_EXPR:
            std::cout << "\nUnary Expression Node" << std::endl;
            std::cout << "Operator: " << CryoTokenToString(node->data.unary_op->op) << std::endl;
            logNode(node->data.unary_op->operand);
            break;

        case NODE_IF_STATEMENT:
            std::cout << "\nIf Statement Node" << std::endl;
            std::cout << "Condition: " << std::endl;
            logNode(node->data.ifStatement->condition);
            std::cout << "\nThen Branch: " << std::endl;
            logNode(node->data.ifStatement->thenBranch);
            if (node->data.ifStatement->elseBranch)
            {
                std::cout << "\nElse Branch: " << std::endl;
                logNode(node->data.ifStatement->elseBranch);
            }
            break;

        case NODE_WHILE_STATEMENT:
            std::cout << "\nWhile Statement Node" << std::endl;
            std::cout << "Condition: " << std::endl;
            logNode(node->data.whileStatement->condition);
            std::cout << "Body: " << std::endl;
            logNode(node->data.whileStatement->body);
            break;

        case NODE_FOR_STATEMENT:
            std::cout << "\nFor Statement Node" << std::endl;
            std::cout << "Initializer: " << std::endl;
            logNode(node->data.forStatement->initializer);
            std::cout << "Condition: " << std::endl;
            logNode(node->data.forStatement->condition);
            std::cout << "Increment: " << std::endl;
            logNode(node->data.forStatement->increment);
            std::cout << "Body: " << std::endl;
            logNode(node->data.forStatement->body);
            break;

        case NODE_RETURN_STATEMENT:
            std::cout << "\nReturn Statement Node" << std::endl;
            std::cout << "Return Value: " << std::endl;
            if (node->data.returnStatement->returnValue)
            {
                logNode(node->data.returnStatement->returnValue);
            }
            else
            {
                std::cout << "Void" << std::endl;
            }
            break;

        case NODE_IMPORT_STATEMENT:
            std::cout << "\nImport Statement Node" << std::endl;
            break;

        case NODE_EXTERN_FUNCTION:
            std::cout << "\nExtern Function Node" << std::endl;
            std::cout << "Function Name: " << strdup(node->data.externFunction->name) << std::endl;
            std::cout << "Return Type: " << CryoDataTypeToString(node->data.externFunction->returnType) << std::endl;
            std::cout << "Parameter Count: " << node->data.externFunction->paramCount << std::endl;
            for (int i = 0; i < node->data.externFunction->paramCount; ++i)
            {
                logNode(node->data.externFunction->params[i]);
            }
            break;

        case NODE_EXTERN_STATEMENT:
            std::cout << "\nExtern Statement Node" << std::endl;
            if (node->data.externNode->externNode->data.functionDecl)
            {
                std::cout << "Extern Function: " << std::endl;
                logNode(reinterpret_cast<ASTNode *>(node->data.externNode->externNode->data.functionDecl));
            }
            break;

        case NODE_ARRAY_LITERAL:
            std::cout << "\nArray Literal Node" << std::endl;
            std::cout << "Element Count: " << node->data.array->elementCount << std::endl;
            for (int i = 0; i < node->data.array->elementCount; ++i)
            {
                logNode(node->data.array->elements[i]);
            }
            break;

        case NODE_PARAM_LIST:
            std::cout << "\nParameter List Node" << std::endl;
            std::cout << "Parameter Count: " << node->data.paramList->paramCount << std::endl;
            for (int i = 0; i < node->data.paramList->paramCount; ++i)
            {
                logNode(node->data.paramList->params[i]->initializer);
            }
            break;

        case NODE_ARG_LIST:
            std::cout << "\nArgument List Node" << std::endl;
            std::cout << "Argument Count: " << node->data.argList->argCount << std::endl;
            for (int i = 0; i < node->data.argList->argCount; ++i)
            {
                logNode(node->data.argList->args[i]->initializer);
            }
            break;

        case NODE_FUNCTION_BLOCK:
            std::cout << "\nFunction Block Node" << std::endl;
            std::cout << "Statement Count: " << node->data.functionBlock->statementCount << std::endl;
            for (int i = 0; i < node->data.functionBlock->statementCount; ++i)
            {
                logNode(node->data.functionBlock->statements[i]);
            }
            break;

        case NODE_VAR_NAME:
            std::cout << "\nVariable Name Node" << std::endl;
            std::cout << "Variable Name: " << node->data.varName->varName << std::endl;
            std::cout << "Is Reference: " << (node->data.varName->isRef ? 0 : 1) << std::endl;
            break;

        case NODE_STRING_LITERAL:
            std::cout << "\nString Literal Node" << std::endl;
            std::cout << "Value: " << node->data.literal->value.stringValue << std::endl;
            break;

        case NODE_BOOLEAN_LITERAL:
            std::cout << "\nBoolean Literal Node" << std::endl;
            std::cout << "Value: " << (node->data.literal->value.booleanValue ? "True" : "False") << std::endl;
            break;

        case NODE_NAMESPACE:
            std::cout << "\nNamespace Node" << std::endl;
            std::cout << "Namespace Name: " << node->metaData->moduleName << std::endl;
            break;

        default:
            std::cout << "\nUnknown Node Type" << std::endl;
            std::cout << "Node Type: " << CryoNodeTypeToString(node->metaData->type) << std::endl;
            break;
        }
    }

    /**
     * @brief Macro Implementation to help reduce code duplication & to simplify type checking.
     */
    void CryoDebugger::checkNode(ASTNode *node)
    {
        if (!node || node == nullptr)
        {
            std::cerr << "<Error>: Undefined AST Node." << std::endl;
            std::cout << "Node: " << node << std::endl;
        }
        if (!isNodeTypeValid(node))
        {
            std::cerr << "<Error>: Unknown or Invalid Node Type." << std::endl;
            std::cout << "Node: " << node << std::endl;
            std::cout << "Node: " << CryoNodeTypeToString(node->metaData->type) << std::endl;
            exit(1);
        }
    }

    /**
     * @public
     * @brief A hard-stop to the program. Captures the final state and as much debug information possible.
     */
    void CryoDebugger::debugPanic(std::string funcName)
    {
        CryoContext &context = this->context;

        std::cout << "<Error> PANIC @" << funcName << "\n"
                  << ">==--------------- Debug Information ---------------==>"
                  << std::endl;
        exit(1);
    }

    /**
     * @private
     * @brief A private method to validate the type of the node and to prove it exists
     */
    bool CryoDebugger::isNodeTypeValid(ASTNode *node)
    {
        if (!node)
        {
            std::cout << "[Debugger] Node is Null!\n"
                      << std::endl;
            return false;
        }
        switch (node->metaData->type)
        {
        case NODE_PROGRAM:
        case NODE_FUNCTION_DECLARATION:
        case NODE_VAR_DECLARATION:
        case NODE_STATEMENT:
        case NODE_EXPRESSION:
        case NODE_BINARY_EXPR:
        case NODE_UNARY_EXPR:
        case NODE_LITERAL_EXPR:
        case NODE_VAR_NAME:
        case NODE_FUNCTION_CALL:
        case NODE_IF_STATEMENT:
        case NODE_WHILE_STATEMENT:
        case NODE_FOR_STATEMENT:
        case NODE_RETURN_STATEMENT:
        case NODE_BLOCK:
        case NODE_FUNCTION_BLOCK:
        case NODE_EXPRESSION_STATEMENT:
        case NODE_ASSIGN:
        case NODE_PARAM_LIST:
        case NODE_TYPE:
        case NODE_STRING_LITERAL:
        case NODE_STRING_EXPRESSION:
        case NODE_BOOLEAN_LITERAL:
        case NODE_ARRAY_LITERAL:
        case NODE_IMPORT_STATEMENT:
        case NODE_EXTERN_STATEMENT:
        case NODE_EXTERN_FUNCTION:
        case NODE_ARG_LIST:
        case NODE_NAMESPACE:
        {
            std::cout << "✅ Node is Valid!\n"
                      << std::endl;
            return true;
        }

        case NODE_UNKNOWN:
        {
            std::cerr << "[❌] Node is Unknown!\n"
                      << std::endl;
            return false;
        }
        default:
            std::cerr << "[❌] Node is Invalid!\n"
                      << std::endl;
            return false;
        }
    }

} // namespace Cryo
