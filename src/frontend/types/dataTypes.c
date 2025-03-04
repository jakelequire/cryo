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
#include "symbolTable/cInterfaceTable.h"
#include "frontend/dataTypes.h"
#include "diagnostics/diagnostics.h"

// Create new TypeContainer
TypeContainer *createTypeContainer(void)
{
    __STACK_FRAME__
    TypeContainer *container = (TypeContainer *)malloc(sizeof(TypeContainer));
    if (!container)
    {
        fprintf(stderr, "[DataTypes] Error: Failed to allocate TypeContainer\n");
        CONDITION_FAILED;
    }

    container->baseType = UNKNOWN_TYPE;
    container->primitive = PRIM_UNKNOWN;
    container->size = 0;
    container->length = 0;
    container->isArray = false;
    container->boolValue = false;
    container->arrayDimensions = 0;
    container->custom.name = (char *)malloc(sizeof(char) * 64);
    container->custom.structDef = NULL;
    container->custom.funcDef = NULL;
    container->custom.structDef = NULL;
    container->custom.arrayDef = NULL;
    container->isPointer = false;
    container->isReference = false;

    return container;
}

DataType *parseDataType(const char *typeStr, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "DataTypes", "Parsing data type '%s'", typeStr);
    // Check if `[]` is at the end of the type string
    size_t len = strlen(typeStr);
    bool isArray = (len >= 2 && typeStr[len - 2] == '[' && typeStr[len - 1] == ']');

    // Create a copy of typeStr without the '[]' if it's an array
    char *baseTypeStr = strdup(typeStr);
    if (isArray)
    {
        baseTypeStr[len - 2] = '\0';
    }

    // First create the base type container
    TypeContainer *container = createTypeContainer();
    if (!container)
    {
        free(baseTypeStr);
        CONDITION_FAILED;
    }

    // Handle primitive types
    if (strcmp(baseTypeStr, "int") == 0)
    {
        container->baseType = PRIMITIVE_TYPE;
        container->primitive = PRIM_INT;
    }
    else if (strcmp(baseTypeStr, "i8") == 0)
    {
        container->baseType = PRIMITIVE_TYPE;
        container->primitive = PRIM_I8;
    }
    else if (strcmp(baseTypeStr, "i16") == 0)
    {
        container->baseType = PRIMITIVE_TYPE;
        container->primitive = PRIM_I16;
    }
    else if (strcmp(baseTypeStr, "i32") == 0)
    {
        container->baseType = PRIMITIVE_TYPE;
        container->primitive = PRIM_I32;
    }
    else if (strcmp(baseTypeStr, "i64") == 0)
    {
        container->baseType = PRIMITIVE_TYPE;
        container->primitive = PRIM_I64;
    }
    else if (strcmp(baseTypeStr, "i128") == 0)
    {
        container->baseType = PRIMITIVE_TYPE;
        container->primitive = PRIM_I128;
    }
    else if (strcmp(baseTypeStr, "float") == 0)
    {
        container->baseType = PRIMITIVE_TYPE;
        container->primitive = PRIM_FLOAT;
    }
    else if (strcmp(baseTypeStr, "string") == 0)
    {
        container->baseType = PRIMITIVE_TYPE;
        container->primitive = PRIM_STRING;
    }
    else if (strcmp(baseTypeStr, "boolean") == 0)
    {
        container->baseType = PRIMITIVE_TYPE;
        container->primitive = PRIM_BOOLEAN;
    }
    else if (strcmp(baseTypeStr, "void") == 0)
    {
        container->baseType = PRIMITIVE_TYPE;
        container->primitive = PRIM_VOID;
    }
    else if (strcmp(baseTypeStr, "null") == 0)
    {
        container->baseType = PRIMITIVE_TYPE;
        container->primitive = PRIM_NULL;
    }
    else if (strcmp(baseTypeStr, "any") == 0)
    {
        logMessage(LMI, "INFO", "DataTypes", "Resolving `any` type!");
        container->baseType = PRIMITIVE_TYPE;
        container->primitive = PRIM_ANY;
    }
    else
    {
        logMessage(LMI, "INFO", "DataTypes", "Resolving custom type '%s'", typeStr);

        // No need for all the string copying - just pass typeStr directly
        DataType *resolvedType = ResolveDataType(globalTable, baseTypeStr);
        if (!resolvedType)
        {
            fprintf(stderr, "[DataTypes] Error: Failed to resolve data type '%s'\n", baseTypeStr);
            free(container);
            free(baseTypeStr);
            CONDITION_FAILED;
        }

        fprintf(stdout, "Resolved type: %s\n", resolvedType->container->custom.name);

        return resolvedType;
    }

    // Handle array types
    if (isArray)
    {
        logMessage(LMI, "INFO", "DataTypes", "Creating array type for '%s'", baseTypeStr);
        TypeContainer *arrayContainer = createArrayType(container, 1);
        free(container); // Free the base container since createArrayType makes a copy
        container = arrayContainer;
    }

    // Wrap in DataType
    DataType *dataType = wrapTypeContainer(container);

    return dataType;
}

