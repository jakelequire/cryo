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
#include "compiler/semantics.h"

// >===------------------------------------===< //
// >===--------- Semantic Checks ----------===< //
// >===------------------------------------===< //
// Check variable usage
void checkVariable(ASTNode *node, CryoSymbolTable *table, Arena *arena)
{
    CryoSymbol *symbol = findSymbol(table, node->data.varDecl->name, arena);
    if (!symbol)
    {
        fprintf(stderr, "[Semantics] Error: Undefined variable '%s'\n", node->data.varDecl->name);
        exit(1);
    }
}

// Check if an assignment is valid
void checkAssignment(ASTNode *node, CryoSymbolTable *table, Arena *arena)
{
    CryoSymbol *symbol = findSymbol(table, node->data.varName->varName, arena);
    if (!symbol)
    {
        fprintf(stderr, "[Sema] Error: Undefined variable '%s' in assignment\n", node->data.varName->varName);
        exit(1);
    }
    if (symbol->isConstant)
    {
        fprintf(stderr, "[Sema] Error: Cannot assign to constant variable '%s'\n", node->data.varName->varName);
        exit(1);
    }

    // Assuming the right-hand side of the assignment is an expression
    ASTNode *rhsNode = node->data.bin_op->right; // Assuming assignment is a binary operation
    CryoDataType rhsType = getExpressionType(rhsNode, arena);

    if (symbol->valueType != rhsType)
    {
        fprintf(stderr, "[Sema] Error: Type mismatch in assignment to '%s'\n", node->data.varName->varName);
        exit(1);
    }
}

// Check a function call
void checkFunctionCall(ASTNode *node, CryoSymbolTable *table, Arena *arena)
{
    CryoSymbol *symbol = findSymbol(table, node->data.functionCall->name, arena);
    if (!symbol || symbol->valueType != DATA_TYPE_FUNCTION)
    {
        fprintf(stderr, "[Sema] Error: Undefined function '%s'\n", node->data.functionCall->name);
        exit(1);
    }
    if (node->data.functionCall->argCount != symbol->argCount)
    {
        fprintf(stderr, "[Sema] Error: Argument count mismatch in function call to '%s'\n", node->data.functionCall->name);
        exit(1);
    }
}

void checkFunctionDeclaration(ASTNode *node, CryoSymbolTable *table, Arena *arena)
{
    enterScope(table, arena);

    // Add function parameters to the symbol table
    for (int i = 0; i < node->data.functionDecl->paramCount; i++)
    {
        ASTNode *param = node->data.functionDecl->params[i];
        // Add param to symbol table
    }

    // Traverse the function body
    // traverseAST(node->data.functionDecl->body, table);

    // Ensure return type matches
    CryoDataType returnType = getExpressionType(node->data.functionDecl->body, arena);
    if (node->data.functionDecl->returnType != returnType)
    {
        fprintf(stderr, "[Sema] Error: Return type mismatch in function '%s'.\n", node->data.functionDecl->name);
        exit(1);
    }

    exitScope(table, arena);
}

void checkBinaryExpression(ASTNode *node, CryoSymbolTable *table, Arena *arena)
{
    if (!node)
        return;

    ASTNode *left = node->data.bin_op->left;
    ASTNode *right = node->data.bin_op->right;

    // Recursively check both operands
    // traverseAST(left, table);
    // traverseAST(right, table);

    CryoDataType leftType = getExpressionType(left, arena);
    CryoDataType rightType = getExpressionType(right, arena);

    // Check if types of left and right operands are compatible
    if (leftType != rightType)
    {
        fprintf(stderr, "[Sema] Error: Type mismatch in binary expression.\n");
        exit(1);
    }

    // Assign result type to the expression node
    // This might need to be adjusted based on your exact AST structure
}

// Helper function to get the type of an expression
CryoDataType getExpressionType(ASTNode *node, Arena *arena)
{
    switch (node->metaData->type)
    {
    case NODE_LITERAL_EXPR:
        return node->data.literal->dataType;
    case NODE_VAR_NAME:
        // You might need to look up the variable in the symbol table here
        return node->data.varName->refType;
    case NODE_BINARY_EXPR:
    // Add more cases as needed
    default:
        fprintf(stderr, "[Sema] Error: Unknown expression type.\n");
        exit(1);
    }
}
