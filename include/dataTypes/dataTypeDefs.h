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
#ifndef DATA_TYPE_DEFS_H
#define DATA_TYPE_DEFS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <ctype.h>

typedef enum PrimitiveDataType
{
    PRIM_INT,  // `int` (32-bit default)
    PRIM_I8,   // `i8`
    PRIM_I16,  // `i16`
    PRIM_I32,  // `i32`
    PRIM_I64,  // `i64`
    PRIM_I128, // `i128`

    PRIM_FLOAT,     // `float`
    PRIM_STRING,    // `string`
    PRIM_BOOLEAN,   // `boolean`
    PRIM_VOID,      // `void`
    PRIM_NULL,      // `null`
    PRIM_ANY,       // `any`
    PRIM_OBJECT,    // Object type
    PRIM_AUTO,      // `auto`
    PRIM_UNDEFINED, // `undefined`
    PRIM_UNKNOWN    // `<UNKNOWN>`
} PrimitiveDataType;

typedef enum TypeofDataType
{
    PRIMITIVE_TYPE, // `int`, `float`, `string`, `boolean`, `void`
    ARRAY_TYPE,     // `int[]`, `float[]`, `string[]`, `boolean[]`
    ENUM_TYPE,      // `enum ... { ... }`
    FUNCTION_TYPE,  // `function (...) -> ...`
    GENERIC_TYPE,   // `T`, `U`, `V`, etc.
    OBJECT_TYPE,    // `struct | class { ... }`
    TYPE_DEF,       // `type ... = ...`
    UNKNOWN_TYPE    // `<UNKNOWN>`
} TypeofDataType;

// =============================== Structs =============================== //

typedef struct DTDebug_t
{
    void (*print)(NEW_DataType *type);
    const char *(*toString)(NEW_DataType *type);
} DTDebug;

typedef struct DTArrayTy_t
{
    int elementCount;
    int dimensions;
    struct NEW_DataType_t *elementType;
} DTArrayTy;

typedef struct DTEnumTy_t
{
    const char *name;
    struct ASTNode_t **values;
    int valueCount;
    int valueCapacity;
} DTEnumTy;

typedef struct DTFunctionTy_t
{
    struct NEW_DataType_t *returnType;
    struct NEW_DataType_t **paramTypes;
    int paramCount;
    int paramCapacity;
} DTFunctionTy;

typedef struct DTSimpleTy_t
{
    PrimitiveDataType primitive;
    TypeofDataType baseType;
    bool isGeneric;
} DTSimpleTy;

typedef struct DTStructTy_t
{
    const char *name;

    struct NEW_DataType_t **properties;
    int propertyCount;
    int propertyCapacity;

    struct NEW_DataType_t **methods;
    int methodCount;
    int methodCapacity;

    bool hasConstructor;
    struct NEW_DataType_t **ctorParams;
    int ctorParamCount;
    int ctorParamCapacity;

    int size;

    struct
    {
        bool isGeneric;
        struct GenericType_t **params;
        int paramCount;
        struct StructType_t *baseStruct;
        struct NEW_DataType_t **typeArgs;
    } generic;

    // Regular Struct Methods

    void (*addProperty)(struct DTStructTy_t *self, struct ASTNode_t *property);
    void (*addMethod)(struct DTStructTy_t *self, struct ASTNode_t *method);
    void (*addCtorParam)(struct DTStructTy_t *self, struct NEW_DataType_t *param);

    // Generic Struct Methods

    void (*addGenericParam)(struct DTStructTy_t *self, struct GenericType_t *param);
    struct StructType_t *(*substituteGenericType)(struct StructType_t *structDef, struct NEW_DataType_t *genericParam, struct NEW_DataType_t *concreteType);
    struct ASTNode_t *(*cloneAndSubstituteGenericMethod)(struct ASTNode_t *method, struct NEW_DataType_t *concreteType);
    struct ASTNode_t *(*cloneAndSubstituteGenericParam)(struct ASTNode_t *param, struct NEW_DataType_t *concreteType);

} DTStructTy;

typedef struct DTClassTy_t
{
    const char *name;

    NEW_DataType **properties;
    int propertyCount;
    int propertyCapacity;

    NEW_DataType **methods;
    int methodCount;
    int methodCapacity;

    bool hasConstructor;
    struct NEW_DataType_t **ctorParams;
    int ctorParamCount;
    int ctorParamCapacity;

    struct PublicMembersTypes_t *publicMembers;
    struct PrivateMembersTypes_t *privateMembers;
    struct ProtectedMembersTypes_t *protectedMembers;

    // Regular Class Methods

    void (*addPublicProperty)(struct DTClassTy_t *self, struct ASTNode_t *property);
    void (*addPublicMethod)(struct DTClassTy_t *self, struct ASTNode_t *method);
    void (*addPrivateProperty)(struct DTClassTy_t *self, struct ASTNode_t *property);
    void (*addPrivateMethod)(struct DTClassTy_t *self, struct ASTNode_t *method);
    void (*addProtectedProperty)(struct DTClassTy_t *self, struct ASTNode_t *property);
    void (*addProtectedMethod)(struct DTClassTy_t *self, struct ASTNode_t *method);
} DTClassTy;

typedef struct DTObjectType_t
{
    const char *name;

    NEW_DataType **properties;
    int propertyCount;
    int propertyCapacity;

    NEW_DataType **methods;
    int methodCount;
    int methodCapacity;
} DTObjectType;

typedef struct TypeContainer_t
{
    PrimitiveDataType primitive;
    TypeofDataType baseType;
    union
    {
        DTSimpleTy      *simpleType;
        DTArrayTy       *arrayType;
        DTEnumTy        *enumType;
        DTFunctionTy    *functionType;
        DTStructTy      *structType;
        DTClassTy       *classType;
        DTObjectType    *objectType;
    } type;
} TypeContainer;

typedef struct NEW_DataType_t
{
    TypeContainer *typeContainer;
    const char *typeName;
    bool isConst;
    bool isPointer;
    bool isReference;

    DTDebug *debug;

    // ============================

    void (*cast)(NEW_DataType *type, NEW_DataType *toType);
} NEW_DataType;

// =========================== Function Prototypes =========================== //

DTDebug *createDTDebug(void);

#endif // DATA_TYPE_DEFS_H
