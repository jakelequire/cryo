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
#ifndef TYPE_TABLE_H
#define TYPE_TABLE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>

#include "frontend/tokens.h"
#include "frontend/AST.h"
#include "settings/compilerSettings.h"
#include "common/common.h"

typedef struct CompilerState CompilerState;
typedef struct ASTNode ASTNode;
typedef struct Arena Arena;
typedef struct DataType DataType;

typedef enum PrimitiveDataType
{
    PRIM_INT,     // `int`
    PRIM_FLOAT,   // `float`
    PRIM_STRING,  // `string`
    PRIM_BOOLEAN, // `boolean`
    PRIM_VOID,    // `void`
    PRIM_NULL,    // `null`
    PRIM_CUSTOM,  // Custom type
    PRIM_UNKNOWN  // `<UNKNOWN>`
} PrimitiveDataType;

typedef enum TypeofDataType
{
    PRIMITIVE_TYPE, // `int`, `float`, `string`, `boolean`, `void`
    STRUCT_TYPE,    // `struct ... { ... }`
    ENUM_TYPE,      // `enum ... { ... }`
    FUNCTION_TYPE,  // `function (...) -> ...`
    GENERIC_TYPE,   // `T`, `U`, `V`, etc.

    UNKNOWN_TYPE // `<UNKNOWN>`
} TypeofDataType;

typedef struct GenericType
{
    const char *name;        // Name of the generic type (e.g., "T")
    DataType *constraint;    // Optional constraint on the generic type
    ASTNode **genericParams; // Array of generic parameter nodes
    int genericParamCount;
    int genericParamCapacity;
    struct
    {
        bool isArray;        // Whether this generic type is an array
        int arrayDimensions; // Number of array dimensions if isArray is true
    } arrayInfo;
    struct GenericType *next; // For linking multiple generic params (e.g., <T, U>)
} GenericType;

typedef struct StructType
{
    const char *name;
    ASTNode **properties;
    int propertyCount;
    int propertyCapacity;
    ASTNode **methods;
    int methodCount;
    int methodCapacity;
    bool hasDefaultValue;
    bool hasConstructor;
    int size;
} StructType;

typedef struct FunctionType
{
    const char *name;
    ASTNode **params;
    int paramCount;
    int paramCapacity;
    ASTNode *body;
    PrimitiveDataType returnType;
} FunctionType;

typedef struct EnumType
{
    const char *name;
    ASTNode **values;
    int valueCount;
    int valueCapacity;
} EnumType;

typedef struct TypeContainer
{
    TypeofDataType baseType;     // Base type (primitive, struct, etc)
    PrimitiveDataType primitive; // If primitive type
    int size;                    // Size of the type
    int length;                  // Length of the type
    bool isArray;                // Array flag
    int arrayDimensions;         // Number of array dimensions
    struct custom
    {
        const char *name;         // Type identifier name
        StructType *structDef;    // For struct types
        FunctionType *funcDef;    // For function types
        EnumType *enumDef;        // For enum types
        DataType **genericParams; // Generic type parameters
        int genericParamCount;    // Number of generic type parameters
    } custom;
} TypeContainer;

typedef struct DataType
{
    TypeContainer *container;      // Type container
    bool isConst;                  // Const modifier
    bool isReference;              // Reference type
    struct DataType *next;         // For linked types (e.g. generics)
    struct DataType *genericParam; // For generic type parameters
} DataType;

// This is the global symbol table specifically for types.
// This is how we will handle type checking and type inference.
typedef struct TypeTable
{
    DataType **types;
    int count;
    int capacity;
    char *namespaceName;
} TypeTable;

