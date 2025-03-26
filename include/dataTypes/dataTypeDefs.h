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
    PRIM_FUNCTION,  // Function type
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

typedef enum TypeofObjectType
{
    STRUCT_OBJ, // `struct { ... }`
    CLASS_OBJ,  // `class { ... }`
    INTERFACE_OBJ,
    OBJECT_OBJ,
    NON_OBJECT,
    UNKNOWN_OBJECT
} TypeofObjectType;

// =============================== Structs =============================== //

typedef struct DTDebug_t
{
    void (*printType)(struct DataType_t *type);
    void (*printVerbosType)(struct DataType_t *type);
    const char *(*toString)(struct DataType_t *type);
} DTDebug;

// ------------------------------------------------------------------------------------------- //
// ------------------------------------ Simple Data Type ------------------------------------- //
// ------------------------------------------------------------------------------------------- //

typedef struct DTSimpleTy_t
{
    // ==================== [ Property Assignments ] ==================== //

    PrimitiveDataType primitive;
    TypeofDataType baseType;
} DTSimpleTy;

// ------------------------------------------------------------------------------------------- //
// ------------------------------------ Array Data Type -------------------------------------- //
// ------------------------------------------------------------------------------------------- //

typedef struct DTArrayTy_t
{
    // ==================== [ Property Assignments ] ==================== //

    struct DataType_t **elements;
    int elementCount;
    int elementCapacity;
    int dimensions;
    struct DataType_t *baseType;

    // ==================== [ Function Assignments ] ==================== //

    void (*addElement)(struct DTArrayTy_t *self, struct DataType_t *element);
    void (*removeElement)(struct DTArrayTy_t *self, struct DataType_t *element);
    void (*resize)(struct DTArrayTy_t *self);
    void (*reset)(struct DTArrayTy_t *self);
    void (*free)(struct DTArrayTy_t *self);
    void (*freeData)(struct DTArrayTy_t *self);
    void (*printArray)(struct DTArrayTy_t *self);
} DTArrayTy;

// ------------------------------------------------------------------------------------------- //
// ------------------------------------ Enum Data Type --------------------------------------- //
// ------------------------------------------------------------------------------------------- //

typedef struct DTEnumTy_t
{
    // ==================== [ Property Assignments ] ==================== //

    const char *name;
    struct ASTNode **values;
    int valueCount;
    int valueCapacity;

    // ==================== [ Function Assignments ] ==================== //

    void (*addValue)(struct DTEnumTy_t *self, struct ASTNode *value);
    void (*setValues)(struct DTEnumTy_t *self, struct ASTNode **values, int valueCount);

} DTEnumTy;

// ------------------------------------------------------------------------------------------- //
// ----------------------------------- Function Data Type ------------------------------------ //
// ------------------------------------------------------------------------------------------- //

typedef struct DTFunctionTy_t
{
    // ==================== [ Property Assignments ] ==================== //

    struct DataType_t *returnType;
    struct DataType_t **paramTypes;
    int paramCount;
    int paramCapacity;

    // ==================== [ Function Assignments ] ==================== //

    void (*addParam)(struct DTFunctionTy_t *self, struct DataType_t *param);
    void (*setParams)(struct DTFunctionTy_t *self, struct DataType_t **params, int paramCount);
    void (*setReturnType)(struct DTFunctionTy_t *self, struct DataType_t *returnType);
    const char *(*signatureToString)(struct DTFunctionTy_t *self);
} DTFunctionTy;

// ------------------------------------------------------------------------------------------- //
// ----------------------------------- Property Data Type ------------------------------------ //
// ------------------------------------------------------------------------------------------- //

