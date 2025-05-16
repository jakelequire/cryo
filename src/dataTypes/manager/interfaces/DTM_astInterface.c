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
#include "frontend/frontendSymbolTable.h"
#include "dataTypes/dataTypeManager.h"

// --------------------------------------------------------------------------------------------------- //
// ---------------------------------- AST Interface Implementation ----------------------------------- //
// --------------------------------------------------------------------------------------------------- //

DataType *DTMastInterface_getTypeofASTNode(ASTNode *node)
{
    if (!node || node == NULL)
    {
        logMessage(LMI, "ERROR", "DTM", "Node is Null or Undefined!");
        fprintf(stderr, "Node is Null or Undefined!\n");
        DEBUG_PRINT_FILTER({
            FEST->printTable(FEST);
        });
        CONDITION_FAILED;
    }

    CryoNodeType nodeType = node->metaData->type;
    switch (nodeType)
    {
    case NODE_PROGRAM:
        fprintf(stderr, "Invalid Node Type! {Program}\n");
        CONDITION_FAILED;
        break;
    case NODE_FUNCTION_DECLARATION:
    {
        DataType *functionType = node->data.functionDecl->type;
        return functionType;
        break;
    }
    case NODE_VAR_DECLARATION:
    {
        DataType *varType = node->data.varDecl->type;
        return varType;
        break;
    }
    case NODE_STATEMENT:
        fprintf(stderr, "Invalid Node Type! {Statement}\n");
        CONDITION_FAILED;
        break;
    case NODE_EXPRESSION:
        fprintf(stderr, "Invalid Node Type! {Expression}\n");
        CONDITION_FAILED;
        break;
    case NODE_BINARY_EXPR:
    {
        DataType *leftType = DTMastInterface_getTypeofASTNode(node->data.bin_op->left);
        DataType *rightType = DTMastInterface_getTypeofASTNode(node->data.bin_op->right);
        if (leftType && rightType)
        {
            return leftType;
        }
        else
        {
            fprintf(stderr, "Invalid Node Type! {Binary Expression}\n");
            CONDITION_FAILED;
        }
        break;
    }
    case NODE_UNARY_EXPR:
    {
        DataType *exprType = DTMastInterface_getTypeofASTNode(node->data.unary_op->expression);
        return exprType;
        break;
    }
    case NODE_LITERAL_EXPR:
    {
        DataType *literalType = node->data.literal->type;
        return literalType;
        break;
    }
    case NODE_VAR_NAME:
    {
        DataType *varType = node->data.varName->type;
        return varType;
        break;
    }
    case NODE_FUNCTION_CALL:
    {
        DataType *functionType = node->data.functionCall->returnType;
        return functionType;
        break;
    }
    case NODE_IF_STATEMENT:
        fprintf(stderr, "Invalid Node Type! {If Statement}\n");
        CONDITION_FAILED;
        break;
    case NODE_WHILE_STATEMENT:
        fprintf(stderr, "Invalid Node Type! {While Statement}\n");
        CONDITION_FAILED;
        break;
    case NODE_FOR_STATEMENT:
        fprintf(stderr, "Invalid Node Type! {For Statement}\n");
        CONDITION_FAILED;
        break;
    case NODE_RETURN_STATEMENT:
    {
        DataType *returnType = DTMastInterface_getTypeofASTNode(node->data.returnStatement->expression);
        return returnType;
        break;
    }
    case NODE_BLOCK:
        fprintf(stderr, "Invalid Node Type! {Block}\n");
        CONDITION_FAILED;
        break;
    case NODE_FUNCTION_BLOCK:
        fprintf(stderr, "Invalid Node Type! {Function Block}\n");
        CONDITION_FAILED;
        break;
    case NODE_EXPRESSION_STATEMENT:
        fprintf(stderr, "Invalid Node Type! {Expression Statement}\n");
        CONDITION_FAILED;
        break;
    case NODE_ASSIGN:
    {
        DataType *assignType = DTMastInterface_getTypeofASTNode(node->data.propertyAccess->property);
        return assignType;
        break;
    }
    case NODE_PARAM_LIST:
        fprintf(stderr, "Invalid Node Type! {Param List}\n");
        CONDITION_FAILED;
        break;
    case NODE_PARAM:
    {
        DataType *paramType = node->data.param->type;
        return paramType;
        break;
    }
    case NODE_TYPE:
    {
        DataType *typeType = node->data.typeDecl->type;
        return typeType;
        break;
    }
    case NODE_STRING_LITERAL:
    {
        DataType *stringType = DTM->primitives->createStr();
        return stringType;
        break;
    }
    case NODE_STRING_EXPRESSION:
    {
        DataType *stringType = DTM->primitives->createStr();
        return stringType;
        break;
    }
    case NODE_BOOLEAN_LITERAL:
    {
        DataType *boolType = DTM->primitives->createBoolean();
        return boolType;
        break;
    }
    case NODE_ARRAY_LITERAL:
    {
        DataType *arrayType = node->data.array->type;
        return arrayType;
        break;
    }
    case NODE_IMPORT_STATEMENT:
        fprintf(stderr, "Invalid Node Type! {Import Statement}\n");
        CONDITION_FAILED;
        break;
    case NODE_EXTERN_STATEMENT:
        fprintf(stderr, "Invalid Node Type! {Extern Statement}\n");
        CONDITION_FAILED;
        break;
    case NODE_EXTERN_FUNCTION:
    {
        DataType *functionType = node->data.externFunction->type;
        return functionType;
        break;
    }
    case NODE_ARG_LIST:
        fprintf(stderr, "Invalid Node Type! {Arg List}\n");
        CONDITION_FAILED;
        break;
    case NODE_NAMESPACE:
        fprintf(stderr, "Invalid Node Type! {Namespace}\n");
        CONDITION_FAILED;
        break;
    case NODE_INDEX_EXPR:
    {
        DataType *indexType = DTMastInterface_getTypeofASTNode(node->data.indexExpr->index);
        return indexType;
        break;
    }
    case NODE_VAR_REASSIGN:
    {
        DataType *reassignType = node->data.varReassignment->existingVarType;
        return reassignType;
        break;
    }
    case NODE_STRUCT_DECLARATION:
    {
        DataType *structType = node->data.structNode->type;
        return structType;
        break;
    }
    case NODE_PROPERTY:
    {
        DataType *propertyType = node->data.property->type;
        return propertyType;
        break;
    }
    case NODE_CUSTOM_TYPE:
        break;
    case NODE_SCOPED_FUNCTION_CALL:
    {
        DataType *functionType = node->data.scopedFunctionCall->type;
        return functionType;
        break;
    }
    case NODE_EXTERNAL_SYMBOL:
        fprintf(stderr, "Invalid Node Type! {External Symbol}\n");
        CONDITION_FAILED;
        break;
    case NODE_STRUCT_CONSTRUCTOR:
        fprintf(stderr, "Invalid Node Type! {Struct Constructor}\n");
        CONDITION_FAILED;
        break;
    case NODE_PROPERTY_ACCESS:
    {
        DataType *propertyType = node->data.propertyAccess->objectType;
        return propertyType;
        break;
    }
    case NODE_THIS:
        fprintf(stderr, "Invalid Node Type! {This}\n");
        CONDITION_FAILED;
        break;
    case NODE_THIS_ASSIGNMENT:
        fprintf(stderr, "Invalid Node Type! {This Assignment}\n");
        CONDITION_FAILED;
        break;
    case NODE_PROPERTY_REASSIGN:
        fprintf(stderr, "Invalid Node Type! {Property Reassignment}\n");
        CONDITION_FAILED;
        break;
    case NODE_METHOD:
    {
        DataType *methodType = node->data.method->type;
        return methodType;
        break;
    }
    case NODE_IDENTIFIER:
        fprintf(stderr, "Invalid Node Type! {Identifier}\n");
        CONDITION_FAILED;
        break;
    case NODE_METHOD_CALL:
    {
        DataType *methodType = node->data.methodCall->returnType;
        return methodType;
        break;
    }
    case NODE_ENUM:
    {
        DataType *enumType = node->data.enumNode->type;
        return enumType;
        break;
    }
    case NODE_GENERIC_DECL:
    {
        DataType *genericType = node->data.genericDecl->type;
        return genericType;
        break;
    }
    case NODE_GENERIC_INST:
    {
        DataType *genericType = node->data.genericInst->resultType;
        return genericType;
        break;
    }
    case NODE_CLASS:
    {
        DataType *classType = node->data.classNode->type;
        return classType;
        break;
    }
    case NODE_CLASS_CONSTRUCTOR:
        fprintf(stderr, "Invalid Node Type! {Class Constructor}\n");
        CONDITION_FAILED;
        break;
    case NODE_OBJECT_INST:
    {
        DataType *objectType = node->data.objectNode->objType;
        return objectType;
        break;
    }
    case NODE_NULL_LITERAL:
    {
        DataType *nullType = DTM->primitives->createNull();
        return nullType;
        break;
    }
    case NODE_TYPE_CAST:
        // TODO
        break;
    case NODE_TYPEOF:
        // Skip
        break;
    case NODE_USING:
        // Skip
        break;
    case NODE_MODULE:
        // Skip
        break;
    case NODE_ANNOTATION:
        // Skip
        break;
    case NODE_UNKNOWN:
        break;
    default:
    {
        const char *nodeTypeStr = CryoNodeTypeToString(nodeType);
        fprintf(stderr, "[Data Type Manager] Error: Unknown node type: %s\n", nodeTypeStr);
        CONDITION_FAILED;
    }
    }
}