DataType *parseGenericArrayType(const char *typeStr, CryoGlobalSymbolTable *globalTable)
{
    // Extract the base type name (without [])
    char *baseTypeName = strdup(typeStr);
    size_t len = strlen(baseTypeName);
    if (len >= 2 && baseTypeName[len - 2] == '[' && baseTypeName[len - 1] == ']')
    {
        baseTypeName[len - 2] = '\0';
    }

    // Resolve the base type
    DataType *baseType = ResolveDataType(globalTable, baseTypeName);
    if (!baseType)
    {
        // Handle error: base type not found
        return NULL;
    }

    // Create array type
    TypeContainer *container = createArrayType(baseType->container, 1);
    DataType *arrayType = wrapTypeContainer(container);

    free(baseTypeName);
    return arrayType;
}

// Create DataType wrapper
DataType *wrapTypeContainer(TypeContainer *container)
{
    __STACK_FRAME__
    DataType *type = (DataType *)malloc(sizeof(DataType));
    if (!type)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate DataType\n");
        CONDITION_FAILED;
    }

    type->container = container;
    type->isConst = false;
    type->isReference = false;
    type->next = NULL;
    type->genericParam = NULL;

    return type;
}

DataType *createPointerType(DataType *operandType)
{
    __STACK_FRAME__
    if (!operandType)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid operand type\n");
        CONDITION_FAILED;
    }

    TypeContainer *container = createTypeContainer();
    container->baseType = operandType->container->baseType;
    container->primitive = operandType->container->primitive;
    container->size = operandType->container->size;
    container->length = operandType->container->length;
    container->isArray = operandType->container->isArray;
    container->arrayDimensions = operandType->container->arrayDimensions;
    container->boolValue = operandType->container->boolValue;
    container->isGeneric = operandType->container->isGeneric;
    container->isConst = operandType->container->isConst;
    container->isReference = true;
    container->isPointer = true;

    DataType *type = wrapTypeContainer(container);
    type->next = operandType;

    return type;
}

ASTNode *findStructProperty(StructType *structType, const char *propertyName)
{
    __STACK_FRAME__
    if (!structType)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid struct type.\n");
        fprintf(stderr, "[TypeTable] Error: Failed to find property '%s' in struct.\n", propertyName);
        CONDITION_FAILED;
    }

    logMessage(LMI, "INFO", "DataTypes", "Finding property '%s' in struct '%s'", propertyName, structType->name);

    for (int i = 0; i < structType->propertyCount; i++)
    {
        ASTNode *property = structType->properties[i];
        if (strcmp(property->data.property->name, propertyName) == 0)
        {
            return property;
        }
    }

    fprintf(stderr, "[TypeTable] Error: Failed to find property '%s' in struct '%s'.\n", propertyName, structType->name);
    CONDITION_FAILED;
}

// # =========================================================================== #
// # Utility Functions