typedef struct DTPropertyTy_t
{
    // ==================== [ Property Assignments ] ==================== //

    const char *name;
    DataType *type;
    ASTNode *node;

    bool isStatic;
    bool isConst;
    bool isPublic;
    bool isPrivate;
    bool isProtected;

    // ==================== [ Function Assignments ] ==================== //

    void (*setStatic)(struct DTPropertyTy_t *self, bool isStatic);
    void (*setConst)(struct DTPropertyTy_t *self, bool isConst);
    void (*setPublic)(struct DTPropertyTy_t *self, bool isPublic);
    void (*setPrivate)(struct DTPropertyTy_t *self, bool isPrivate);
    void (*setProtected)(struct DTPropertyTy_t *self, bool isProtected);
    void (*setType)(struct DTPropertyTy_t *self, DataType *type);
    void (*setName)(struct DTPropertyTy_t *self, const char *name);

} DTPropertyTy;

// ------------------------------------------------------------------------------------------- //
// ------------------------------------ Struct Data Type ------------------------------------- //
// ------------------------------------------------------------------------------------------- //

typedef struct DTStructTy_t
{
    // ==================== [ Property Assignments ] ==================== //

    const char *name;

    struct DTPropertyTy_t **properties;
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
        struct DTGenericTy_t **params;
        int paramCount;
        int paramCapacity;
        struct StructType_t *baseStruct;
        struct DataType_t **typeArgs;
    } generic;

    // ==================== [ Function Assignments ] ==================== //

    void (*addProperty)(struct DTStructTy_t *self, DTPropertyTy *property);
    void (*addMethod)(struct DTStructTy_t *self, struct DataType_t *method);
    void (*addCtorParam)(struct DTStructTy_t *self, struct DataType_t *param);

    // Generic Struct Methods

    void (*addGenericParam)(struct DTStructTy_t *self, struct DTGenericTy_t *param);
    struct DTStructTy_t *(*substituteGenericType)(struct DTStructTy_t *structDef, struct DTGenericTy_t *genericParam, struct DataType_t *concreteType);
    struct DTStructTy_t *(*cloneAndSubstituteGenericMethod)(struct DTStructTy_t *structType, struct DTGenericTy_t *DTGenericType, struct DataType_t *substituteType);
    struct DTStructTy_t *(*cloneAndSubstituteGenericParam)(struct DTStructTy_t *structType, struct DTGenericTy_t *DTGenericType, struct DataType_t *substituteType);

} DTStructTy;

// ------------------------------------------------------------------------------------------- //
// ------------------------------------- Class Data Type ------------------------------------- //
// ------------------------------------------------------------------------------------------- //

typedef struct DTPublicMembersTypes_t
{
    DTPropertyTy **properties;
    int propertyCount;
    int propertyCapacity;

    DataType **methods;
    int methodCount;
    int methodCapacity;
} DTPublicMembersTypes;

typedef struct DTPrivateMembersTypes_t
{
    DTPropertyTy **properties;
    int propertyCount;
    int propertyCapacity;

    DataType **methods;
    int methodCount;
    int methodCapacity;
} DTPrivateMembersTypes;

typedef struct DTProtectedMembersTypes_t
{
    DTPropertyTy **properties;
    int propertyCount;
    int propertyCapacity;

    DataType **methods;
    int methodCount;
    int methodCapacity;
} DTProtectedMembersTypes;

typedef struct DTClassTy_t
{
    const char *name;

    DTPropertyTy **properties;
    int propertyCount;
    int propertyCapacity;

    DataType **methods;
    int methodCount;
    int methodCapacity;

    bool hasConstructor;
    struct DataType_t **ctorParams;
    int ctorParamCount;
    int ctorParamCapacity;

    struct DTPublicMembersTypes_t *publicMembers;
    struct DTPrivateMembersTypes_t *privateMembers;
    struct DTProtectedMembersTypes_t *protectedMembers;

    // ==================== [ Function Assignments ] ==================== //

    void (*addPublicProperty)(struct DTClassTy_t *self, DTPropertyTy *property);
    void (*addPublicProperties)(struct DTClassTy_t *self, DTPropertyTy **properties, int propertyCount);
    void (*addPublicMethod)(struct DTClassTy_t *self, struct DataType_t *method);
    void (*addPublicMethods)(struct DTClassTy_t *self, struct DataType_t **methods, int methodCount);

    void (*addPrivateProperty)(struct DTClassTy_t *self, DTPropertyTy *property);
    void (*addPrivateProperties)(struct DTClassTy_t *self, DTPropertyTy **properties, int propertyCount);
    void (*addPrivateMethod)(struct DTClassTy_t *self, struct DataType_t *method);
    void (*addPrivateMethods)(struct DTClassTy_t *self, struct DataType_t **methods, int methodCount);

    void (*addProtectedProperty)(struct DTClassTy_t *self, DTPropertyTy *property);
    void (*addProtectedProperties)(struct DTClassTy_t *self, DTPropertyTy **properties, int propertyCount);
    void (*addProtectedMethod)(struct DTClassTy_t *self, struct DataType_t *method);
    void (*addProtectedMethods)(struct DTClassTy_t *self, struct DataType_t **methods, int methodCount);

} DTClassTy;

