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


void logNode(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case CryoNodeType::NODE_PROGRAM:
            std::cout   << "\n### Program Node ###"
                        << "\nType:\t" << node->type
                        << "\nStatements:\t" << node->data.program.stmtCount
                        << "\nCapacity:\t" << node->data.program.stmtCapacity
                        << "\nStatements:\t"
                        << "\n######################"
                        << std::endl;
            for (int i = 0; i < node->data.program.stmtCount; ++i) {
                logNode(node->data.program.statements[i]);
            }
            break;
        
        case CryoNodeType::NODE_STATEMENT:
            std::cout   << "\n### Statement Node ###"
                        << "\nType:\t" << node->type
                        << "\nStatement:\t"
                        << "\n######################"
                        << std::endl;
            logNode(node->data.stmt.stmt);
            break;

        case CryoNodeType::NODE_EXPRESSION:
            std::cout   << "\n### Expression Node ###"
                        << "\nType:\t" << node->type
                        << "\nExpression:\t"
                        << "\n######################"
                        << std::endl;
            logNode(node->data.expr.expr);
            break;

        case CryoNodeType::NODE_LITERAL_EXPR:
            std::cout   << "### Literal Node ###\nType:\t" 
                        << node->type 
                        << "\n######################"
                        << std::endl;
            switch (node->data.literalExpression.dataType) {
                case DATA_TYPE_INT:
                    std::cout << "Integer Literal: " << node->data.literalExpression.intValue << std::endl;
                    break;
                case DATA_TYPE_STRING:
                    std::cout << "String Literal: " << node->data.literalExpression.stringValue << std::endl;
                    break;
                // Handle other types
                default:
                    std::cerr << "Unknown data type\n";
                    break;
            }
            break;


        case CryoNodeType::NODE_FUNCTION_DECLARATION:
            std::cout   << "\n### Function Node ###"
                        << "\nType:\t" << node->type
                        << "\nVisibility:\t" << node->data.functionDecl.visibility
                        << "\nName:\t" << node->data.functionDecl.name
                        << "\nParams:\t" << node->data.functionDecl.params
                        << "\nReturn Type:\t" << node->data.functionDecl.returnType
                        << "\nBody:\t" << node->data.functionDecl.body
                        << "\n######################"
                        << std::endl;
            logNode(node->data.functionDecl.body);
            break;
        
        case CryoNodeType::NODE_VAR_DECLARATION:
            std::cout   << "\n### Variable Node ###"
                        << "\nType:\t" << node->type
                        << "\nName:\t" << node->data.varDecl.name
                        << "\nVar Type:\t" << node->data.varDecl.dataType
                        << "\nInitializer:\t"
                        << "\n######################"
                        << std::endl;
            logNode(node->data.varDecl.initializer);
            break;

        case CryoNodeType::NODE_PARAM_LIST:
            std::cout   << "\n### Parameter Node ###"
                        << "\nType:\t" << node->type
                        << "\nParams:\t" << node->data.paramList.params
                        << "\nParam Count:\t" << node->data.paramList.paramCount
                        << "\nParam Capacity:\t" << node->data.paramList.paramCapacity
                        << "\n######################"
                        << std::endl;
            break;
        
        case CryoNodeType::NODE_BLOCK:
            std::cout   << "\n### Block Node ###"
                        << "\nType:\t" << node->type
                        << "\nStatements:\t" << node->data.block.statements
                        << "\nStmt Count:\t" << node->data.block.stmtCount
                        << "\nStmt Capacity:\t" << node->data.block.stmtCapacity
                        << "\n######################"
                        << std::endl;
            logNode(*node->data.block.statements);
            break;

        case CryoNodeType::NODE_BINARY_EXPR:
            std::cout   << "\n### Binary Operation Node ###"
                        << "\nType:\t" << node->type
                        << "\nLeft:\t" << node->data.bin_op.left
                        << "\nRight:\t" << node->data.bin_op.right
                        << "\nOperator:\t" << node->data.bin_op.op
                        << "\nOperator Text:\t" << node->data.bin_op.operatorText
                        << "\n######################"
                        << std::endl;
            logNode(node->data.bin_op.left);
            logNode(node->data.bin_op.right);
            break;

        default:
            std::cerr << "Unknown node type\n";
            break;
    }
}




