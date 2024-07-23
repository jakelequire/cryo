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
#include "cpp/cppLogger.h"


void printIndentation(int level) {
    for (int i = 0; i < level; ++i) {
        std::cout << "  ";
    }
}

void logASTNode(ASTNode* node, int indentLevel = 0) {
    if (!node) {
        printIndentation(indentLevel);
        std::cout << "\"Node\": null" << std::endl;
        return;
    };

    printIndentation(indentLevel);
    std::cout << "\"Node\": {" << std::endl;
    indentLevel++;

    printIndentation(indentLevel);
    std::cout << "\"Type\": \"" << CryoNodeTypeToString(node->type) << "\"," << std::endl;

    switch (node->type) {
        case CryoNodeType::NODE_PROGRAM:
            printIndentation(indentLevel);
            std::cout << "\"Program\": {" << std::endl;
            indentLevel++;
            printIndentation(indentLevel);
            std::cout << "\"Statements\": " << node->data.program.stmtCount << "," << std::endl;
            printIndentation(indentLevel);
            std::cout << "\"Capacity\": " << node->data.program.stmtCapacity << "," << std::endl;
            printIndentation(indentLevel);
            std::cout << "\"StatementNodes\": [" << std::endl;
            for (int i = 0; i < node->data.program.stmtCount; ++i) {
                logASTNode(node->data.program.statements[i], indentLevel + 1);
                if (i < node->data.program.stmtCount - 1) {
                    printIndentation(indentLevel + 1);
                    std::cout << "," << std::endl;
                }
            }
            std::cout << std::endl;
            printIndentation(indentLevel);
            std::cout << "]" << std::endl;
            indentLevel--;
            printIndentation(indentLevel);
            std::cout << "}" << std::endl;
            break;

        case CryoNodeType::NODE_STATEMENT:
            printIndentation(indentLevel);
            std::cout << "\"Statement\": {" << std::endl;
            logASTNode(node->data.stmt.stmt, indentLevel + 1);
            printIndentation(indentLevel);
            std::cout << "}" << std::endl;
            break;

        case CryoNodeType::NODE_EXPRESSION:
            printIndentation(indentLevel);
            std::cout << "\"Expression\": {" << std::endl;
            logASTNode(node->data.expr.expression, indentLevel + 1);
            printIndentation(indentLevel);
            std::cout << "}" << std::endl;
            break;

        case CryoNodeType::NODE_LITERAL_EXPR:
            printIndentation(indentLevel);
            std::cout << "\"Literal\": {" << std::endl;
            switch (node->data.literalExpression.dataType) {
                case DATA_TYPE_INT:
                    printIndentation(indentLevel + 1);
                    std::cout << "\"Integer\": " << node->data.literalExpression.intValue << std::endl;
                    break;
                case DATA_TYPE_STRING:
                    printIndentation(indentLevel + 1);
                    std::cout << "\"String\": \"" << node->data.literalExpression.stringValue << "\"" << std::endl;
                    break;
                case DATA_TYPE_BOOLEAN:
                    printIndentation(indentLevel + 1);
                    std::cout << "\"Boolean\": " << (node->data.literalExpression.booleanValue ? "true" : "false") << std::endl;
                    break;
                default:
                    printIndentation(indentLevel + 1);
                    std::cerr << "\"Unknown data type\"" << std::endl;
                    break;
            }
            printIndentation(indentLevel);
            std::cout << "}" << std::endl;
            break;

        case CryoNodeType::NODE_FUNCTION_DECLARATION:
            printIndentation(indentLevel);
            std::cout << "\"Function\": {" << std::endl;
            indentLevel++;
            printIndentation(indentLevel);
            std::cout << "\"Visibility\": \"" << CryoVisibilityTypeToString(node->data.functionDecl.function->visibility) << "\"," << std::endl;
            printIndentation(indentLevel);
            std::cout << "\"Name\": \"" << node->data.functionDecl.function->name << "\"," << std::endl;
            printIndentation(indentLevel);
            for (int i = 0; i < node->data.functionDecl.function->paramCount; ++i) {
                logASTNode(node->data.functionDecl.function->params[i], indentLevel + 1);
                if (i < node->data.functionDecl.function->paramCount - 1) {
                    printIndentation(indentLevel + 1);
                    std::cout << "," << std::endl;
                }
            }
            std::cout << "\"ReturnType\": \"" << CryoDataTypeToString(node->data.functionDecl.function->returnType) << "\"," << std::endl;
            logASTNode(node->data.functionDecl.function->body, indentLevel + 1);
            indentLevel--;
            printIndentation(indentLevel);
            std::cout << "}" << std::endl;
            break;

        case CryoNodeType::NODE_FUNCTION_BLOCK:
            printIndentation(indentLevel);
            std::cout << "\"FunctionBlock\": {" << std::endl;
            indentLevel++;
            printIndentation(indentLevel);
            std::cout << "\"Block\": [" << std::endl;

            if (!node->data.functionBlock.function) {
                printIndentation(indentLevel + 1);
                std::cout << "\"Function\": null" << std::endl;
            } else {
                logASTNode(node->data.functionBlock.function, indentLevel + 1);
            }

            if (node->data.functionBlock.block) {
                for (int i = 0; i < node->data.functionBlock.block->data.block.stmtCount; ++i) {
                    logASTNode(node->data.functionBlock.block->data.block.statements[i], indentLevel + 1);
                    if (i < node->data.functionBlock.block->data.block.stmtCount - 1) {
                        std::cout << "," << std::endl;
                    }
                }
            }

            std::cout << std::endl;
            printIndentation(indentLevel);
            std::cout << "]" << std::endl;
            indentLevel--;
            printIndentation(indentLevel);
            std::cout << "}" << std::endl;
            break;

        case CryoNodeType::NODE_RETURN_STATEMENT:
            printIndentation(indentLevel);
            std::cout << "\"ReturnStatement\": {" << std::endl;
        
            indentLevel++;
            printIndentation(indentLevel);
            std::cout << "\"Expression\": {" << std::endl;
        
            indentLevel++;
            logASTNode(node->data.returnStmt.expression, indentLevel);
            indentLevel--;
        
            printIndentation(indentLevel);
            std::cout << "}," << std::endl;
        
            printIndentation(indentLevel);
            std::cout << "\"Return Value\": " << node->data.returnStmt.returnValue << "," << std::endl;
        
            printIndentation(indentLevel);
            std::cout << "\"Return Type\": \"" << CryoDataTypeToString(node->data.returnStmt.returnType) << "\"" << std::endl;
        
            indentLevel--;
            printIndentation(indentLevel);
            std::cout << "}" << std::endl;
            break;


        case CryoNodeType::NODE_VAR_DECLARATION:
            printIndentation(indentLevel);
            std::cout << "\"Variable\": {" << std::endl;
            indentLevel++;
            printIndentation(indentLevel);
            std::cout << "\"Name\": \"" << (char*)node->data.varDecl.name << "\"," << std::endl;
            printIndentation(indentLevel);
            std::cout << (node->data.varDecl.isReference ? "\"Reference\": true" : "\"Reference\": false") << "," << std::endl;
            printIndentation(indentLevel);
            std::cout << "\"VarType\": \"" << CryoDataTypeToString(node->data.varDecl.dataType) << "\"," << std::endl;
            logASTNode(node->data.varDecl.initializer, indentLevel + 1);
            indentLevel--;
            printIndentation(indentLevel);
            std::cout << "}" << std::endl;
            break;

        case CryoNodeType::NODE_PARAM_LIST:
            printIndentation(indentLevel);
            std::cout << "\"Parameters\": {" << std::endl;
            indentLevel++;
            printIndentation(indentLevel);
            std::cout << "\"Params\": " << node->data.paramList.params << "," << std::endl;
            printIndentation(indentLevel);
            std::cout << "\"ParamCount\": " << node->data.paramList.paramCount << "," << std::endl;
            printIndentation(indentLevel);
            std::cout << "\"ParamCapacity\": " << node->data.paramList.paramCapacity << std::endl;
            indentLevel--;
            printIndentation(indentLevel);
            std::cout << "}" << std::endl;
            break;

        case CryoNodeType::NODE_BLOCK:
            printIndentation(indentLevel);
            std::cout << "\"Block\": {" << std::endl;
            indentLevel++;
            printIndentation(indentLevel);
            std::cout << "\"Statements\": " << node->data.block.stmtCount << "," << std::endl;
            printIndentation(indentLevel);
            std::cout << "\"StmtCapacity\": " << node->data.block.stmtCapacity << "," << std::endl;
            printIndentation(indentLevel);
            std::cout << "\"StatementNodes\": [" << std::endl;
            for (int i = 0; i < node->data.block.stmtCount; ++i) {
                logASTNode(node->data.block.statements[i], indentLevel + 1);
                if (i < node->data.block.stmtCount - 1) {
                    printIndentation(indentLevel + 1);
                    std::cout << "," << std::endl;
                }
            }
            std::cout << std::endl;
            printIndentation(indentLevel);
            std::cout << "]" << std::endl;
            indentLevel--;
            printIndentation(indentLevel);
            std::cout << "}" << std::endl;
            break;

        case CryoNodeType::NODE_BINARY_EXPR:
            printIndentation(indentLevel);
            std::cout << "\"BinaryOperation\": {" << std::endl;
            indentLevel++;
            printIndentation(indentLevel);
            std::cout << "\"Left\": ";
            switch (node->data.bin_op.left->type) {
                case CryoNodeType::NODE_VAR_NAME:
                    std::cout << "\"" << node->data.bin_op.left->data.varName.varName << "\"," << std::endl;
                    break;
                case CryoNodeType::NODE_LITERAL_EXPR:
                    switch (node->data.bin_op.left->data.literalExpression.dataType) {
                        case DATA_TYPE_INT:
                            std::cout << node->data.bin_op.left->data.literalExpression.intValue << "," << std::endl;
                            break;
                        case DATA_TYPE_STRING:
                            std::cout << "\"" << node->data.bin_op.left->data.literalExpression.stringValue << "\"," << std::endl;
                            break;
                        case DATA_TYPE_BOOLEAN:
                            std::cout << (node->data.bin_op.left->data.literalExpression.booleanValue ? "true" : "false") << "," << std::endl;
                            break;
                        default:
                            std::cerr << "\"<!UnknownDataType!>\"," << std::endl;
                            break;
                    }
                    break;
                default:
                    std::cerr << "\"<!UnknownNodeType!>\"," << std::endl;
                    break;
            }
            printIndentation(indentLevel);
            std::cout << "\"Right\": ";
            switch (node->data.bin_op.right->type) {
                case CryoNodeType::NODE_VAR_NAME:
                    std::cout << "\"" << node->data.bin_op.right->data.varName.varName << "\"," << std::endl;
                    break;
                case CryoNodeType::NODE_LITERAL_EXPR:
                    switch (node->data.bin_op.right->data.literalExpression.dataType) {
                        case DATA_TYPE_INT:
                            std::cout << node->data.bin_op.right->data.literalExpression.intValue << "," << std::endl;
                            break;
                        case DATA_TYPE_STRING:
                            std::cout << "\"" << node->data.bin_op.right->data.literalExpression.stringValue << "\"," << std::endl;
                            break;
                        case DATA_TYPE_BOOLEAN:
                            std::cout << (node->data.bin_op.right->data.literalExpression.booleanValue ? "true" : "false") << "," << std::endl;
                            break;
                        default:
                            std::cerr << "\"<!UnknownDataType!>\"," << std::endl;
                            break;
                    }
                    break;
                default:
                    std::cerr << "\"<!UnknownNodeType!>\"," << std::endl;
                    break;
            }
            printIndentation(indentLevel);
            std::cout << "\"Operator\": " << node->data.bin_op.op << "," << std::endl;
            printIndentation(indentLevel);
            std::cout << "\"OperatorText\": \"" << node->data.bin_op.operatorText << "\"" << std::endl;
            logASTNode(node->data.bin_op.left, indentLevel + 1);
            logASTNode(node->data.bin_op.right, indentLevel + 1);
            indentLevel--;
            printIndentation(indentLevel);
            std::cout << "}" << std::endl;
            break;

        case CryoNodeType::NODE_UNARY_EXPR:
            printIndentation(indentLevel);
            std::cout << "\"UnaryOperation\": {" << std::endl;
            indentLevel++;
            printIndentation(indentLevel);
            std::cout << "\"Operand\": " << node->data.unary_op.operand << "," << std::endl;
            printIndentation(indentLevel);
            std::cout << "\"Operator\": " << node->data.unary_op.op << std::endl;
            logASTNode(node->data.unary_op.operand, indentLevel + 1);
            indentLevel--;
            printIndentation(indentLevel);
            std::cout << "}" << std::endl;
            break;

        case CryoNodeType::NODE_VAR_NAME:
            printIndentation(indentLevel);
            std::cout << "\"VariableName\": \"" << node->data.varName.varName << "\"" << std::endl;
            break;

        case CryoNodeType::NODE_FUNCTION_CALL:
            printIndentation(indentLevel);
            std::cout << "\"FunctionCall\": {" << std::endl;
            indentLevel++;
            printIndentation(indentLevel);
            std::cout << "\"Name\": \"" << node->data.functionCall.name << "\"," << std::endl;
            printIndentation(indentLevel);
            std::cout << "\"Args\": [" << std::endl;
            for (int i = 1; i < node->data.functionCall.argCount; ++i) {
                logASTNode(node->data.functionCall.args[i], indentLevel + 1);
                if (i < node->data.functionCall.argCount - 1) {
                    printIndentation(indentLevel + 1);
                    std::cout << "," << std::endl;
                }
            }
            std::cout << std::endl;
            printIndentation(indentLevel);
            std::cout << "]" << std::endl;
            indentLevel--;
            printIndentation(indentLevel);
            std::cout << "}" << std::endl;
            break;

        case CryoNodeType::NODE_IF_STATEMENT:
            printIndentation(indentLevel);
            std::cout << "\"IfStatement\": {" << std::endl;
            indentLevel++;
            printIndentation(indentLevel);
            std::cout << "\"Condition\": {" << std::endl;
            logASTNode(node->data.ifStmt.condition, indentLevel + 1);
            printIndentation(indentLevel);
            std::cout << "}," << std::endl;
            printIndentation(indentLevel);
            std::cout << "\"Then\": {" << std::endl;
            logASTNode(node->data.ifStmt.thenBranch, indentLevel + 1);
            printIndentation(indentLevel);
            std::cout << "}," << std::endl;
            if (node->data.ifStmt.elseBranch) {
                printIndentation(indentLevel);
                std::cout << "\"Else\": {" << std::endl;
                logASTNode(node->data.ifStmt.elseBranch, indentLevel + 1);
                printIndentation(indentLevel);
                std::cout << "}," << std::endl;
            }
            indentLevel--;
            printIndentation(indentLevel);
            std::cout << "}" << std::endl;
            break;

        case CryoNodeType::NODE_WHILE_STATEMENT:
            printIndentation(indentLevel);
            std::cout << "\"WhileStatement\": {" << std::endl;
            indentLevel++;
            printIndentation(indentLevel);
            std::cout << "\"Condition\": {" << std::endl;
            logASTNode(node->data.whileStmt.condition, indentLevel + 1);
            printIndentation(indentLevel);
            std::cout << "}," << std::endl;
            printIndentation(indentLevel);
            std::cout << "\"Body\": {" << std::endl;
            logASTNode(node->data.whileStmt.body, indentLevel + 1);
            printIndentation(indentLevel);
            std::cout << "}" << std::endl;
            indentLevel--;
            printIndentation(indentLevel);
            std::cout << "}" << std::endl;
            break;

        case CryoNodeType::NODE_FOR_STATEMENT:
            printIndentation(indentLevel);
            std::cout << "\"ForStatement\": {" << std::endl;
            indentLevel++;
            printIndentation(indentLevel);
            std::cout << "\"Initializer\": {" << std::endl;
            logASTNode(node->data.forStmt.initializer, indentLevel + 1);
            printIndentation(indentLevel);
            std::cout << "}," << std::endl;
            printIndentation(indentLevel);
            std::cout << "\"Condition\": {" << std::endl;
            logASTNode(node->data.forStmt.condition, indentLevel + 1);
            printIndentation(indentLevel);
            std::cout << "}," << std::endl;
            printIndentation(indentLevel);
            std::cout << "\"Increment\": {" << std::endl;
            logASTNode(node->data.forStmt.increment, indentLevel + 1);
            printIndentation(indentLevel);
            std::cout << "}," << std::endl;
            printIndentation(indentLevel);
            std::cout << "\"Body\": {" << std::endl;
            logASTNode(node->data.forStmt.body, indentLevel + 1);
            printIndentation(indentLevel);
            std::cout << "}" << std::endl;
            indentLevel--;
            printIndentation(indentLevel);
            std::cout << "}" << std::endl;
            break;

        case CryoNodeType::NODE_STRING_LITERAL:
            printIndentation(indentLevel);
            std::cout << "\"StringLiteral\": \"" << node->data.literalExpression.stringValue << "\"" << std::endl;
            break;

        case CryoNodeType::NODE_BOOLEAN_LITERAL:
            printIndentation(indentLevel);
            std::cout << "\"BooleanLiteral\": " << (node->data.boolean.value ? "true" : "false") << std::endl;
            break;

        case CryoNodeType::NODE_ARRAY_LITERAL:
            printIndentation(indentLevel);
            std::cout << "\"ArrayLiteral\": {" << std::endl;
            indentLevel++;
            printIndentation(indentLevel);
            std::cout << "\"Elements\": " << node->data.arrayLiteral.elementCount << "," << std::endl;
            printIndentation(indentLevel);
            std::cout << "\"Capacity\": " << node->data.arrayLiteral.elementCount << "," << std::endl;
            printIndentation(indentLevel);
            std::cout << "\"Elements\": [" << std::endl;
            for (int i = 0; i < node->data.arrayLiteral.elementCount; ++i) {
                logASTNode(node->data.arrayLiteral.elements[i], indentLevel + 1);
                if (i < node->data.arrayLiteral.elementCount - 1) {
                    printIndentation(indentLevel + 1);
                    std::cout << "," << std::endl;
                }
            }
            std::cout << std::endl;
            printIndentation(indentLevel);
            std::cout << "]" << std::endl;
            indentLevel--;
            printIndentation(indentLevel);
            std::cout << "}" << std::endl;
            break;

        case CryoNodeType::NODE_EXTERN_STATEMENT:
            printIndentation(indentLevel);
            std::cout << "ExternStatement\": {" << std::endl;
            indentLevel++;
            printIndentation(indentLevel);
            std::cout << "\"Name\": \"" << node->data.externNode.decl.function->name << "\"," << std::endl;
            printIndentation(indentLevel);
            std::cout << "\n\t\tParams: " << "\n";
            for (int i = 0; i < node->data.externNode.decl.function->paramCount; ++i) {
                logASTNode(node->data.externNode.decl.function->params[i], indentLevel + 1);
                if (i < node->data.externNode.decl.function->paramCount - 1) {
                    printIndentation(indentLevel + 1);
                    std::cout << "," << std::endl;
                }
            }
            printIndentation(indentLevel);
            std::cout << "\"ReturnType\": \"" << CryoDataTypeToString(node->data.externNode.decl.function->returnType) << "\"" << std::endl;
            indentLevel--;
            printIndentation(indentLevel);
            std::cout << "}" << std::endl;
            break;

        case CryoNodeType::NODE_ARG_LIST:
            printIndentation(indentLevel);
            std::cout << "\"ArgumentList\": {" << std::endl;
            indentLevel++;
            printIndentation(indentLevel);
            std::cout << "\"Args\": " << node->data.argList.argCount << "," << std::endl;
            printIndentation(indentLevel);
            std::cout << "\"Capacity\": " << node->data.argList.argCapacity << "," << std::endl;
            printIndentation(indentLevel);
            std::cout << "\"Arguments\": [" << std::endl;
            for (int i = 0; i < node->data.argList.argCount; ++i) {
                logASTNode(node->data.argList.args[i], indentLevel + 1);
                if (i < node->data.argList.argCount - 1) {
                    printIndentation(indentLevel + 1);
                    std::cout << "," << std::endl;
                }
            }
            std::cout << std::endl;
            printIndentation(indentLevel);
            std::cout << "]" << std::endl;
            indentLevel--;
            printIndentation(indentLevel);
            std::cout << "}" << std::endl;
            break;
        
        case CryoNodeType::NODE_EXTERN_FUNCTION:
            printIndentation(indentLevel);
            std::cout << "\"ExternFunction\": {" << std::endl;
            indentLevel++;
            printIndentation(indentLevel);
            std::cout << "\"Name\": \"" << node->data.externNode.decl.function->name << "\"," << std::endl;
            printIndentation(indentLevel);
            std::cout << "\"ReturnType\": \"" << CryoDataTypeToString(node->data.externNode.decl.function->returnType) << "\"," << std::endl;
            printIndentation(indentLevel);
            std::cout << "\"Params\": [" << std::endl;
            for (int i = 0; i < node->data.externNode.decl.function->paramCount; ++i) {
                logASTNode(node->data.externNode.decl.function->params[i], indentLevel + 1);
                if (i < node->data.externNode.decl.function->paramCount - 1) {
                    printIndentation(indentLevel + 1);
                    std::cout << "," << std::endl;
                }
            }
            std::cout << std::endl;
            printIndentation(indentLevel);
            std::cout << "]" << std::endl;
            indentLevel--;
            printIndentation(indentLevel);
            std::cout << "}" << std::endl;
            break;
        default:
            printIndentation(indentLevel);
            std::cerr << "\"<!Defaulted: UnknownNodeType!>\"" << std::endl;
            break;
    }

    indentLevel--;
    printIndentation(indentLevel);
    std::cout << "}" << std::endl;
}


