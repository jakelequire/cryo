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
            std::cout << "Has Index Expression: " << (node->data.varDecl->hasIndexExpr ? "Yes" : "No") << std::endl;
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
            std::cout << "----------------------------------------" << std::endl;

            break;

        case NODE_BINARY_EXPR:
            std::cout << "\nBinary Expression Node" << std::endl;
            std::cout << "Operator: " << CryoOperatorTypeToString(node->data.bin_op->op) << std::endl;
            std::cout << "Left Operand: " << std::endl;
            logNode(node->data.bin_op->left);
            std::cout << "Right Operand: " << std::endl;
            logNode(node->data.bin_op->right);
            std::cout << "----------------------------------------" << std::endl;

            break;

        case NODE_UNARY_EXPR:
            std::cout << "\nUnary Expression Node" << std::endl;
            std::cout << "Operator: " << CryoTokenToString(node->data.unary_op->op) << std::endl;
            logNode(node->data.unary_op->operand);
            std::cout << "----------------------------------------" << std::endl;

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
            std::cout << "----------------------------------------" << std::endl;

            break;

        case NODE_WHILE_STATEMENT:
            std::cout << "\nWhile Statement Node" << std::endl;
            std::cout << "Condition: " << std::endl;
            logNode(node->data.whileStatement->condition);
            std::cout << "Body: " << std::endl;
            logNode(node->data.whileStatement->body);
            std::cout << "----------------------------------------" << std::endl;

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
            std::cout << "----------------------------------------" << std::endl;

            break;

        case NODE_RETURN_STATEMENT:
            std::cout << "\nReturn Statement Node" << std::endl;
            std::cout << "Return Type : " << CryoDataTypeToString(node->data.returnStatement->returnType) << std::endl;
            std::cout << "Return Value: " << std::endl;
            if (node->data.returnStatement->returnValue)
            {
                logNode(node->data.returnStatement->returnValue);
            }
            else
            {
                std::cout << "Void" << std::endl;
            }
            std::cout << "----------------------------------------" << std::endl;

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
            std::cout << "----------------------------------------" << std::endl;

            break;

        case NODE_EXTERN_STATEMENT:
            std::cout << "\nExtern Statement Node" << std::endl;
            if (node->data.externNode->externNode->data.functionDecl)
            {
                std::cout << "Extern Function: " << std::endl;
                logNode(reinterpret_cast<ASTNode *>(node->data.externNode->externNode->data.functionDecl));
            }
            std::cout << "----------------------------------------" << std::endl;

            break;

        case NODE_ARRAY_LITERAL:
            std::cout << "\nArray Literal Node" << std::endl;
            std::cout << "Element Count: " << node->data.array->elementCount << std::endl;
            for (int i = 0; i < node->data.array->elementCount; ++i)
            {
                logNode(node->data.array->elements[i]);
            }
            std::cout << "----------------------------------------" << std::endl;

            break;

        case NODE_PARAM_LIST:
            std::cout << "\nParameter List Node" << std::endl;
            std::cout << "Parameter Count: " << node->data.paramList->paramCount << std::endl;
            for (int i = 0; i < node->data.paramList->paramCount; ++i)
            {
                logNode(node->data.paramList->params[i]->initializer);
            }
            std::cout << "----------------------------------------" << std::endl;

            break;

        case NODE_ARG_LIST:
            std::cout << "\nArgument List Node" << std::endl;
            std::cout << "Argument Count: " << node->data.argList->argCount << std::endl;
            for (int i = 0; i < node->data.argList->argCount; ++i)
            {
                logNode(node->data.argList->args[i]->initializer);
            }
            std::cout << "----------------------------------------" << std::endl;

            break;

        case NODE_FUNCTION_BLOCK:
            std::cout << "----------------------------------------" << std::endl;
            std::cout << "\nFunction Block Node" << std::endl;
            std::cout << "Statement Count: " << node->data.functionBlock->statementCount << std::endl;
            for (int i = 0; i < node->data.functionBlock->statementCount; ++i)
            {
                logNode(node->data.functionBlock->statements[i]);
            }
            std::cout << "----------------------------------------" << std::endl;

            break;

        case NODE_VAR_NAME:
            std::cout << "----------------------------------------" << std::endl;
            std::cout << "\nVariable Name Node" << std::endl;
            std::cout << "Variable Name: " << node->data.varName->varName << std::endl;
            std::cout << "Is Reference: " << (node->data.varName->isRef ? "True" : "False") << std::endl;
            std::cout << "----------------------------------------" << std::endl;

            break;

        case NODE_STRING_LITERAL:
            std::cout << "----------------------------------------" << std::endl;
            std::cout << "\nString Literal Node" << std::endl;
            std::cout << "Value: " << node->data.literal->value.stringValue << std::endl;
            std::cout << "----------------------------------------" << std::endl;

            break;

        case NODE_BOOLEAN_LITERAL:
            std::cout << "----------------------------------------" << std::endl;
            std::cout << "\nBoolean Literal Node" << std::endl;
            std::cout << "Value: " << (node->data.literal->value.booleanValue ? "True" : "False") << std::endl;
            std::cout << "----------------------------------------" << std::endl;

            break;

        case NODE_NAMESPACE:
            std::cout << "----------------------------------------" << std::endl;
            std::cout << "\nNamespace Node" << std::endl;
            std::cout << "Namespace Name: " << node->data.cryoNamespace->name << std::endl;
            std::cout << "----------------------------------------" << std::endl;

            break;

        case NODE_BLOCK:
            std::cout << "----------------------------------------" << std::endl;
            std::cout << "\nBlock Node" << std::endl;
            std::cout << "Statement Count: " << node->data.block->statementCount << std::endl;
            for (int i = 0; i < node->data.block->statementCount; ++i)
            {
                logNode(node->data.block->statements[i]);
            }
            std::cout << "----------------------------------------" << std::endl;

            break;

        case NODE_INDEX_EXPR:
            std::cout << "----------------------------------------" << std::endl;
            std::cout << "\nIndex Expression Node" << std::endl;
            std::cout << "Array Name: " << std::endl;
            logNode(node->data.indexExpr->array);
            std::cout << "Index: " << std::endl;
            logNode(node->data.indexExpr->index);
            std::cout << "----------------------------------------" << std::endl;

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
        case NODE_INDEX_EXPR:
        {
            // std::cout << "✅ Node is Valid!\n"
            //           << std::endl;
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

    // -----------------------------------------------------------------------------------------------

    bool CryoDebugger::lintTree(ASTNode *node)
    {
        static int nodeCount = 0;
        nodeCount++;

        if (!node)
        {
            logMessage("ERROR", __LINE__, "Debugger", "Node is null");
            return false;
        }

        std::stringstream ss;
        ss << "Processing node " << nodeCount << " of type: " << CryoNodeTypeToString(node->metaData->type)
           << " at address: " << node;
        logMessage("INFO", __LINE__, "Debugger", ss.str());

        // Perform a basic memory integrity check
        if (!isValidNodeType(node->metaData->type))
        {
            ss.str("");
            ss << "Invalid node type detected: " << node->metaData->type;
            logMessage("ERROR", __LINE__, "Debugger", ss.str());
            return false;
        }

        switch (node->metaData->type)
        {
        case NODE_PROGRAM:
            logMessage("INFO", __LINE__, "Debugger", "Processing PROGRAM node");
            assertNode(node);
            for (int i = 0; i < node->data.program->statementCount; ++i)
            {
                ss.str("");
                ss << "Processing statement " << i + 1 << " of " << node->data.program->statementCount;
                logMessage("INFO", __LINE__, "Debugger", ss.str());
                lintTree(node->data.program->statements[i]);
            }
            break;

        case NODE_VAR_DECLARATION:
            logMessage("INFO", __LINE__, "Debugger", "Processing VAR_DECLARATION node");
            assertNode(node);
            if (node->data.varDecl->initializer)
            {
                lintTree(node->data.varDecl->initializer);
            }
            break;

        case NODE_FUNCTION_DECLARATION:
            logMessage("WARNING", __LINE__, "Debugger", "Unexpected FUNCTION_DECLARATION node encountered");
            assertNode(node);
            break;

        case NODE_FUNCTION_CALL:
            assertNode(node);
            for (int i = 0; i < node->data.functionCall->argCount; ++i)
            {
                lintTree(node->data.functionCall->args[i]);
            }
            break;

        case NODE_LITERAL_EXPR:
            assertNode(node);
            break;

        case NODE_BINARY_EXPR:
            assertNode(node);
            lintTree(node->data.bin_op->left);
            lintTree(node->data.bin_op->right);
            break;

        case NODE_UNARY_EXPR:
            assertNode(node);
            lintTree(node->data.unary_op->operand);
            break;

        case NODE_IF_STATEMENT:
            assertNode(node);
            lintTree(node->data.ifStatement->condition);
            lintTree(node->data.ifStatement->thenBranch);
            if (node->data.ifStatement->elseBranch)
            {
                lintTree(node->data.ifStatement->elseBranch);
            }
            break;

        case NODE_WHILE_STATEMENT:
            assertNode(node);
            lintTree(node->data.whileStatement->condition);
            lintTree(node->data.whileStatement->body);
            break;

        case NODE_FOR_STATEMENT:
            assertNode(node);
            lintTree(node->data.forStatement->initializer);
            lintTree(node->data.forStatement->condition);
            lintTree(node->data.forStatement->increment);
            lintTree(node->data.forStatement->body);
            break;

        case NODE_RETURN_STATEMENT:
            assertNode(node);
            if (node->data.returnStatement->returnValue)
            {
                lintTree(node->data.returnStatement->returnValue);
            }
            break;

        case NODE_IMPORT_STATEMENT:
            assertNode(node);
            break;

        case NODE_EXTERN_FUNCTION:
            assertNode(node);
            for (int i = 0; i < node->data.externFunction->paramCount; ++i)
            {
                lintTree(node->data.externFunction->params[i]);
            }
            break;

        case NODE_EXTERN_STATEMENT:
            assertNode(node);
            lintTree(node->data.externNode->externNode);
            break;

        case NODE_ARRAY_LITERAL:
            assertNode(node);
            for (int i = 0; i < node->data.array->elementCount; ++i)
            {
                lintTree(node->data.array->elements[i]);
            }
            break;

        case NODE_PARAM_LIST:
            assertNode(node);
            for (int i = 0; i < node->data.paramList->paramCount; ++i)
            {
                lintTree(node->data.paramList->params[i]->initializer);
            }
            break;

        case NODE_ARG_LIST:
            assertNode(node);
            for (int i = 0; i < node->data.argList->argCount; ++i)
            {
                lintTree(node->data.argList->args[i]->initializer);
            }
            break;

        case NODE_FUNCTION_BLOCK:
            assertNode(node);
            for (int i = 0; i < node->data.functionBlock->statementCount; ++i)
            {
                lintTree(node->data.functionBlock->statements[i]);
            }
            break;

        case NODE_VAR_NAME:
            assertNode(node);
            break;

        case NODE_STRING_LITERAL:
            assertNode(node);
            break;

        case NODE_BOOLEAN_LITERAL:
            assertNode(node);
            break;

        case NODE_NAMESPACE:
            assertNode(node);
            break;

        case NODE_BLOCK:
            assertNode(node);
            for (int i = 0; i < node->data.block->statementCount; ++i)
            {
                lintTree(node->data.block->statements[i]);
            }
            break;

        case NODE_INDEX_EXPR:
            assertNode(node);
            lintTree(node->data.indexExpr->array);
            lintTree(node->data.indexExpr->index);
            break;

        case NODE_UNKNOWN:
            logMessage("ERROR", __LINE__, "Debugger", "Unknown node type");
            break;

        default:
            ss.str("");
            ss << "Unknown node type: " << node->metaData->type;
            logMessage("ERROR", __LINE__, "Debugger", ss.str());
        }

        return true;
    }

    bool CryoDebugger::isValidNodeType(CryoNodeType type)
    {
        // Add all valid node types to this check
        return (type >= NODE_PROGRAM && type < NODE_UNKNOWN);
    }

    // -----------------------------------------------------------------------------------------------

    bool CryoDebugger::assertNode(ASTNode *node)
    {
        if (!node)
        {
            logMessage("ERROR", __LINE__, "Debugger", "Node is null");
            return false;
        }

        switch (node->metaData->type)
        {
        case NODE_PROGRAM:
        {
            if (node->data.program->statementCount == 0)
            {
                logMessage("ERROR", __LINE__, "Debugger", "Program has no statements");
                return false;
            }
            logMessage("INFO", __LINE__, "Debugger", "Program passed assertion.");
            break;
        }

        case NODE_FUNCTION_DECLARATION:
        {
            logMessage("INFO", __LINE__, "Debugger", "Checking function declaration node");

            if (node->data.functionDecl == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "Function declaration data is null");
                return false;
            }

            if (node->data.functionDecl->name == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "Function has no name");
                return false;
            }
            else
            {
                logMessage("INFO", __LINE__, "Debugger", "Function name: " + std::string(node->data.functionDecl->name));
            }

            if (node->data.functionDecl->returnType == DATA_TYPE_UNKNOWN)
            {
                logMessage("ERROR", __LINE__, "Debugger", "Function has unknown return type");
                return false;
            }
            else
            {
                logMessage("INFO", __LINE__, "Debugger", "Function return type: " + std::string(CryoDataTypeToString(node->data.functionDecl->returnType)));
            }

            if (node->data.functionDecl->body == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "Function has no body");
                return false;
            }
            else
            {
                logMessage("INFO", __LINE__, "Debugger", "Function body exists");
            }

            logMessage("INFO", __LINE__, "Debugger", "Function declaration node passed basic checks");

            // Additional checks for parameters, if needed
            if (node->data.functionDecl->params != nullptr)
            {
                logMessage("INFO", __LINE__, "Debugger", "Checking function parameters");
                for (int i = 0; i < node->data.functionDecl->paramCount; ++i)
                {
                    if (node->data.functionDecl->params[i] == nullptr)
                    {
                        logMessage("ERROR", __LINE__, "Debugger", "Function has a null parameter at index " + std::to_string(i));
                        return false;
                    }
                }
                logMessage("INFO", __LINE__, "Debugger", "All function parameters are non-null");
            }
            else
            {
                logMessage("INFO", __LINE__, "Debugger", "Function has no parameters");
            }

            logMessage("INFO", __LINE__, "Debugger", "Function declaration passed all assertions");
            logNode(node);
            break;
        }

        case NODE_FUNCTION_CALL:
        {
            if (node->data.functionCall->name == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "Function call has no name");
                return false;
            }
            logMessage("INFO", __LINE__, "Debugger", "Function call passed assertion.");
            logNode(node);
            break;
        }

        case NODE_VAR_DECLARATION:
        {
            if (node->data.varDecl->name == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "Variable has no name");
                return false;
            }
            if (node->data.varDecl->type == DATA_TYPE_UNKNOWN)
            {
                logMessage("ERROR", __LINE__, "Debugger", "Variable has no type");
                return false;
            }
            logMessage("INFO", __LINE__, "Debugger", "Variable passed assertion:");
            logNode(node);
            break;
        }

        case NODE_LITERAL_EXPR:
        {
            if (node->data.literal->dataType == DATA_TYPE_UNKNOWN)
            {
                logMessage("ERROR", __LINE__, "Debugger", "Literal has no type");
                return false;
            }
            logMessage("INFO", __LINE__, "Debugger", "Literal passed assertion.");
            logNode(node);
            break;
        }

        case NODE_BINARY_EXPR:
        {
            if (node->data.bin_op->left == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "Binary expression has no left operand");
                return false;
            }
            if (node->data.bin_op->right == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "Binary expression has no right operand");
                return false;
            }
            logMessage("INFO", __LINE__, "Debugger", "Binary expression passed assertion.");
            logNode(node);
            break;
        }

        case NODE_UNARY_EXPR:
        {
            if (node->data.unary_op->operand == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "Unary expression has no operand");
                return false;
            }
            logMessage("INFO", __LINE__, "Debugger", "Unary expression passed assertion.");
            logNode(node);
            break;
        }

        case NODE_IF_STATEMENT:
        {
            if (node->data.ifStatement->condition == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "If statement has no condition");
                return false;
            }
            if (node->data.ifStatement->thenBranch == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "If statement has no then branch");
                return false;
            }
            logMessage("INFO", __LINE__, "Debugger", "If statement passed assertion.");
            logNode(node);
            break;
        }

        case NODE_WHILE_STATEMENT:
        {
            if (node->data.whileStatement->condition == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "While statement has no condition");
                return false;
            }
            if (node->data.whileStatement->body == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "While statement has no body");
                return false;
            }
            logMessage("INFO", __LINE__, "Debugger", "While statement passed assertion.");
            logNode(node);
            break;
        }

        case NODE_FOR_STATEMENT:
        {
            if (node->data.forStatement->initializer == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "For statement has no initializer");
                return false;
            }
            if (node->data.forStatement->condition == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "For statement has no condition");
                return false;
            }
            if (node->data.forStatement->increment == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "For statement has no increment");
                return false;
            }
            if (node->data.forStatement->body == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "For statement has no body");
                return false;
            }
            logMessage("INFO", __LINE__, "Debugger", "For statement passed assertion.");
            logNode(node);
            break;
        }

        case NODE_RETURN_STATEMENT:
        {
            if (node->data.returnStatement->returnValue == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "Return statement has no return value");
                return false;
            }
            logMessage("INFO", __LINE__, "Debugger", "Return statement passed assertion.");
            logNode(node);
            break;
        }

        case NODE_IMPORT_STATEMENT:
        {
            logMessage("INFO", __LINE__, "Debugger", "Import statement passed assertion.");
            logNode(node);
            break;
        }

        case NODE_EXTERN_FUNCTION:
        {
            if (node->data.externFunction->name == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "Extern function has no name");
                return false;
            }
            if (node->data.externFunction->returnType == DATA_TYPE_UNKNOWN)
            {
                logMessage("ERROR", __LINE__, "Debugger", "Extern function has no return type");
                return false;
            }
            logMessage("INFO", __LINE__, "Debugger", "Extern function passed assertion.");
            logNode(node);
            break;
        }

        case NODE_EXTERN_STATEMENT:
        {
            if (node->data.externNode->externNode == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "Extern statement has no extern node");
                return false;
            }
            logMessage("INFO", __LINE__, "Debugger", "Extern statement passed assertion.");
            logNode(node);
            break;
        }

        case NODE_ARRAY_LITERAL:
        {
            if (node->data.array->elementCount == 0)
            {
                logMessage("ERROR", __LINE__, "Debugger", "Array literal has no elements");
                return false;
            }
            for (int i = 0; i < node->data.array->elementCount; ++i)
            {
                if (node->data.array->elements[i] == nullptr)
                {
                    logMessage("ERROR", __LINE__, "Debugger", "Array literal has a null element");
                    return false;
                }
            }
            logMessage("INFO", __LINE__, "Debugger", "Array literal passed assertion.");
            logNode(node);
            break;
        }

        case NODE_PARAM_LIST:
        {
            for (int i = 0; i < node->data.paramList->paramCount; ++i)
            {
                if (node->data.paramList->params[i] == nullptr)
                {
                    logMessage("ERROR", __LINE__, "Debugger", "Parameter list has a null parameter");
                    return false;
                }
            }
            logMessage("INFO", __LINE__, "Debugger", "Parameter list passed assertion.");
            logNode(node);
            break;
        }

        case NODE_ARG_LIST:
        {
            for (int i = 0; i < node->data.argList->argCount; ++i)
            {
                if (node->data.argList->args[i] == nullptr)
                {
                    logMessage("ERROR", __LINE__, "Debugger", "Argument list has a null argument");
                    return false;
                }
            }
            logMessage("INFO", __LINE__, "Debugger", "Argument list passed assertion.");
            logNode(node);
            break;
        }

        case NODE_FUNCTION_BLOCK:
        {
            if (node->data.functionBlock->statementCount == 0)
            {
                logMessage("ERROR", __LINE__, "Debugger", "Function block has no statements");
                return false;
            }
            for (int i = 0; i < node->data.functionBlock->statementCount; ++i)
            {
                if (node->data.functionBlock->statements[i] == nullptr)
                {
                    logMessage("ERROR", __LINE__, "Debugger", "Function block has a null statement");
                    return false;
                }
            }
            logMessage("INFO", __LINE__, "Debugger", "Function block passed assertion.");
            logNode(node);
            break;
        }

        case NODE_VAR_NAME:
        {
            if (node->data.varName->varName == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "Variable name has no name");
                return false;
            }
            logMessage("INFO", __LINE__, "Debugger", "Variable name passed assertion.");
            logNode(node);
            break;
        }

        case NODE_STRING_LITERAL:
        {
            if (node->data.literal->value.stringValue == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "String literal has no value");
                return false;
            }
            logMessage("INFO", __LINE__, "Debugger", "String literal passed assertion.");
            logNode(node);
            break;
        }

        case NODE_BOOLEAN_LITERAL:
        {
            logMessage("INFO", __LINE__, "Debugger", "Boolean literal passed assertion.");
            logNode(node);
            break;
        }

        case NODE_NAMESPACE:
        {
            if (node->data.cryoNamespace->name == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "Namespace has no name");
                return false;
            }
            logMessage("INFO", __LINE__, "Debugger", "Namespace passed assertion.");
            logNode(node);
            break;
        }

        case NODE_UNKNOWN:
        {
            logMessage("ERROR", __LINE__, "Debugger", "Unknown node type");
            return false;
        }

        default:
            logMessage("ERROR", __LINE__, "Debugger", "Invalid node type");
            return false;
        }

        logMessage("INFO", __LINE__, "Debugger", "Node passed assertion.");

        return true;
    }

} // namespace Cryo