DataType *CryoDataTypeStringToType(const char *typeStr)
{
    __STACK_FRAME__
    if (strcmp(typeStr, "int") == 0)
    {
        return createPrimitiveIntType();
    }
    else if (strcmp(typeStr, "float") == 0)
    {
        return createPrimitiveFloatType();
    }
    else if (strcmp(typeStr, "string") == 0)
    {
        return createPrimitiveStringType(0);
    }
    else if (strcmp(typeStr, "boolean") == 0)
    {
        return createPrimitiveBooleanType(false);
    }
    else if (strcmp(typeStr, "void") == 0)
    {
        return createPrimitiveVoidType();
    }
    else if (strcmp(typeStr, "null") == 0)
    {
        return createPrimitiveNullType();
    }
    else
    {
        return createUnknownType();
    }
}

DataType *DataTypeFromNode(ASTNode *node)
{
    __STACK_FRAME__
    CryoNodeType nodeType = node->metaData->type;

    switch (nodeType)
    {
    case NODE_LITERAL_EXPR:
        logMessage(LMI, "INFO", "DataTypes", "Getting data type from literal expression");
        return node->data.literal->type;
    case NODE_VAR_NAME:
        logMessage(LMI, "INFO", "DataTypes", "Getting data type from var name");
        return node->data.varName->type;
    case NODE_STRUCT_DECLARATION:
        logMessage(LMI, "INFO", "DataTypes", "Getting data type from struct declaration");
        return node->data.structNode->type;
    case NODE_FUNCTION_DECLARATION:
        logMessage(LMI, "INFO", "DataTypes", "Getting data type from function declaration");
        return node->data.functionDecl->type;
    case NODE_PROPERTY:
        logMessage(LMI, "INFO", "DataTypes", "Getting data type from property");
        return node->data.property->type;
    case NODE_PARAM:
        logMessage(LMI, "INFO", "DataTypes", "Getting data type from param");
        return node->data.param->type;
    case NODE_VAR_DECLARATION:
        logMessage(LMI, "INFO", "DataTypes", "Getting data type from var declaration");
        return node->data.varDecl->type;
    case NODE_CLASS:
        logMessage(LMI, "INFO", "DataTypes", "Getting data type from class");
        return node->data.classNode->type;
    default:
    {
        fprintf(stderr, "[TypeTable] Error: Failed to get data type from node.\n");
        printf("Received node type: %s\n", CryoNodeTypeToString(nodeType));
        CONDITION_FAILED;
    }
    }

    CONDITION_FAILED;
}

const char *getDataTypeName(DataType *type)
{
    __STACK_FRAME__
    if (!type)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid data type. @getDataTypeName\n");
        CONDITION_FAILED;
    }

    if (type->container->baseType == PRIMITIVE_TYPE)
    {
        return PrimitiveDataTypeToString_UF(type->container->primitive);
    }
    else if (type->container->baseType == STRUCT_TYPE)
    {
        return type->container->custom.structDef->name;
    }
    else if (type->container->baseType == CLASS_TYPE)
    {
        return type->container->custom.classDef->name;
    }
    else if (type->container->baseType == FUNCTION_TYPE)
    {
        return type->container->custom.funcDef->name;
    }
    else
    {
        return "unknown";
    }
}

DataType **getTypeArrayFromASTNode(ASTNode **node, int size)
{
    __STACK_FRAME__
    if (!node)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid AST node.\n");
        CONDITION_FAILED;
    }

    DataType **types = (DataType **)malloc(sizeof(DataType *) * size);
    if (!types)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate memory for type array. size: %d\n", size);
        CONDITION_FAILED;
    }

    for (int i = 0; i < size; i++)
    {
        if (!node[i])
        {
            char *nodeType = CryoNodeTypeToString(node[i]->metaData->type);
            logMessage(LMI, "INFO", "DataTypes", "Node is NULL, breaking loop! Node type: %s", nodeType);
            break;
        }
        types[i] = DataTypeFromNode(node[i]);
        logMessage(LMI, "INFO", "DataTypes", "Got data type from AST node: %s", DataTypeToString(types[i]));
    }

    return types;
}

DataType **getTypeFromParamList(CryoVariableNode **params, int paramCount)
{
    __STACK_FRAME__
    if (!params)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid AST node.\n");
        CONDITION_FAILED;
    }

    DataType **types = (DataType **)malloc(sizeof(DataType *) * paramCount);
    if (!types)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate memory for type array. size: %d\n", paramCount);
        CONDITION_FAILED;
    }

    for (int i = 0; i < paramCount; i++)
    {
        if (!params[i])
        {
            logMessage(LMI, "INFO", "DataTypes", "Node is NULL, breaking loop!");
            break;
        }
        types[i] = params[i]->type;
        logMessage(LMI, "INFO", "DataTypes", "Got data type from AST node: %s", DataTypeToString(types[i]));
    }

    return types;
}

