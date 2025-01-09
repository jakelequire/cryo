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
#include "codegen/devDebugger/devDebugger.hpp"
#include "tools/logger/logger_config.h"

namespace Cryo
{
    void DevDebugger::logMessage(const char *type, int line, const std::string &category, const std::string &message)
    {
        DEBUG_PRINT_FILTER({
            std::cout << "[" << type << std::setw(6) << "] "
                      << "\t@" << std::setw(4) << line
                      << "\t{ " << std::setw(12) << std::left << category << "}"
                      << "\t" << message
                      << std::endl;
        });
    }

    /**
     * @public
     * @brief Used to log a specific AST node.
     */
    void DevDebugger::logNode(ASTNode *node)
    {
        DEBUG_PRINT_FILTER({
            VALIDATE_ASTNODE(node);

            if (!node)
            {
                std::cerr << "Node is null." << std::endl;
                return;
            }

            // Migrated to new AST Debugging Logging in the C-side of the compiler.
            // (See: src/frontend/AST/debugASTOutput.c)
            logASTNode(node);

            return;
        });
    }

    /**
     * @brief Macro Implementation to help reduce code duplication & to simplify type checking.
     */
    void DevDebugger::checkNode(ASTNode *node)
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
            CONDITION_FAILED;
        }
    }
    /**
     * @private
     * @brief A private method to validate the type of the node and to prove it exists
     */
    bool DevDebugger::isNodeTypeValid(ASTNode *node)
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
        case NODE_PARAM:
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
        case NODE_VAR_REASSIGN:
        case NODE_PROPERTY:
        case NODE_STRUCT_DECLARATION:
        case NODE_CUSTOM_TYPE:
        case NODE_SCOPED_FUNCTION_CALL:
        case NODE_STRUCT_CONSTRUCTOR:
        case NODE_THIS:
        case NODE_THIS_ASSIGNMENT:
        case NODE_PROPERTY_ACCESS:
        case NODE_PROPERTY_REASSIGN:
        case NODE_METHOD:
        case NODE_METHOD_CALL:
        case NODE_GENERIC_DECL:
        case NODE_GENERIC_INST:
        case NODE_CLASS:
        case NODE_CLASS_CONSTRUCTOR:
        case NODE_OBJECT_INST:
        {
            // std::cout << "✅ Node is Valid!\n"
            //           << std::endl;
            return true;
        }

        case NODE_UNKNOWN:
        {
            std::cerr << RED BOLD "============================================================" COLOR_RESET << std::endl;
            std::cerr << "[❌] Node is Unknown!\n"
                      << std::endl;
            std::cerr << "Received Node: " << std::endl;
            logASTNode(node);
            std::cerr << RED BOLD "============================================================" COLOR_RESET << std::endl;

            return false;
        }
        default:
            std::cerr << RED BOLD "============================================================" COLOR_RESET << std::endl;

            std::cerr << "[❌] Node is Invalid!\n"
                      << std::endl;
            std::cerr << "Received Node: " << std::endl;
            logASTNode(node);
            std::cerr << RED BOLD "============================================================" COLOR_RESET << std::endl;

            return false;
        }
    }

    // -----------------------------------------------------------------------------------------------

    bool DevDebugger::lintTree(ASTNode *node)
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
                lintTree(node->data.argList->args[i]);
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

        case NODE_VAR_REASSIGN:
            assertNode(node);
            break;

        case NODE_PARAM:
            assertNode(node);
            if (node->data.param->hasDefaultValue)
            {
                lintTree(node->data.param->defaultValue);
            }
            break;

        case NODE_SCOPED_FUNCTION_CALL:
            assertNode(node);
            break;

        case NODE_STRUCT_CONSTRUCTOR:
            assertNode(node);
            break;

        case NODE_PROPERTY_ACCESS:
            assertNode(node);
            break;

        case NODE_THIS:
            assertNode(node);
            break;

        case NODE_PROPERTY_REASSIGN:
            assertNode(node);
            break;

        case NODE_METHOD:
            assertNode(node);
            break;

        case NODE_PROPERTY:
            assertNode(node);
            break;

        case NODE_METHOD_CALL:
            assertNode(node);
            break;

        case NODE_GENERIC_DECL:
            assertNode(node);
            break;

        case NODE_GENERIC_INST:
            assertNode(node);
            break;

        case NODE_CLASS:
            assertNode(node);
            break;

        case NODE_CLASS_CONSTRUCTOR:
            assertNode(node);
            break;

        case NODE_OBJECT_INST:
            assertNode(node);
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

    bool DevDebugger::isValidNodeType(CryoNodeType type)
    {
        // Add all valid node types to this check
        return (type >= NODE_PROGRAM && type < NODE_UNKNOWN);
    }

    // -----------------------------------------------------------------------------------------------

    bool DevDebugger::assertNode(ASTNode *node)
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

            if (node->data.functionDecl->type->container->baseType == UNKNOWN_TYPE)
            {
                logMessage("ERROR", __LINE__, "Debugger", "Function has unknown return type");
                return false;
            }
            else
            {
                logMessage("INFO", __LINE__, "Debugger", "Function return type: " + std::string(DataTypeToString(node->data.functionDecl->type)));
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
            if (node->data.varDecl->type->container->baseType == UNKNOWN_TYPE)
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
            if (node->data.literal->type->container->baseType == UNKNOWN_TYPE)
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
            if (node->data.externFunction->type->container->baseType == UNKNOWN_TYPE)
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

        case NODE_VAR_REASSIGN:
        {
            if (node->data.varReassignment->existingVarName == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "Variable reassign has no name");
                return false;
            }
            if (node->data.varReassignment->existingVarNode == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "Variable reassign has no value");
                return false;
            }
            if (node->data.varReassignment->existingVarType->container->baseType == UNKNOWN_TYPE)
            {
                logMessage("ERROR", __LINE__, "Debugger", "Variable reassign has no type");
                return false;
            }
            if (node->data.varReassignment->newVarNode == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "Variable reassign has no new value");
                return false;
            }
            logMessage("INFO", __LINE__, "Debugger", "Variable reassign passed assertion.");
            logNode(node);
            break;
        }

        case NODE_PARAM:
        {
            if (node->data.param->name == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "Parameter has no name");
                return false;
            }
            if (node->data.param->functionName == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "Parameter has no function name");
                return false;
            }
            if (node->data.param->type->container->baseType == UNKNOWN_TYPE)
            {
                logMessage("ERROR", __LINE__, "Debugger", "Parameter has no type");
                return false;
            }
            if (node->data.param->hasDefaultValue && node->data.param->defaultValue == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "Parameter has no default value");
                return false;
            }
            logMessage("INFO", __LINE__, "Debugger", "Parameter passed assertion.");
            logNode(node);
            break;
        }

        case NODE_SCOPED_FUNCTION_CALL:
        {
            logMessage("INFO", __LINE__, "Debugger", "Scoped function call passed assertion.");
            logNode(node);
            break;
        }

        case NODE_STRUCT_CONSTRUCTOR:
        {
            logMessage("INFO", __LINE__, "Debugger", "Struct constructor passed assertion.");
            logNode(node);
            break;
        }

        case NODE_PROPERTY_ACCESS:
        {
            logMessage("INFO", __LINE__, "Debugger", "Property access passed assertion.");
            logNode(node);
            break;
        }

        case NODE_THIS:
        {
            logMessage("INFO", __LINE__, "Debugger", "This passed assertion.");
            logNode(node);
            break;
        }

        case NODE_PROPERTY_REASSIGN:
        {
            logMessage("INFO", __LINE__, "Debugger", "Property reassign passed assertion.");
            logNode(node);
            break;
        }

        case NODE_METHOD:
        {
            logMessage("INFO", __LINE__, "Debugger", "Method passed assertion.");
            logNode(node);
            break;
        }

        case NODE_PROPERTY:
        {
            logMessage("INFO", __LINE__, "Debugger", "Property passed assertion.");
            logNode(node);
            break;
        }

        case NODE_METHOD_CALL:
        {
            logMessage("INFO", __LINE__, "Debugger", "Method call passed assertion.");
            logNode(node);
            break;
        }

        case NODE_GENERIC_DECL:
        {
            logMessage("INFO", __LINE__, "Debugger", "Generic type passed assertion.");
            logNode(node);
            break;
        }

        case NODE_GENERIC_INST:
        {
            logMessage("INFO", __LINE__, "Debugger", "Generic instance passed assertion.");
            logNode(node);
            break;
        }

        case NODE_CLASS:
        {
            logMessage("INFO", __LINE__, "Debugger", "Class passed assertion.");
            logNode(node);
            break;
        }

        case NODE_CLASS_CONSTRUCTOR:
        {
            logMessage("INFO", __LINE__, "Debugger", "Class constructor passed assertion.");
            logNode(node);
            break;
        }

        case NODE_OBJECT_INST:
        {
            logMessage("INFO", __LINE__, "Debugger", "Object instance passed assertion.");
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

    // -----------------------------------------------------------------------------------------------

    void DevDebugger::logLLVMValue(llvm::Value *valueNode)
    {
        DEBUG_PRINT_FILTER({
            if (valueNode == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "LLVM Value is null");
                return;
            }

            std::cout << "\n";
            std::cout << ">>===-----------<LLVM Value Node>-----------===<<" << std::endl;
            std::string valueStr;
            llvm::raw_string_ostream rso(valueStr);

            // This prints out the line of IR that was generated
            valueNode->print(rso);
            std::cout << "Inst:" << "" << rso.str() << std::endl;

            // Print out the type of the value
            std::string tyName = LLVMTypeIDToString(valueNode->getType());
            std::cout << "Type: " << tyName << std::endl;

            // Print out the name of the value
            std::string valName = valueNode->getName().str();
            std::cout << "Name: " << valName << std::endl;

            // Print out the address of the value
            std::cout << "Address: " << valueNode << std::endl;

            std::cout << ">>===--------------------------------------===<<" << std::endl;
            std::cout << "\n";
        });
    }

    void DevDebugger::logLLVMStruct(llvm::StructType *structTy)
    {
        DEBUG_PRINT_FILTER({
            logMessage("INFO", __LINE__, "Debugger", "Logging LLVM Struct Type");

            if (structTy == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "LLVM Struct Type is null");
                return;
            }

            std::cout << "\n";
            std::cout << ">>===-----------<LLVM Struct Type Node>-----------===<<" << std::endl;

            // Print out the name of the struct
            std::string structName = structTy->getName().str();
            std::cout << "Name: " << structName << std::endl;

            // Print out the instruction of the struct
            std::string structInst;
            llvm::raw_string_ostream rso(structInst);
            structTy->print(rso);
            std::cout << "Inst: " << rso.str() << std::endl;

            // Print out the address of the struct
            std::cout << "Address: " << structTy << std::endl;
            std::cout << ">>===-------------------------------------------===<<" << std::endl;
        });
    }

    void DevDebugger::logLLVMType(llvm::Type *type)
    {
        DEBUG_PRINT_FILTER({
            if (type == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "LLVM Type is null");
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
        });
    }

    void DevDebugger::logLLVMInst(llvm::Instruction *inst)
    {
        DEBUG_PRINT_FILTER({
            if (inst == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "LLVM Instruction is null");
                return;
            }

            std::cout << "\n";
            std::cout << ">>===-----------<LLVM Instruction Node>-----------===<<" << std::endl;

            // Print out the name of the instruction
            std::string instName = inst->getName().str();
            std::cout << "Name: " << instName << std::endl;

            // Print out the instruction of the instruction
            std::string instInst;
            llvm::raw_string_ostream rso(instInst);
            inst->print(rso);
            std::cout << "Inst: " << rso.str() << std::endl;

            // Print out the address of the instruction
            std::cout << "Address: " << inst << std::endl;
            std::cout << ">>===-------------------------------------------===<<" << std::endl;
        });
    }

    void DevDebugger::logLLVMFunction(llvm::Function *func)
    {
        DEBUG_PRINT_FILTER({
            if (func == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "LLVM Function is null");
                return;
            }

            std::cout << "\n";
            std::cout << ">>===-----------<LLVM Function Node>-----------===<<" << std::endl;

            // Print out the name of the function
            std::string funcName = func->getName().str();
            std::cout << "Name: " << funcName << std::endl;

            // Print out the instruction of the function
            std::string funcInst;
            llvm::raw_string_ostream rso(funcInst);
            func->print(rso);
            std::cout << "Inst: " << rso.str() << std::endl;

            // Print out the address of the function
            std::cout << "Address: " << func << std::endl;
            std::cout << ">>===-------------------------------------------===<<" << std::endl;
        });
    }

    void DevDebugger::logLLVMBlock(llvm::BasicBlock *block)
    {
        DEBUG_PRINT_FILTER({
            if (block == nullptr)
            {
                logMessage("ERROR", __LINE__, "Debugger", "LLVM Basic Block is null");
                return;
            }

            std::cout << "\n";
            std::cout << ">>===-----------<LLVM Basic Block Node>-----------===<<" << std::endl;

            // Print out the name of the block
            std::string blockName = block->getName().str();
            std::cout << "Name: " << blockName << std::endl;

            // Check for a parent function (if any)
            llvm::Function *parentFunc = block->getParent();
            if (parentFunc != nullptr)
            {
                std::string parentFuncName = parentFunc->getName().str();
                std::cout << "Parent Function: " << parentFuncName << std::endl;
            }
            else
            {
                std::cout << "Parent Function: " << "<None>" << std::endl;
            }

            // Print out the instruction of the block
            std::string blockInst;
            llvm::raw_string_ostream rso(blockInst);
            block->print(rso);
            std::cout << "Inst: " << rso.str() << std::endl;

            // Print out the address of the block
            std::cout << "Address: " << block << std::endl;
            std::cout << ">>===---------------------------------------------===<<" << std::endl;
        });
    }

    // -----------------------------------------------------------------------------------------------

    std::string DevDebugger::LLVMTypeIDToString(llvm::Type *type)
    {
        if (!type || type == nullptr)
        {
            logMessage("ERROR", __LINE__, "Debugger", "LLVM Type is null");
            return "Unknown";
        }

        llvm::Type::TypeID tyID = type->getTypeID();
        std::string typeStr = "Unknown";
        switch (tyID)
        {
        case llvm::Type::TypeID::HalfTyID:
        {
            typeStr = "HalfTyID";
            break;
        }
        case llvm::Type::TypeID::BFloatTyID:
        {
            typeStr = "BFloatTyID";
            break;
        }
        case llvm::Type::TypeID::FloatTyID:
        {
            typeStr = "FloatTyID";
            break;
        }
        case llvm::Type::TypeID::DoubleTyID:
        {
            typeStr = "DoubleTyID";
            break;
        }
        case llvm::Type::TypeID::X86_FP80TyID:
        {
            typeStr = "X86_FP80TyID";
            break;
        }
        case llvm::Type::TypeID::FP128TyID:
        {
            typeStr = "FP128TyID";
            break;
        }
        case llvm::Type::TypeID::PPC_FP128TyID:
        {
            typeStr = "PPC_FP128TyID";
            break;
        }
        case llvm::Type::TypeID::VoidTyID:
        {
            typeStr = "VoidTyID";
            break;
        }
        case llvm::Type::TypeID::LabelTyID:
        {
            typeStr = "LabelTyID";
            break;
        }
        case llvm::Type::TypeID::MetadataTyID:
        {
            typeStr = "MetadataTyID";
            break;
        }
        case llvm::Type::TypeID::X86_MMXTyID:
        {
            typeStr = "X86_MMXTyID";
            break;
        }
        case llvm::Type::TypeID::X86_AMXTyID:
        {
            typeStr = "X86_AMXTyID";
            break;
        }
        case llvm::Type::TypeID::TokenTyID:
        {
            typeStr = "TokenTyID";
            break;
        }
        case llvm::Type::TypeID::IntegerTyID:
        {
            typeStr = "IntegerTyID";
            break;
        }
        case llvm::Type::TypeID::FunctionTyID:
        {
            typeStr = "FunctionTyID";
            break;
        }
        case llvm::Type::TypeID::PointerTyID:
        {
            typeStr = "PointerTyID";
            break;
        }
        case llvm::Type::TypeID::StructTyID:
        {
            typeStr = "StructTyID";
            break;
        }
        case llvm::Type::TypeID::ArrayTyID:
        {
            typeStr = "ArrayTyID";
            break;
        }
        case llvm::Type::TypeID::FixedVectorTyID:
        {
            typeStr = "FixedVectorTyID";
            break;
        }
        case llvm::Type::TypeID::ScalableVectorTyID:
        {
            typeStr = "ScalableVectorTyID";
            break;
        }
        case llvm::Type::TypeID::TypedPointerTyID:
        {
            typeStr = "TypedPointerTyID";
            break;
        }
        case llvm::Type::TypeID::TargetExtTyID:
        {
            typeStr = "TargetExtTyID";
            break;
        }
        default:
        {
            typeStr = "TyID Defaulted";
            break;
        }
        }

        return typeStr;
    }

    void DevDebugger::UNIMPLEMENTED_FUNCTION(std::string functionName, std::string fileName, int lineNumber)
    {
        const char *cStr_funcName = functionName.c_str();
        const char *cStr_fileName = fileName.c_str();
        __UNIMPLEMENTED_FUNCTION_EX__(cStr_funcName, cStr_fileName, lineNumber);
    }

} // namespace Cryo