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
#include "frontend/AST.h"

// <printAST>
void printAST(ASTNode *node, int indent, Arena *arena)
{
    if (!node)
        return;

    for (int i = 0; i < indent; i++)
        printf(" ");

    switch (node->metaData->type)
    {
    case NODE_PROGRAM:
        printf("Program Node with %zu statements (capacity: %zu)\n",
               node->data.program->statementCount,
               node->data.program->statementCapacity);
        for (size_t i = 0; i < node->data.program->statementCount; i++)
        {
            printAST(node->data.program->statements[i], indent + 2, arena);
        }
        break;

    case NODE_EXTERN_FUNCTION:
        printf("\nExtern Function Node: %s\n", strdup(node->data.externFunction->name));
        printf("Extern Function Return Type: %s\n", DataTypeToString(node->data.externFunction->type));
        printf("Extern Function Parameters:\n");
        for (int i = 0; i < node->data.externFunction->paramCount; i++)
        {
            printAST(node->data.externFunction->params[i], indent + 2, arena);
        }
        break;

    case NODE_FUNCTION_DECLARATION:
        printf("\nFunction Declaration Node name: %s\n", strdup(node->data.functionDecl->name));
        printf("Function Declaration Node returnType: %s\n",
               DataTypeToString(node->data.functionDecl->type));
        printf("Function Declaration Node visibility: %s\n",
               CryoVisibilityTypeToString(node->data.functionDecl->visibility));
        printf("Function Declaration Node params:\n");
        for (int i = 0; i < node->data.functionDecl->paramCount; i++)
        {
            printAST(node->data.functionDecl->params[i], indent + 2, arena);
        }
        printf("Function Declaration Node body:\n");
        printAST(node->data.functionDecl->body, indent + 2, arena);
        break;

    case NODE_VAR_DECLARATION:
        printf("\nVariable Declaration Node: %s\n", strdup(node->data.varDecl->name));
        printf("Variable Type: %s\n", DataTypeToString(node->data.varDecl->type));
        printf("Is Global: %s\n", node->data.varDecl->isGlobal ? "true" : "false");
        printf("Is Reference: %s\n", node->data.varDecl->isReference ? "true" : "false");
        printf("Is Mutable: %s\n", node->data.varDecl->isMutable ? "true" : "false");
        printf("Is Iterator: %s\n", node->data.varDecl->isIterator ? "true" : "false");
        printf("Has Index Expression: %s\n", node->data.varDecl->hasIndexExpr ? "true" : "false");

        if (node->data.varDecl->initializer)
        {
            printf("Initializer:");
            printAST(node->data.varDecl->initializer, indent + 2, arena);
        }
        else
        {
            printf("No Initializer\n");
        }
        break;

    case NODE_EXPRESSION:
        printf("\nExpression Node\n");
        break;

    case NODE_BINARY_EXPR:
        printf("\nBinary Expression Node: %s\n", CryoOperatorTypeToString(node->data.bin_op->op));
        printf("\nLeft Operand:");
        printAST(node->data.bin_op->left, indent + 2, arena);
        printf("\nOperator: %s\n", CryoOperatorTypeToString(node->data.bin_op->op));
        printf("\nRight Operand:");
        printAST(node->data.bin_op->right, indent + 2, arena);
        printf("\n");
        break;

    case NODE_UNARY_EXPR:
        printf("\nUnary Expression Node: %s\n", CryoTokenToString(node->data.unary_op->op));
        break;

    case NODE_LITERAL_EXPR:
        printf("\nLiteral Expression Node: ");
        switch (node->data.literal->type->container->primitive)
        {
        case PRIM_INT:
            printf("\n\tInteger Literal Node: %d\n", node->data.literal->value.intValue);
            break;
        case PRIM_FLOAT:
            printf("\n\tFloat Literal Node: %f\n", node->data.literal->value.floatValue);
            break;
        case PRIM_STRING:
            printf("\n\tString Literal Node: %s\n", strdup(node->data.literal->value.stringValue));
            break;
        case PRIM_BOOLEAN:
            printf("\n\tBoolean Literal Node: %s\n", node->data.literal->value.booleanValue ? "true" : "false");
            break;
        default:
            printf("\n\tUnknown Literal Type\n");
        }
        break;
    case NODE_STRING_EXPRESSION:
        printf("\nString Expression Node: %s\n", strdup(node->data.literal->value.stringValue));
        break;
    case NODE_VAR_NAME:
        printf("\nVariable Name Node: %s\n", strdup(node->data.varName->varName));
        printf("Is Reference: %s\n", node->data.varName->isRef ? "true" : "false");
        break;

    case NODE_FUNCTION_CALL:
        printf("\nFunction Call Node: %s\n", strdup(node->data.functionCall->name));
        printf("Arguments:\n");
        for (int i = 0; i < node->data.functionCall->argCount; i++)
        {
            printAST(node->data.functionCall->args[i], indent + 2, arena);
        }
        break;

    case NODE_IF_STATEMENT:
        printf("\nIf Statement Node\n");
        printf("Condition:\n");
        printAST(node->data.ifStatement->condition, indent + 2, arena);
        printf("Then Branch:\n");
        printAST(node->data.ifStatement->thenBranch, indent + 2, arena);
        if (node->data.ifStatement->elseBranch)
        {
            printf("Else Branch:\n");
            printAST(node->data.ifStatement->elseBranch, indent + 2, arena);
        }
        break;

    case NODE_WHILE_STATEMENT:
        printf("\nWhile Statement Node\n");
        printf("Condition:\n");
        printAST(node->data.whileStatement->condition, indent + 2, arena);
        printf("Body:\n");
        printAST(node->data.whileStatement->body, indent + 2, arena);
        break;

    case NODE_FOR_STATEMENT:
        printf("\nFor Statement Node\n");
        printf("Initializer:\n");
        printAST(node->data.forStatement->initializer, indent + 2, arena);
        printf("Condition:\n");
        printAST(node->data.forStatement->condition, indent + 2, arena);
        printf("Increment:\n");
        printAST(node->data.forStatement->increment, indent + 2, arena);
        printf("Body:\n");
        printAST(node->data.forStatement->body, indent + 2, arena);
        break;

    case NODE_RETURN_STATEMENT:
        printf("\nReturn Statement Node\n");
        if (node->data.returnStatement->returnValue)
        {
            printAST(node->data.returnStatement->returnValue, indent + 2, arena);
        }
        break;

    case NODE_BLOCK:
        printf("\nBlock Node with %d statements (capacity: %d)\n",
               node->data.block->statementCount,
               node->data.block->statementCapacity);
        for (int i = 0; i < node->data.block->statementCount; i++)
        {
            printAST(node->data.block->statements[i], indent + 2, arena);
        }
        break;

    case NODE_FUNCTION_BLOCK:
        printf("\nFunction Block Node\n");
        printf("Function:\n");
        printAST(node->data.functionBlock->function, indent + 2, arena);
        printf("Block:\n");
        for (int i = 0; i < node->data.functionBlock->statementCount; i++)
        {
            printAST(node->data.functionBlock->statements[i], indent + 2, arena);
        }
        break;

    case NODE_PARAM_LIST:
        printf("\nParameter List Node with %d parameters (capacity: %d)\n",
               node->data.paramList->paramCount,
               node->data.paramList->paramCapacity);
        for (int i = 0; i < node->data.paramList->paramCount; i++)
        {
            printAST((ASTNode *)node->data.paramList->params[i], indent + 2, arena);
        }
        break;

    case NODE_ARG_LIST:
        printf("\nArgument List Node with %d arguments (capacity: %d)\n",
               node->data.argList->argCount,
               node->data.argList->argCapacity);
        for (int i = 0; i < node->data.argList->argCount; i++)
        {
            printAST((ASTNode *)node->data.argList->args[i], indent + 2, arena);
        }
        break;

    case NODE_ARRAY_LITERAL:
        printf("\nArray Literal Node with %d elements (capacity: %d)\n",
               node->data.array->elementCount,
               node->data.array->elementCapacity);
        for (int i = 0; i < node->data.array->elementCount; i++)
        {
            printAST(node->data.array->elements[i], indent + 2, arena);
        }
        break;

    case NODE_NAMESPACE:
        printf("\nNamespace Node: %s\n", node->data.cryoNamespace->name);
        break;

    case NODE_INDEX_EXPR:
    {
        printf("\nIndex Expression Node\n");
        printf("Array:\n");
        printAST(node->data.indexExpr->array, indent + 2, arena);
        printf("Index:\n");
        printAST(node->data.indexExpr->index, indent + 2, arena);
        break;
    }

    case NODE_VAR_REASSIGN:
    {
        printf("\nVariable Reassignment Node\n");
        printf("Referenced Variable Name: %s\n", node->data.varReassignment->existingVarName);
        printf("Existing Value:\n");
        printAST(node->data.varReassignment->existingVarNode, indent + 2, arena);
        printf("New Value:\n");
        printAST(node->data.varReassignment->newVarNode, indent + 2, arena);
        break;
    }

    case NODE_PARAM:
    {
        printf("\nParameter Node: %s\n", node->data.param->name);
        printf("Parameter Type: %s\n", DataTypeToString(node->data.param->type));
        printf("Function Name: %s\n", node->data.param->functionName);
        printf("Has Default Value: %s\n", node->data.param->hasDefaultValue ? "true" : "false");
        if (node->data.param->hasDefaultValue)
        {
            printf("Default Value:\n");
            printAST(node->data.param->defaultValue, indent + 2, arena);
        }
        break;
    }

    case NODE_PROPERTY:
    {
        printf("\nProperty Node: %s\n", node->data.property->name);
        printf("Property Type: %s\n", DataTypeToString(node->data.property->type));
        printf("Property Value:\n");
        printAST(node->data.property->value, indent + 2, arena);
        break;
    }

    case NODE_STRUCT_DECLARATION:
    {
        printf("\nStruct Declaration Node: %s\n", node->data.structNode->name);
        printf("Struct Properties:\n");
        for (int i = 0; i < node->data.structNode->propertyCount; i++)
        {
            printAST(node->data.structNode->properties[i], indent + 2, arena);
        }
        break;
    }

    case NODE_IMPORT_STATEMENT:
    {
        printf("\nImport Statement Node: \n");
        printf("Is STD Module: %s\n", node->data.import->isStdModule ? "true" : "false");
        printf("Module Name: %s\n", node->data.import->moduleName);
        if (node->data.import->subModuleName)
        {
            printf("Submodule Name: %s\n", node->data.import->subModuleName);
        }
        break;
    }

    case NODE_EXTERNAL_SYMBOL:
        printf("\nExternal Symbol Node\n");
        break;

    case NODE_SCOPED_FUNCTION_CALL:
        printf("\nScoped Function Call Node\n");
        printf("Function Name: %s\n", node->data.scopedFunctionCall->functionName);
        printf("Scope Name: %s\n", node->data.scopedFunctionCall->scopeName);
        printf("Arg Count: %d\n", node->data.scopedFunctionCall->argCount);
        printf("Arguments:\n");
        for (int i = 0; i < node->data.scopedFunctionCall->argCount; i++)
        {
            printAST(node->data.scopedFunctionCall->args[i], indent + 2, arena);
        }
        break;

    case NODE_UNKNOWN:
        printf("\n<Unknown Node>\n");
        break;

    default:
        printf("\n <!> Unhandled Node Type: %s\n", CryoNodeTypeToString(node->metaData->type));
        break;
    }
}
// </printAST>