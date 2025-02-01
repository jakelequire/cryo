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
#include "settings/compilerSettings.h"
#include "common/common.h"
#include "frontend/AST.h"

// Define each token mapping
#define DECLARE_TOKEN(str, type) {str, type}

// Define the data types table with X-Macros
#define DATA_TYPE_TABLE                      \
    X("int", TOKEN_TYPE_INT)                 \
    X("string", TOKEN_TYPE_STRING)           \
    X("boolean", TOKEN_TYPE_BOOLEAN)         \
    X("void", TOKEN_TYPE_VOID)               \
    X("int[]", TOKEN_TYPE_INT_ARRAY)         \
    X("string[]", TOKEN_TYPE_STRING_ARRAY)   \
    X("boolean[]", TOKEN_TYPE_BOOLEAN_ARRAY) \
    X("i8", TOKEN_TYPE_I8)                   \
    X("i16", TOKEN_TYPE_I16)                 \
    X("i32", TOKEN_TYPE_I32)                 \
    X("i64", TOKEN_TYPE_I64)                 \
    X("i128", TOKEN_TYPE_I128)

typedef struct CompilerState CompilerState;
typedef struct ASTNode ASTNode;
typedef struct Arena Arena;
typedef struct DataType DataType;
typedef struct TypeContainer TypeContainer;
typedef struct ClassNode ClassNode;
typedef struct CryoVariableNode CryoVariableNode;

typedef struct PublicMembers PublicMembers;
typedef struct PrivateMembers PrivateMembers;
typedef struct ProtectedMembers ProtectedMembers;

typedef struct CryoGlobalSymbolTable_t *CryoGlobalSymbolTable;

typedef enum PrimitiveDataType
{
    PRIM_INT,  // `int` (32-bit default)
    PRIM_I8,   // `i8`
    PRIM_I16,  // `i16`
    PRIM_I32,  // `i32`
    PRIM_I64,  // `i64`
    PRIM_I128, // `i128`

    PRIM_FLOAT,   // `float`
    PRIM_STRING,  // `string`
    PRIM_BOOLEAN, // `boolean`
    PRIM_VOID,    // `void`
    PRIM_NULL,    // `null`
    PRIM_ANY,     // `any`
    PRIM_CUSTOM,  // Custom type
    PRIM_UNKNOWN  // `<UNKNOWN>`
} PrimitiveDataType;

typedef enum TypeofDataType
{
    PRIMITIVE_TYPE, // `int`, `float`, `string`, `boolean`, `void`
    ARRAY_TYPE,     // `int[]`, `float[]`, `string[]`, `boolean[]`
    STRUCT_TYPE,    // `struct ... { ... }`
    ENUM_TYPE,      // `enum ... { ... }`
    FUNCTION_TYPE,  // `function (...) -> ...`
    GENERIC_TYPE,   // `T`, `U`, `V`, etc.
    CLASS_TYPE,     // `class ... { ... }`

    UNKNOWN_TYPE // `<UNKNOWN>`
} TypeofDataType;

typedef struct GenericType
{
    const char *name;     // Name of the generic type (e.g., "T")
    DataType *constraint; // Optional constraint on the generic type
    bool isType;          // Whether this is a type parameter (T) or a concrete type (int)
    union
    {
        struct
        {                            // For type parameters (when isType is false)
            ASTNode **genericParams; // Array of generic parameter nodes
            int genericParamCount;
            int genericParamCapacity;
            struct
            {
                bool isArray;
                int arrayDimensions;
            } arrayInfo;
        } parameter;

        struct
        {                           // For concrete types (when isType is true)
            DataType *concreteType; // The actual type used in instantiation
        } concrete;
    };
    struct GenericType *next; // For linking multiple generic params (e.g., <T, U>)
} GenericType;

typedef struct StructType
{
    const char *name;

    DataType **ctorParams;
    int ctorParamCount;
    int ctorParamCapacity;

    ASTNode **properties;
    int propertyCount;
    int propertyCapacity;

    ASTNode **methods;
    int methodCount;
    int methodCapacity;

    bool hasDefaultValue;
    bool hasConstructor;

    int size;

    // New fields for generic support
    struct
    {
        bool isGeneric;         // Whether this is a generic struct
        GenericType **params;   // Array of generic type parameters
        int paramCount;         // Number of generic parameters
        StructType *baseStruct; // Original generic struct (for instantiations)
        DataType **typeArgs;    // Concrete type arguments (for instantiations)
    } generic;

} StructType;