/// @brief This function should be used to get a DataType array from an ASTNode that is a **valid array of data types**.
/// This includes function parameters/arguments, array declarations, and other similar constructs.
/// This will return a `nullptr` if the node is not an array of data types and will not stop the program.
/// @param node
/// @return
DataType **getDataTypeArrayFromASTNode(ASTNode *node)
{
    __STACK_FRAME__
    if (!node)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid AST node.\n");
        return NULL;
    }

    switch (node->metaData->type)
    {
    case NODE_PARAM_LIST:
    {
        logMessage(LMI, "INFO", "DataTypes", "Getting data type array from param list");
        return getTypeFromParamList(node->data.paramList->params, node->data.paramList->paramCount);
    }
    case NODE_METHOD:
    {
        logMessage(LMI, "INFO", "DataTypes", "Getting data type array from method");
        return getTypeArrayFromASTNode(node->data.method->params, node->data.method->paramCount);
    }
    case NODE_FUNCTION_CALL:
    {
        logMessage(LMI, "INFO", "DataTypes", "Getting data type array from function call");
        return getTypeArrayFromASTNode(node->data.functionCall->args, node->data.functionCall->argCount);
    }
    case NODE_ARRAY_LITERAL:
    {
        logMessage(LMI, "INFO", "DataTypes", "Getting data type array from array declaration");
        return getTypeArrayFromASTNode(node->data.array->elements, node->data.array->elementCount);
    }
    default:
    {
        fprintf(stderr, "[TypeTable] Error: Failed to get data type array from AST node, received node type: %s\n",
                CryoNodeTypeToString(node->metaData->type));
        return NULL;
    }
    }

    return NULL;
}

DataType *getDataTypeFromASTNode(ASTNode *node)
{
    __STACK_FRAME__
    if (!node)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid AST node.\n");
        CONDITION_FAILED;
    }

    switch (node->metaData->type)
    {
    case NODE_LITERAL_EXPR:
        logMessage(LMI, "INFO", "DataTypes", "Getting data type from literal expression");
        return node->data.literal->type;
    case NODE_VAR_NAME:
        logMessage(LMI, "INFO", "DataTypes", "Getting data type from var name");
        return node->data.varName->type;
    case NODE_STRUCT_DECLARATION:
        logMessage(LMI, "INFO", "DataTypes", "Getting data type from struct declaration");
        return node->data.structNode->type;
    case NODE_FUNCTION_DECLARATION:
        logMessage(LMI, "INFO", "DataTypes", "Getting data type from function declaration");
        return node->data.functionDecl->type;
    case NODE_PROPERTY:
        logMessage(LMI, "INFO", "DataTypes", "Getting data type from property");
        return node->data.property->type;
    case NODE_PARAM:
        logMessage(LMI, "INFO", "DataTypes", "Getting data type from param");
        return node->data.param->type;
    case NODE_VAR_DECLARATION:
        logMessage(LMI, "INFO", "DataTypes", "Getting data type from var declaration");
        return node->data.varDecl->type;
    case NODE_METHOD:
        logMessage(LMI, "INFO", "DataTypes", "Getting data type from method");
        return node->data.method->type;
    case NODE_FUNCTION_CALL:
        logMessage(LMI, "INFO", "DataTypes", "Getting data type from function call");
        return node->data.functionCall->returnType;
    case NODE_ARRAY_LITERAL:
        logMessage(LMI, "INFO", "DataTypes", "Getting data type from array literal");
        return node->data.array->type;
    case NODE_INDEX_EXPR:
    {
        logMessage(LMI, "INFO", "DataTypes", "Getting data type from index expression");
        ASTNode *arr = node->data.indexExpr->array;
        return getDataTypeFromASTNode(arr);
    }
    case NODE_OBJECT_INST:
        logMessage(LMI, "INFO", "DataTypes", "Getting data type from object instantiation");
        return node->data.objectNode->objType;
    case NODE_PROPERTY_ACCESS:
    {
        logMessage(LMI, "INFO", "DataTypes", "Getting data type from property access");
        // Get the object type
        DataType *objectType = getDataTypeFromASTNode(node->data.propertyAccess->object);
        if (!objectType)
        {
            logMessage(LMI, "ERROR", "DataTypes", "Failed to get object type from property access.");
            CONDITION_FAILED;
        }
        // Get the property name
        const char *propertyName = node->data.propertyAccess->propertyName;

        // Get the struct type
        if (objectType->container->baseType == STRUCT_TYPE)
        {
            StructType *structType = objectType->container->custom.structDef;
            if (!structType)
            {
                fprintf(stderr, "[TypeTable] Error: Failed to get struct type from property access.\n");
                CONDITION_FAILED;
            }

            // Find the property in the struct
            for (int i = 0; i < structType->propertyCount; i++)
            {
                ASTNode *property = structType->properties[i];
                if (strcmp(property->data.property->name, propertyName) == 0)
                {
                    return property->data.property->type;
                }
            }

            fprintf(stderr, "[TypeTable] Error: Property '%s' not found in struct '%s'.\n", propertyName, structType->name);
            CONDITION_FAILED;
        }
        else
        {
            fprintf(stderr, "[TypeTable] Error: Property access on non-struct type.\n");
            CONDITION_FAILED;
        }
    }
    case NODE_BINARY_EXPR:
    {
        logMessage(LMI, "INFO", "DataTypes", "Getting data type from binary expression");
        ASTNode *lhs = node->data.bin_op->left;
        ASTNode *rhs = node->data.bin_op->right;
        DataType *lhsType = getDataTypeFromASTNode(lhs);
        DataType *rhsType = getDataTypeFromASTNode(rhs);
        // TODO: Implement type checking for binary expressions
        return lhsType;
    }
    default:
        logMessage(LMI, "ERROR", "DataTypes", "Failed to get data type from AST node, received node type: %s",
                   CryoNodeTypeToString(node->metaData->type));
        CONDITION_FAILED;
    }
}