#ifdef __cplusplus
extern "C"
{
#endif
    // # =============================================================================================== #

    // # =========================================================================== #
    // # DataType Functions
    // # (datatypes.c)
    // # =========================================================================== #

    TypeTable *initTypeTable(void);
    TypeContainer *createTypeContainer(void);

    DataType *parseDataType(const char *typeStr, TypeTable *typeTable);
    DataType *wrapTypeContainer(TypeContainer *container);

    TypeContainer *lookupType(TypeTable *table, const char *name);
    void addTypeToTypeTable(TypeTable *table, const char *name, DataType *type);

    ASTNode *findStructProperty(StructType *structType, const char *propertyName);
    DataType *CryoDataTypeStringToType(const char *typeStr);
    DataType *DataTypeFromNode(ASTNode *node);
    const char *getDataTypeName(DataType *type);
    DataType *getDataTypeFromASTNode(ASTNode *node);
    void setNewDataTypeForNode(ASTNode *node, DataType *type);
    DataType *cloneDataType(DataType *type);

    // # =========================================================================== #
    // # Primitive Type Functions
    // # (primitives.c)
    // # =========================================================================== #

    DataType *createPrimitiveIntType(void);
    DataType *createPrimitiveFloatType(void);
    DataType *createPrimitiveStringType(void);
    DataType *createPrimitiveBooleanType(void);
    DataType *createPrimitiveVoidType(void);
    DataType *createPrimitiveNullType(void);
    DataType *createUnknownType(void);
    TypeContainer *createPrimitiveType(PrimitiveDataType primType);
    TypeContainer *createStructType(const char *name, StructType *structDef);
    TypeContainer *createArrayType(TypeContainer *baseType, int dimensions);

    PrimitiveDataType getPrimativeTypeFromString(const char *typeStr);
    bool isPrimitiveType(const char *typeStr);

    // # =========================================================================== #
    // # Struct Type Functions
    // # (structs.c)
    // # =========================================================================== #

    StructType *createStructTypeFromStructNode(ASTNode *structNode, CompilerState *state, TypeTable *typeTable);
    DataType *createDataTypeFromStructNode(
        ASTNode *structNode, ASTNode **properties, int propCount,
        ASTNode **methods, int methodCount,
        CompilerState *state, TypeTable *typeTable);

    int getPropertyAccessIndex(DataType *type, const char *propertyName);

    void addPropertiesToStruct(ASTNode **properties, int propCount, StructType *structType);
    void addMethodsToStruct(ASTNode **methods, int methodCount, StructType *structType);

    // # =========================================================================== #
    // # Array Type Functions
    // # (arrayTypes.c)
    // # =========================================================================== #

    TypeContainer *createArrayType(TypeContainer *baseType, int dimensions);

    // # =========================================================================== #
    // # Type Validation Functions
    // # (typeValidation.c)
    // # =========================================================================== #

    bool areTypesCompatible(TypeContainer *left, TypeContainer *right);
    bool isValidType(DataType *type);

    // # =========================================================================== #
    // # Generic Type Functions
    // # (generics.c)
    // # =========================================================================== #

    void initGenericType(GenericType *type, const char *name);
    GenericType *createGenericParameter(const char *name);
    TypeContainer *createGenericArrayType(DataType *genericParam);

    TypeContainer *createGenericStructInstance(TypeContainer *genericDef, DataType *concreteType);
    TypeContainer *createGenericInstance(StructType *baseStruct, DataType *concreteType);

    void addGenericConstraint(GenericType *type, DataType *constraint);
    void setGenericArrayInfo(GenericType *type, int dimensions);
    void linkGenericParameter(GenericType *base, GenericType *next);
    bool validateGenericType(GenericType *type, DataType *concrete_type);

    bool isGenericInstance(TypeContainer *type);
    bool isGenericType(TypeContainer *type);

    // # =========================================================================== #
    // # Print Functions
    // # (printFunctions.c)
    // # =========================================================================== #

    char *TypeofDataTypeToString(TypeofDataType type);
    char *PrimitiveDataTypeToString(PrimitiveDataType type);
    char *PrimitiveDataTypeToString_UF(PrimitiveDataType type);
    char *VerboseStructTypeToString(StructType *type);

    void printFormattedStructType(StructType *type);
    void printFormattedPrimitiveType(PrimitiveDataType type);

    void printFormattedType(DataType *type);
    void logDataType(DataType *type);
    void logStructType(StructType *type);
    void logVerboseDataType(DataType *type);

    void printTypeTable(TypeTable *table);
    void printTypeContainer(TypeContainer *type);
    void printVerboseTypeContainer(TypeContainer *type);

    char *DataTypeToString(DataType *dataType);
    char *DataTypeToStringUnformatted(DataType *type);

    bool typeAlreadyExists(TypeTable *table, const char *name);

    // # =============================================================================================== #

#define VALIDATE_TYPE(type)                                         \
    if (!isValidType(type))                                         \
    {                                                               \
        logMessage("ERROR", __LINE__, "TypeTable", "Invalid type"); \
        CONDITION_FAILED;                                           \
    }

#ifdef __cplusplus
}
#endif

#endif // TYPE_TABLE_H