typedef struct FunctionType
{
    const char *name;
    DataType **paramTypes;
    int paramCount;
    int paramCapacity;
    ASTNode *body;
    DataType *returnType;
} FunctionType;

typedef struct ArrayType
{
    DataType *baseType;
    int dimensions;
    int length;
    DataType **elements;
    int elementCount;
} ArrayType;

typedef struct EnumType
{
    const char *name;
    ASTNode **values;
    int valueCount;
    int valueCapacity;
} EnumType;

typedef struct GenericDeclType
{
    StructType *genericDef; // The generic type definition
    DataType **params;      // Generic parameters
    int paramCount;
} GenericDeclType;

typedef struct GenericInstType
{
    StructType *structDef; // The concrete struct definition
    DataType **typeArgs;   // The concrete type arguments
    int argCount;
    TypeContainer *baseDef; // Reference to the generic declaration
} GenericInstType;

typedef struct
{
    DataType *parentType;

    DataType **properties;
    int propertyCount;
    int propertyCapacity;

    DataType **methods;
    int methodCount;
    int methodCapacity;
} PublicMembersTypes, PrivateMembersTypes, ProtectedMembersTypes;

typedef struct ClassType
{
    const char *name;
    ASTNode *constructor;
    ASTNode *classNode;
    int propertyCount;
    int propertyCapacity;
    int methodCount;
    int methodCapacity;
    bool hasConstructor;
    bool isStatic;

    PublicMembersTypes *publicMembers;
    PrivateMembersTypes *privateMembers;
    ProtectedMembersTypes *protectedMembers;
} ClassType;