void setNewDataTypeForNode(ASTNode *node, DataType *type)
{
    __STACK_FRAME__
    if (!node || !type)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid node or type.\n");
        return;
    }

    switch (node->metaData->type)
    {
    case NODE_LITERAL_EXPR:
        node->data.literal->type = type;
        break;
    case NODE_VAR_NAME:
        node->data.varName->type = type;
        break;
    case NODE_STRUCT_DECLARATION:
        node->data.structNode->type = type;
        break;
    case NODE_FUNCTION_DECLARATION:
        node->data.functionDecl->type = type;
        break;
    case NODE_PROPERTY:
        node->data.property->type = type;
        break;
    case NODE_PARAM:
        node->data.param->type = type;
        break;
    case NODE_VAR_DECLARATION:
        node->data.varDecl->type = type;
        break;
    default:
        fprintf(stderr, "[TypeTable] Error: Failed to set data type for node, received node type: %s\n",
                CryoNodeTypeToString(node->metaData->type));
        break;
    }
}

DataType *cloneDataType(DataType *type)
{
    __STACK_FRAME__
    if (!type)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid data type. @cloneDataType\n");
        CONDITION_FAILED;
    }

    TypeContainer *container = type->container;
    if (!container)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid type container.\n");
        CONDITION_FAILED;
    }

    TypeContainer *newContainer = createTypeContainer();
    if (!newContainer)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to create new type container.\n");
        CONDITION_FAILED;
    }

    newContainer->baseType = container->baseType;
    newContainer->primitive = container->primitive;
    newContainer->size = container->size;
    newContainer->length = container->length;
    newContainer->isArray = container->isArray;
    newContainer->arrayDimensions = container->arrayDimensions;
    newContainer->custom.name = container->custom.name;
    newContainer->custom.structDef = container->custom.structDef;
    newContainer->custom.funcDef = container->custom.funcDef;

    DataType *newType = wrapTypeContainer(newContainer);
    if (!newType)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to wrap new type container.\n");
        CONDITION_FAILED;
    }

    newType->isConst = type->isConst;
    newType->isReference = type->isReference;
    newType->next = type->next;
    newType->genericParam = type->genericParam;

    return newType;
}

