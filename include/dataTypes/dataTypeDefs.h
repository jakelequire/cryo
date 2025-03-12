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

typedef struct DataType_t DataType;

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
    PRIM_TYPE,     // `int`, `float`, `string`, `boolean`, `void`
    ARRAY_TYPE,    // `int[]`, `float[]`, `string[]`, `boolean[]`
    ENUM_TYPE,     // `enum ... { ... }`
    FUNCTION_TYPE, // `function (...) -> ...`
    GENERIC_TYPE,  // `T`, `U`, `V`, etc.
    OBJECT_TYPE,   // `struct | class { ... }`
    TYPE_DEF,      // `type ... = ...`
    UNKNOWN_TYPE   // `<UNKNOWN>`
} TypeofDataType;

// =============================== Structs =============================== //

typedef struct DTDebug_t
{
    void (*printType)(struct DataType_t *type);
    void (*printVerbosType)(struct DataType_t *type);
    const char *(*typeToString)(struct DataType_t *type);
} DTDebug;

typedef struct DTSimpleTy_t
{
    PrimitiveDataType primitive;
    TypeofDataType baseType;
    bool isGeneric;
} DTSimpleTy;

typedef struct DTArrayTy_t
{
    int elementCount;
    int dimensions;
    struct DataType_t *elementType;
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
    struct DataType_t *returnType;
    struct DataType_t **paramTypes;
    int paramCount;
    int paramCapacity;

    void (*addParam)(struct DTFunctionTy_t *self, struct DataType_t *param);
    void (*setParams)(struct DTFunctionTy_t *self, struct DataType_t **params, int paramCount);
    void (*setReturnType)(struct DTFunctionTy_t *self, struct DataType_t *returnType);
} DTFunctionTy;

typedef struct DTStructTy_t
{
    const char *name;

    struct DataType_t **properties;
    int propertyCount;
    int propertyCapacity;

    struct DataType_t **methods;
    int methodCount;
    int methodCapacity;

    bool hasConstructor;
    struct DataType_t **ctorParams;
    int ctorParamCount;
    int ctorParamCapacity;

    int size;

    struct
    {
        bool isGeneric;
        struct GenericType_t **params;
        int paramCount;
        struct StructType_t *baseStruct;
        struct DataType_t **typeArgs;
    } generic;

    // Regular Struct Methods

    void (*addProperty)(struct DTStructTy_t *self, struct ASTNode_t *property);
    void (*addMethod)(struct DTStructTy_t *self, struct ASTNode_t *method);
    void (*addCtorParam)(struct DTStructTy_t *self, struct DataType_t *param);

    // Generic Struct Methods

    void (*addGenericParam)(struct DTStructTy_t *self, struct GenericType_t *param);
    struct StructType_t *(*substituteGenericType)(struct StructType_t *structDef, struct DataType_t *genericParam, struct DataType_t *concreteType);
    struct ASTNode_t *(*cloneAndSubstituteGenericMethod)(struct ASTNode_t *method, struct DataType_t *concreteType);
    struct ASTNode_t *(*cloneAndSubstituteGenericParam)(struct ASTNode_t *param, struct DataType_t *concreteType);

} DTStructTy;

typedef struct DTClassTy_t
{
    const char *name;

    DataType **properties;
    int propertyCount;
    int propertyCapacity;

    DataType **methods;
    int methodCount;
    int methodCapacity;

    bool hasConstructor;
    struct DataType_t **ctorParams;
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

    DataType **properties;
    int propertyCount;
    int propertyCapacity;

    DataType **methods;
    int methodCount;
    int methodCapacity;
} DTObjectType;

typedef struct TypeContainer_t
{
    PrimitiveDataType primitive;
    TypeofDataType baseType;
    union
    {
        DTSimpleTy *simpleType;
        DTArrayTy *arrayType;
        DTEnumTy *enumType;
        DTFunctionTy *functionType;
        DTStructTy *structType;
        DTClassTy *classType;
        DTObjectType *objectType;
    } type;
} TypeContainer;

typedef struct DataType_t
{
    TypeContainer *container;
    const char *typeName;
    bool isConst;
    bool isPointer;
    bool isReference;

    DTDebug *debug;

    // ============================

    void (*cast)(DataType *fromType, DataType *toType);
} DataType;

// =========================== Function Prototypes =========================== //

DTDebug *createDTDebug(void);

DTArrayTy *createDTArrayTy(void);
DTEnumTy *createDTEnumTy(void);
DTFunctionTy *createDTFunctionTy(void);
DTSimpleTy *createDTSimpleTy(void);
DTStructTy *createDTStructTy(void);
DTClassTy *createDTClassTy(void);
DTObjectType *createDTObjectType(void);
TypeContainer *createTypeContainer(void);

#endif // DATA_TYPE_DEFS_H