DataType **DTMastInterface_createTypeArrayFromAST(ASTNode *node)
{
    fprintf(stderr, "[Data Type Manager] Error: Invalid AST Node (createTypeArrayFromAST)\n");
    return NULL;
}

DataType **DTMastInterface_createTypeArrayFromASTArray(ASTNode **nodes, int count)
{
    if (!nodes)
    {
        fprintf(stderr, "[Data Type Manager] Error: Invalid AST Node Array (createTypeArrayFromAST)\n");
        CONDITION_FAILED;
    }
    if (count == 0)
    {
        return NULL;
    }

    DataType **types = (DataType **)malloc(sizeof(DataType *) * count);
    if (!types)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate memory for type array (createTypeArrayFromAST)\n");
        CONDITION_FAILED;
    }

    for (int i = 0; i < count; i++)
    {
        ASTNode *node = nodes[i];
        DataType *type = DTM->astInterface->getTypeofASTNode(node);
        if (!type)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to get type of AST Node (createTypeArrayFromASTArray)\n");
            CONDITION_FAILED;
        }
        types[i] = type;
    }

    return types;
}

DTPropertyTy **DTMastInterface_createPropertyArrayFromAST(ASTNode **nodes, int count)
{
    if (!nodes)
    {
        fprintf(stderr, "[Data Type Manager] Error: Invalid AST Node Array (createPropertyArrayFromAST)\n");
        CONDITION_FAILED;
    }

    if (count == 0)
    {
        return NULL;
    }

    DTPropertyTy **properties = (DTPropertyTy **)malloc(sizeof(DTPropertyTy *) * count);
    if (!properties)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate memory for property array (createPropertyArrayFromAST)\n");
        CONDITION_FAILED;
    }

    for (int i = 0; i < count; i++)
    {
        ASTNode *node = nodes[i];
        DataType *type = DTM->astInterface->getTypeofASTNode(node);
        if (!type)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to get type of AST Node (createPropertyArrayFromAST)\n");
            CONDITION_FAILED;
        }

        DTPropertyTy *property = DTM->propertyTypes->createPropertyTemplate();
        property->setName(property, node->data.property->name);
        property->setType(property, type);
        properties[i] = property;
    }

    return properties;
}

DTMastInterface *createDTMAstInterface(void)
{
    DTMastInterface *astInterface = (DTMastInterface *)malloc(sizeof(DTMastInterface));
    if (!astInterface)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM AST Interface\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    astInterface->getTypeofASTNode = DTMastInterface_getTypeofASTNode;
    astInterface->createTypeArrayFromAST = DTMastInterface_createTypeArrayFromAST;
    astInterface->createTypeArrayFromASTArray = DTMastInterface_createTypeArrayFromASTArray;
    astInterface->createPropertyArrayFromAST = DTMastInterface_createPropertyArrayFromAST;

    return astInterface;
}