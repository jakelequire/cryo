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

        std::cout << "Node Type: " << CryoNodeTypeToString(node->type) << std::endl;
        std::cout << "Line: " << node->line << std::endl;

        switch (node->type)
        {
        case NODE_PROGRAM:
            std::cout << "Program Node" << std::endl;
            std::cout << "Statements Count: " << node->data.program.stmtCount << std::endl;
            for (int i = 0; i < node->data.program.stmtCount; ++i)
            {
                logNode(node->data.program.statements[i]);
            }
            break;

        case NODE_FUNCTION_DECLARATION:
            std::cout << "Function Declaration Node" << std::endl;
            std::cout << "Function Name: " << node->data.functionDecl.function->name << std::endl;
            std::cout << "Return Type: " << CryoDataTypeToString(node->data.functionDecl.function->returnType) << std::endl;
            std::cout << "Parameter Count: " << node->data.functionDecl.function->paramCount << std::endl;
            for (int i = 0; i < node->data.functionDecl.function->paramCount; ++i)
            {
                logNode(node->data.functionDecl.function->params[i]);
            }
            logNode(node->data.functionDecl.function->body);
            break;

        case NODE_FUNCTION_CALL:
            std::cout << "Function Call Node" << std::endl;
            std::cout << "Function Name: " << node->data.functionCall.name << std::endl;
            std::cout << "Argument Count: " << node->data.functionCall.argCount << std::endl;
            for (int i = 0; i < node->data.functionCall.argCount; ++i)
            {
                logNode(node->data.functionCall.args[i]);
            }
            break;

        case NODE_VAR_DECLARATION:
            std::cout << "Variable Declaration Node" << std::endl;
            std::cout << "Variable Name: " << node->data.varDecl.name << std::endl;
            std::cout << "Data Type: " << CryoDataTypeToString(node->data.varDecl.dataType) << std::endl;
            std::cout << "Is Global: " << (node->data.varDecl.isGlobal ? "Yes" : "No") << std::endl;
            std::cout << "Is Reference: " << (node->data.varDecl.isReference ? "Yes" : "No") << std::endl;
            std::cout << "Scope Level: " << node->data.varDecl.scopeLevel << std::endl;
            logNode(node->data.varDecl.initializer);
            break;

        case NODE_LITERAL_EXPR:
            std::cout << "Literal Expression Node" << std::endl;
            std::cout << "Data Type: " << CryoDataTypeToString(node->data.literalExpression.dataType) << std::endl;
            switch (node->data.literalExpression.dataType)
            {
            case DATA_TYPE_INT:
                std::cout << "Value: " << node->data.literalExpression.intValue << std::endl;
                break;
            case DATA_TYPE_FLOAT:
                std::cout << "Value: " << node->data.literalExpression.floatValue << std::endl;
                break;
            case DATA_TYPE_STRING:
                std::cout << "Value: " << node->data.literalExpression.stringValue << std::endl;
                break;
            case DATA_TYPE_BOOLEAN:
                std::cout << "Value: " << (node->data.literalExpression.booleanValue ? "True" : "False") << std::endl;
                break;
            }
            break;

        case NODE_BINARY_EXPR:
            std::cout << "Binary Expression Node" << std::endl;
            std::cout << "Operator: " << node->data.bin_op.operatorText << std::endl;
            std::cout << "Left Operand: " << std::endl;
            logNode(node->data.bin_op.left);
            std::cout << "Right Operand: " << std::endl;
            logNode(node->data.bin_op.right);
            break;

        case NODE_UNARY_EXPR:
            std::cout << "Unary Expression Node" << std::endl;
            std::cout << "Operator: " << CryoTokenToString(node->data.unary_op.op) << std::endl;
            logNode(node->data.unary_op.operand);
            break;

        case NODE_IF_STATEMENT:
            std::cout << "If Statement Node" << std::endl;
            std::cout << "Condition: " << std::endl;
            logNode(node->data.ifStmt.condition);
            std::cout << "Then Branch: " << std::endl;
            logNode(node->data.ifStmt.thenBranch);
            if (node->data.ifStmt.elseBranch)
            {
                std::cout << "Else Branch: " << std::endl;
                logNode(node->data.ifStmt.elseBranch);
            }
            break;

        case NODE_WHILE_STATEMENT:
            std::cout << "While Statement Node" << std::endl;
            std::cout << "Condition: " << std::endl;
            logNode(node->data.whileStmt.condition);
            std::cout << "Body: " << std::endl;
            logNode(node->data.whileStmt.body);
            break;

        case NODE_FOR_STATEMENT:
            std::cout << "For Statement Node" << std::endl;
            std::cout << "Initializer: " << std::endl;
            logNode(node->data.forStmt.initializer);
            std::cout << "Condition: " << std::endl;
            logNode(node->data.forStmt.condition);
            std::cout << "Increment: " << std::endl;
            logNode(node->data.forStmt.increment);
            std::cout << "Body: " << std::endl;
            logNode(node->data.forStmt.body);
            break;

        case NODE_RETURN_STATEMENT:
            std::cout << "Return Statement Node" << std::endl;
            std::cout << "Return Value: " << std::endl;
            logNode(node->data.returnStmt.returnValue);
            break;

        case NODE_IMPORT_STATEMENT:
            std::cout << "Import Statement Node" << std::endl;
            std::cout << "Module Path: " << node->data.importStatementNode.modulePath << std::endl;
            break;

        case NODE_EXTERN_STATEMENT:
            std::cout << "Extern Statement Node" << std::endl;
            if (node->data.externNode.decl.function)
            {
                std::cout << "Extern Function: " << std::endl;
                logNode(reinterpret_cast<ASTNode *>(node->data.externNode.decl.function));
            }
            break;

        case NODE_ARRAY_LITERAL:
            std::cout << "Array Literal Node" << std::endl;
            std::cout << "Element Count: " << node->data.arrayLiteral.array->elementCount << std::endl;
            for (int i = 0; i < node->data.arrayLiteral.array->elementCount; ++i)
            {
                logNode(node->data.arrayLiteral.array->elements[i]);
            }
            break;

        case NODE_PARAM_LIST:
            std::cout << "Parameter List Node" << std::endl;
            std::cout << "Parameter Count: " << node->data.paramList.paramCount << std::endl;
            for (int i = 0; i < node->data.paramList.paramCount; ++i)
            {
                logNode(node->data.paramList.params[i]);
            }
            break;

        case NODE_ARG_LIST:
            std::cout << "Argument List Node" << std::endl;
            std::cout << "Argument Count: " << node->data.argList.argCount << std::endl;
            for (int i = 0; i < node->data.argList.argCount; ++i)
            {
                logNode(node->data.argList.args[i]);
            }
            break;

        case NODE_FUNCTION_BLOCK:
            std::cout << "Function Block Node" << std::endl;
            logNode(node->data.functionBlock.function);
            logNode(node->data.functionBlock.block);
            break;

        case NODE_VAR_NAME:
            std::cout << "Variable Name Node" << std::endl;
            std::cout << "Variable Name: " << node->data.varName.varName << std::endl;
            std::cout << "Is Reference: " << (node->data.varName.isReference ? "Yes" : "No") << std::endl;
            break;

        case NODE_STRING_LITERAL:
            std::cout << "String Literal Node" << std::endl;
            std::cout << "Value: " << node->data.literalExpression.stringValue << std::endl;
            break;

        case NODE_BOOLEAN_LITERAL:
            std::cout << "Boolean Literal Node" << std::endl;
            std::cout << "Value: " << (node->data.boolean.value ? "True" : "False") << std::endl;
            break;

        default:
            std::cout << "Unknown Node Type" << std::endl;
            break;
        }

        if (node->firstChild)
        {
            std::cout << "First Child: " << std::endl;
            logNode(node->firstChild);
        }

        if (node->nextSibling)
        {
            std::cout << "Next Sibling: " << std::endl;
            logNode(node->nextSibling);
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
            exit(1);
        }
        if (!isNodeTypeValid(node))
        {
            std::cerr << "<Error>: Unknown or Invalid Node Type." << std::endl;
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
        switch (node->type)
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
        {
            return true;
        }

        case NODE_UNKNOWN:
        {
            std::cerr << "[Debugger] Node is Unknown!\n"
                      << std::endl;
            return false;
        }
        default:
            return false;
        }
    }

} // namespace Cryo