DataType **getParamTypeArray(ASTNode **node)
{
    __STACK_FRAME__
    if (!node)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid node.\n");
        CONDITION_FAILED;
    }

    DataType **types = (DataType **)malloc(sizeof(DataType *) * 64);
    if (!types)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate memory for type array.\n");
        CONDITION_FAILED;
    }

    for (int i = 0; i < 64; i++)
    {
        printf("Viewing Node Type: %s\n", CryoNodeTypeToString(node[i]->metaData->type));
        if (!node[i])
        {
            logMessage(LMI, "INFO", "DataTypes", "Node is NULL, breaking loop");
            break;
        }
        types[i] = node[i]->data.param->type;
        logMessage(LMI, "INFO", "DataTypes", "Got data type from AST node");
    }

    logMessage(LMI, "INFO", "DataTypes", "Returning param type array");
    return types;
}

ASTNode **getAllClassMethods(ASTNode *classNode)
{
    __STACK_FRAME__
    if (!classNode)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid class node.\n");
        CONDITION_FAILED;
    }

    if (classNode->metaData->type != NODE_CLASS)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid class node type.\n");
        CONDITION_FAILED;
    }

    int memberCount = classNode->data.classNode->methodCount;
    logMessage(LMI, "INFO", "DataTypes", "Getting class members, count: %d", memberCount);
    ASTNode **members = (ASTNode **)malloc(sizeof(ASTNode *) * memberCount);
    if (!members)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate memory for class members.\n");
        CONDITION_FAILED;
    }

    PrivateMembers *privateMembers = classNode->data.classNode->privateMembers;
    PublicMembers *publicMembers = classNode->data.classNode->publicMembers;
    ProtectedMembers *protectedMembers = classNode->data.classNode->protectedMembers;

    int privateCount = 0;
    while (privateMembers->methods[privateCount] != NULL)
    {
        members[privateCount] = privateMembers->methods[privateCount];
        privateCount++;
    }

    int publicCount = 0;
    while (publicMembers->methods[publicCount] != NULL)
    {
        members[privateCount + publicCount] = publicMembers->methods[publicCount];
        publicCount++;
    }

    int protectedCount = 0;
    while (protectedMembers->methods[protectedCount] != NULL)
    {
        members[privateCount + publicCount + protectedCount] = protectedMembers->methods[protectedCount];
        protectedCount++;
    }

    logMessage(LMI, "INFO", "DataTypes", "Returning class members");
    return members;
}

ASTNode **getAllClassPropsFromDataType(DataType *classType)
{
    __STACK_FRAME__
    if (!classType)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid class type.\n");
        CONDITION_FAILED;
    }

    if (classType->container->baseType != CLASS_TYPE)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid class type.\n");
        CONDITION_FAILED;
    }

    ASTNode *classNode = classType->container->custom.classDef->classNode;
    if (!classNode)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to get class node from class type.\n");
        CONDITION_FAILED;
    }

    int memberCount = classNode->data.classNode->propertyCount;
    PrivateMembers *privateMembers = classNode->data.classNode->privateMembers;
    PublicMembers *publicMembers = classNode->data.classNode->publicMembers;
    ProtectedMembers *protectedMembers = classNode->data.classNode->protectedMembers;

    ASTNode **members = (ASTNode **)malloc(sizeof(ASTNode *) * memberCount);
    if (!members)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate memory for class members.\n");
        CONDITION_FAILED;
    }

    int privateCount = 0;
    while (privateMembers->properties[privateCount] != NULL)
    {
        members[privateCount] = privateMembers->properties[privateCount];
        privateCount++;
    }

    int publicCount = 0;
    while (publicMembers->properties[publicCount] != NULL)
    {
        members[privateCount + publicCount] = publicMembers->properties[publicCount];
        publicCount++;
    }

    int protectedCount = 0;
    while (protectedMembers->properties[protectedCount] != NULL)
    {
        members[privateCount + publicCount + protectedCount] = protectedMembers->properties[protectedCount];
        protectedCount++;
    }

    return members;
}