typedef struct TypeContainer
{
    TypeofDataType baseType;     // Base type (primitive, struct, etc)
    PrimitiveDataType primitive; // All types have a primitive type (PRIM_NT, PRIM_CUSTOM, etc)
    int size;                    // Size of the type
    int length;                  // Length of the type
    bool isArray;                // Array flag
    int arrayDimensions;         // Number of array dimensions
    bool boolValue;              // Boolean value
    bool isGeneric;              // Generic type flag
    bool isConst;                // Const modifier
    struct custom
    {
        const char *name; // Type identifier name

        union generic // For generic types
        {
            GenericDeclType *declaration;   // For generic declarations
            GenericInstType *instantiation; // For generic instantiations
        } generic;

        StructType *structDef; // For struct types
        FunctionType *funcDef; // For function types
        ArrayType *arrayDef;   // For array types
        EnumType *enumDef;     // For enum types
        ClassType *classDef;   // For class types
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

// Helper macros for type checking
#define IS_GENERIC_DECLARATION(type) \
    ((type)->baseType == GENERIC_TYPE && (type)->custom.generic.declaration.genericDef != NULL)

#define IS_GENERIC_INSTANTIATION(type) \
    ((type)->baseType == STRUCT_TYPE && (type)->custom.generic.instantiation.baseDef != NULL)

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

    DataType *parseDataType(const char *typeStr, TypeTable *typeTable, CryoGlobalSymbolTable *globalTable);
    DataType *wrapTypeContainer(TypeContainer *container);

    DataType *lookupType(TypeTable *table, const char *name);
    void addTypeToTypeTable(TypeTable *table, const char *name, DataType *type);

    ASTNode *findStructProperty(StructType *structType, const char *propertyName, TypeTable *typeTable);
    DataType *CryoDataTypeStringToType(const char *typeStr);
    DataType *DataTypeFromNode(ASTNode *node);
    const char *getDataTypeName(DataType *type);

    DataType **getDataTypeArrayFromASTNode(ASTNode *node);
    DataType **getTypeFromParamList(CryoVariableNode **params, int paramCount);
    DataType **getTypeArrayFromASTNode(ASTNode **node, int size);
    DataType **getParamTypeArray(ASTNode **node);

    DataType *getDataTypeFromASTNode(ASTNode *node);
    void setNewDataTypeForNode(ASTNode *node, DataType *type);
    DataType *cloneDataType(DataType *type);

    void updateTypeInTypeTable(TypeTable *table, const char *name, DataType *type);
    void importTypesFromRootNode(TypeTable *typeTable, ASTNode *root);

    DataType *findClassType(ASTNode *node, TypeTable *typeTable);
    DataType *findClassTypeFromName(const char *name, TypeTable *typeTable);

    ASTNode **getAllClassMethods(ASTNode *classNode);
    ASTNode **getAllClassPropsFromDataType(DataType *classType);

    // # =========================================================================== #
    // # Primitive Type Functions
    // # (primitives.c)
    // # =========================================================================== #

    DataType *createPrimitiveIntType(void);
    DataType *createPrimitiveFloatType(void);
    DataType *createPrimitiveStringType(int length);
    DataType *createPrimitiveBooleanType(bool booleanValue);
    DataType *createPrimitiveVoidType(void);
    DataType *createPrimitiveNullType(void);
    DataType *createPrimitiveAnyType(void);
    DataType *createUnknownType(void);

    TypeContainer *createPrimitiveType(PrimitiveDataType primType);
    TypeContainer *createStructType(const char *name, StructType *structDef);
    TypeContainer *createArrayType(TypeContainer *baseType, int dimensions);

    PrimitiveDataType getPrimativeTypeFromString(const char *typeStr);
    bool isPrimitiveType(const char *typeStr);
    bool isStringType(DataType *type);

    // # =========================================================================== #
    // # Struct Type Functions
    // # (structs.c)
    // # =========================================================================== #

    DataType *createStructDefinition(const char *structName);
    StructType *createStructTypeFromStructNode(ASTNode *structNode, CompilerState *state, TypeTable *typeTable);
    DataType *createDataTypeFromStructNode(
        ASTNode *structNode, ASTNode **properties, int propCount,
        ASTNode **methods, int methodCount,
        CompilerState *state, TypeTable *typeTable);

    int getPropertyAccessIndex(DataType *type, const char *propertyName);

    void addPropertiesToStruct(ASTNode **properties, int propCount, StructType *structType);
    void addMethodsToStruct(ASTNode **methods, int methodCount, StructType *structType);
    int calculateStructSize(StructType *structType);

    DataType *wrapStructType(StructType *structDef);
    bool isStructDeclaration(TypeTable *table, const char *name);
    bool isStructType(DataType *type);

    // # =========================================================================== #
    // # Array Type Functions
    // # (arrayTypes.c)
    // # =========================================================================== #

    TypeContainer *createArrayType(TypeContainer *baseType, int dimensions);
    ArrayType *createArrayTypeContainer(DataType *baseType, DataType **elementTypes, int length, int dimensions);
    DataType *wrapArrayType(ArrayType *arrayType);

    // # =========================================================================== #
    // # Type Validation Functions
    // # (typeValidation.c)
    // # =========================================================================== #

    bool areTypesCompatible(TypeContainer *left, TypeContainer *right);
    bool isValidType(DataType *type);
    bool isSameType(DataType *left, DataType *right);

    bool isNumericDataType(DataType *type);
    bool isStringDataType(DataType *type);

    bool binOpEligible(DataType *lhs, DataType *rhs);

    // # =========================================================================== #
    // # Generic Type Functions
    // # (generics.c)
    // # =========================================================================== #

    TypeContainer *createGenericTypeContainer(void);

    void initGenericType(GenericType *type, const char *name);
    GenericType *createGenericParameter(const char *name);
    TypeContainer *createGenericArrayType(DataType *genericParam);
    GenericDeclType *createGenericDeclarationContainer(StructType *structDef, DataType **genericParam, int paramCount);
    GenericInstType *createGenericInstanceContainer(StructType *structDef, DataType **typeArgs, int argCount, TypeContainer *baseDef);

    TypeContainer *createGenericStructInstance(TypeContainer *genericDef, DataType *concreteType);
    TypeContainer *createGenericInstance(StructType *baseStruct, DataType *concreteType);

    void addGenericConstraint(GenericType *type, DataType *constraint);
    void setGenericArrayInfo(GenericType *type, int dimensions);
    void linkGenericParameter(GenericType *base, GenericType *next);

    bool isGenericInstance(TypeContainer *type);
    bool isGenericType(DataType *type);
    bool validateGenericType(DataType *type, DataType *concreteType);
    bool isTypeCompatible(DataType *type, DataType *other);

    const char *getGenericTypeName(DataType *type);
    int getGenericParameterCount(TypeContainer *type);

    StructType *substituteGenericType(StructType *structDef, DataType *genericParam, DataType *concreteType);
    ASTNode *cloneAndSubstituteGenericMethod(ASTNode *method, DataType *concreteType);
    ASTNode *cloneAndSubstituteGenericParam(ASTNode *param, DataType *concreteType);
    ASTNode *cloneAndSubstituteGenericBody(ASTNode *body, DataType *concreteType);
    ASTNode *cloneAndSubstituteGenericStatement(ASTNode *statement, DataType *concreteType);

    void addGenericTypeParam(TypeContainer *container, DataType *param);
    DataType *createGenericDataTypeInstance(DataType *genericType, DataType **concreteTypes, int paramCount);

    // # =========================================================================== #
    // # Class Type Functions
    // # (classTypes.c)
    // # =========================================================================== #

    ClassType *createClassType(const char *name, ClassNode *classNode);
    TypeContainer *wrapClassInTypeContainer(ClassType *classType);
    DataType *createClassDataType(const char *name, ClassNode *classNode);

    void addPublicPropertyToClassType(ClassType *classType, ASTNode *property);
    void addPublicMethodToClassType(ClassType *classType, ASTNode *method);

    void addPrivatePropertyToClassType(ClassType *classType, ASTNode *property);
    void addPrivateMethodToClassType(ClassType *classType, ASTNode *method);

    void addProtectedPropertyToClassType(ClassType *classType, ASTNode *property);
    void addProtectedMethodToClassType(ClassType *classType, ASTNode *method);

    PublicMembersTypes *createPublicMembersType(void);
    PrivateMembersTypes *createPrivateMembersType(void);
    ProtectedMembersTypes *createProtectedMembersType(void);

    void linkPublicMemebers(PublicMembersTypes *membersType, PublicMembers *membersNode);
    void linkPrivateMemebers(PrivateMembersTypes *membersType, PrivateMembers *membersNode);
    void linkProtectedMemebers(ProtectedMembersTypes *membersType, ProtectedMembers *membersNode);

    // # =========================================================================== #
    // # Function Types
    // # (functionTypes.c)
    // # =========================================================================== #

    FunctionType *createFunctionTypeContainer(void);
    DataType *createMethodType(const char *methodName, DataType *returnType, DataType **paramTypes, int paramCount,
                               Arena *arena, CompilerState *state, TypeTable *typeTable);
    DataType *createFunctionType(const char *functionName, DataType *returnType, DataType **paramTypes, int paramCount,
                                 Arena *arena, CompilerState *state, TypeTable *typeTable);

    // # =========================================================================== #
    // # Print Functions
    // # (printFunctions.c)
    // # =========================================================================== #

    char *TypeofDataTypeToString(TypeofDataType type);
    char *PrimitiveDataTypeToString(PrimitiveDataType type);
    char *PrimitiveDataTypeToString_UF(PrimitiveDataType type);
    char *VerboseStructTypeToString(StructType *type);
    char *VerboseClassTypeToString(ClassType *type);
    char *VerboseFunctionTypeToString(FunctionType *type);
    char *VerboseDataTypeToString(DataType *dataType);

    void printFormattedStructType(StructType *type);
    void printFormattedPrimitiveType(PrimitiveDataType type);

    void printFormattedType(DataType *type);
    void logDataType(DataType *type);
    void logStructType(StructType *type);
    void logVerboseDataType(DataType *type);
    void printClassType(ClassType *type);
    void printFunctionType(FunctionType *funcType);
    void printFunctionType_UF(FunctionType *funcType);
    char *getFunctionTypeStr_UF(FunctionType *funcType);
    char *getFunctionArgTypeArrayStr(ASTNode *functionNode);

    void printTypeTable(TypeTable *table);
    void printTypeContainer(TypeContainer *type);
    void printVerboseTypeContainer(TypeContainer *type);

    char *DataTypeToString(DataType *dataType);
    char *DataTypeToStringUnformatted(DataType *type);

    bool typeAlreadyExists(TypeTable *table, const char *name);

    // # =============================================================================================== #

#define VALIDATE_TYPE(type)                                       \
    if (!isValidType(type))                                       \
    {                                                             \
        logMessage(LMI, "ERROR", "TypeTable", "Invalid type: %s", \
                   DataTypeToString(type));                       \
        CONDITION_FAILED;                                         \
    }

#ifdef __cplusplus
}
#endif

#endif // TYPE_TABLE_H
