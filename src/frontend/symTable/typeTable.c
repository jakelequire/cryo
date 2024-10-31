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
#include "frontend/typeTable.h"

char *TypeofDataTypeToString(TypeofDataType type)
{
    switch (type)
    {
    case PRIMITIVE_TYPE:
        return "PRIMITIVE_TYPE";
    case STRUCT_TYPE:
        return "STRUCT_TYPE";
    case ENUM_TYPE:
        return "ENUM_TYPE";
    case FUNCTION_TYPE:
        return "FUNCTION_TYPE";
    case UNKNOWN_TYPE:
        return "UNKNOWN_TYPE";
    default:
        return "<TYPE UNKNOWN>";
    }
}

TypeTable *initTypeTable(void)
{
    TypeTable *table = (TypeTable *)malloc(sizeof(TypeTable));
    if (!table)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate memory for type table.\n");
        return NULL;
    }

    table->types = (DataType **)malloc(sizeof(DataType *) * 64);
    if (!table->types)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate memory for type table types.\n");
        return NULL;
    }

    table->count = 0;
    table->capacity = 64;

    return table;
}

StructType *createStructDataType(const char *name)
{
    StructType *type = (StructType *)malloc(sizeof(StructType));
    if (!type)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate memory for struct type.\n");
        return NULL;
    }

    type->name = name;
    type->properties = (ASTNode **)malloc(sizeof(ASTNode *) * 64);
    if (!type->properties)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate memory for struct type properties.\n");
        return NULL;
    }

    type->propertyCount = 0;
    type->propertyCapacity = 64;

    type->methods = (ASTNode **)malloc(sizeof(ASTNode *) * 64);
    if (!type->methods)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate memory for struct type methods.\n");
        return NULL;
    }

    type->methodCount = 0;
    type->methodCapacity = 64;

    type->hasDefaultValue = false;

    return type;
}

DataType *createDataTypeFromPrimitive(PrimitiveDataType type)
{
    DataType *dataType = (DataType *)malloc(sizeof(DataType));
    if (!dataType)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate memory for data type.\n");
        return NULL;
    }

    dataType->typeOf = PRIMITIVE_TYPE;
    dataType->primitiveType = type;

    return dataType;
}

DataType *createDataTypeFromStruct(Arena *arena, CompilerState *state, ASTNode *structNode)
{
    DataType *type = (DataType *)ARENA_ALLOC(arena, sizeof(DataType));
    if (!type)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate memory for data type.\n");
        return NULL;
    }

    type->typeOf = STRUCT_TYPE;

    const char *structName = structNode->data.structNode->name;
    StructType *structType = createStructDataType(structName);
    if (!structType)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to create struct type.\n");
        return NULL;
    }

    structType->properties = structNode->data.structNode->properties;
    structType->propertyCount = structNode->data.structNode->propertyCount;
    structType->propertyCapacity = structNode->data.structNode->propertyCapacity;

    structType->methods = structNode->data.structNode->methods;
    structType->methodCount = structNode->data.structNode->methodCount;
    structType->methodCapacity = structNode->data.structNode->methodCapacity;

    structType->hasDefaultValue = structNode->data.structNode->hasDefaultValue;
    structType->hasConstructor = structNode->data.structNode->hasConstructor;

    type->structType = structType;

    return type;
}

DataType *createDataTypeFromEnum(Arena *arena, CompilerState *state, ASTNode *enumNode)
{
    DataType *type = (DataType *)ARENA_ALLOC(arena, sizeof(DataType));
    if (!type)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate memory for data type.\n");
        return NULL;
    }

    type->typeOf = ENUM_TYPE;
    type->enumType = enumNode;

    return type;
}

DataType *createDataTypeFromFunction(Arena *arena, CompilerState *state, ASTNode *functionNode)
{
    DataType *type = (DataType *)ARENA_ALLOC(arena, sizeof(DataType));
    if (!type)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate memory for data type.\n");
        return NULL;
    }

    type->typeOf = FUNCTION_TYPE;
    type->functionType = functionNode;

    return type;
}

DataType *createDataTypeFromUnknown(void)
{
    DataType *type = (DataType *)malloc(sizeof(DataType));
    if (!type)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate memory for data type.\n");
        return NULL;
    }

    type->typeOf = UNKNOWN_TYPE;

    return type;
}

// # =========================================================================== #