// ------------------------------------------------------------------------------------------- //
// ------------------------------------ Object Data Type ------------------------------------- //
// ------------------------------------------------------------------------------------------- //

typedef struct DTObjectTy_t
{
    const char *name;

    DataType **properties;
    int propertyCount;
    int propertyCapacity;

    DataType **methods;
    int methodCount;
    int methodCapacity;

    // ==================== [ Function Assignments ] ==================== //

    void (*addProperty)(struct DTObjectTy_t *self, struct DataType_t *property);
    void (*addMethod)(struct DTObjectTy_t *self, struct DataType_t *method);

    void (*_resizeProperties)(struct DTObjectTy_t *self); // Private method, do not use outside DTM internals
    void (*_resizeMethods)(struct DTObjectTy_t *self);    // Private method, do not use outside DTM internals

} DTObjectTy;

// ------------------------------------------------------------------------------------------- //
// ------------------------------------ Generic Data Type ------------------------------------ //
// ------------------------------------------------------------------------------------------- //

typedef struct DTGenericTy_t
{
    const char *name;
    DataType *constraint;
    int dimensions;
    int paramCount;
    struct GenericType_t *next;
} DTGenericTy;

// ------------------------------------------------------------------------------------------- //
// ------------------------------------- Type Container -------------------------------------- //
// ------------------------------------------------------------------------------------------- //

typedef struct TypeContainer_t
{
    PrimitiveDataType primitive;
    TypeofDataType typeOf;
    TypeofObjectType objectType;
    union
    {
        struct DTSimpleTy_t *simpleType;
        struct DTArrayTy_t *arrayType;
        struct DTEnumTy_t *enumType;
        struct DTFunctionTy_t *functionType;
        struct DTStructTy_t *structType;
        struct DTClassTy_t *classType;
        struct DTObjectTy_t *objectType;
        struct DTGenericTy_t *genericType;
        struct DTPropertyTy_t *propertyType;
    } type;
} TypeContainer;

// ------------------------------------------------------------------------------------------- //
// ---------------------------------------- Data Type ---------------------------------------- //
// ------------------------------------------------------------------------------------------- //

typedef struct DataType_t
{
    TypeContainer *container;
    const char *typeName;
    bool isConst;
    bool isPointer;
    bool isReference;
    bool isArray;

    DTDebug *debug; // This is the debug Object for development purposes

    // ==================== [ Function Assignments ] ==================== //

    void (*cast)(DataType *fromType, DataType *toType);
    void (*setConst)(DataType *type, bool isConst);
    void (*setPointer)(DataType *type, bool isPointer);
    void (*setReference)(DataType *type, bool isReference);
    void (*setTypeName)(DataType *type, const char *name);
    void (*free)(DataType *type);
} DataType;

// ----------------------------------- Function Prototypes ----------------------------------- //

DTDebug *createDTDebug(void);

DTArrayTy *createDTArrayTy(void);
DTEnumTy *createDTEnumTy(void);
DTFunctionTy *createDTFunctionTy(void);
DTSimpleTy *createDTSimpleTy(void);
DTStructTy *createDTStructTy(void);
DTClassTy *createDTClassTy(void);
DTObjectTy *createDTObjectType(void);
DTPropertyTy *createDTProperty(void);

TypeContainer *createTypeContainer(void);

#endif // DATA_TYPE_DEFS_H
