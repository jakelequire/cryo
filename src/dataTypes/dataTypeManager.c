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
#include "dataTypes/dataTypeManager.h"

// The global Data Type Manager instance. This is a static instance that is used to manage data types in the compiler.
// This is initialized in the `main` function of the compiler.
DataTypeManager *globalDataTypeManager = NULL;

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------- DTM Debug Functions ----------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

void DTMDebug_printDataType(DataType *type)
{
    if (!type)
    {
        fprintf(stderr, "[Data Type Manager] Error: Attempted to print NULL data type\n");
        CONDITION_FAILED;
    }

    const char *typeName = type->typeName;
    if (!typeName || typeName == NULL)
    {
        // Check to see if the DataType is a primitive type
        if (type->container->typeOf == PRIM_TYPE)
        {
            switch (type->container->primitive)
            {
            case PRIM_INT:
                typeName = "int";
                break;
            case PRIM_FLOAT:
                typeName = "float";
                break;
            case PRIM_STRING:
                typeName = "string";
                break;
            case PRIM_BOOLEAN:
                typeName = "boolean";
                break;
            case PRIM_VOID:
                typeName = "void";
                break;
            case PRIM_NULL:
                typeName = "null";
                break;
            case PRIM_ANY:
                typeName = "any";
                break;
            case PRIM_UNDEFINED:
                typeName = "undefined";
                break;
            case PRIM_AUTO:
                typeName = "auto";
                break;
            default:
                fprintf(stderr, "[Data Type Manager] Error: Unknown primitive type\n");
                CONDITION_FAILED;
            }
        }
        else
        {
            fprintf(stderr, "[Data Type Manager] Error: Unknown data type\n");
            CONDITION_FAILED;
        }
    }

    printf("%s\n", typeName);
}

const char *DTMDebug_typeofDataTypeToString(TypeofDataType type)
{
    switch (type)
    {
    case PRIM_TYPE:
        return "Primitive Type";
    case ARRAY_TYPE:
        return "Array Type";
    case ENUM_TYPE:
        return "Enum Type";
    case FUNCTION_TYPE:
        return "Function Type";
    case GENERIC_TYPE:
        return "Generic Type";
    case OBJECT_TYPE:
        return "Object Type";
    case TYPE_DEF:
        return "Type Definition";
    case UNKNOWN_TYPE:
        return "<UNKNOWN>";
    default:
        return "<UNKNOWN>";
    }
}

const char *DTMDebug_typeofObjectTypeToString(TypeofObjectType type)
{
    switch (type)
    {
    case STRUCT_OBJ:
        return "Struct Object";
    case CLASS_OBJ:
        return "Class Object";
    case INTERFACE_OBJ:
        return "Interface Object";
    case TRAIT_OBJ:
        return "Trait Object";
    case MODULE_OBJ:
        return "Module Object";
    case OBJECT_OBJ:
        return "Object Object";
    case NON_OBJECT:
        return "Non-Object";
    case UNKNOWN_OBJECT:
        return "<UNKNOWN>";
    default:
        return "<UNKNOWN>";
    }
}

const char *DTMDebug_primitiveDataTypeToString(PrimitiveDataType type)
{
    switch (type)
    {
    case PRIM_INT:
        return "int";
    case PRIM_FLOAT:
        return "float";
    case PRIM_STRING:
        return "string";
    case PRIM_BOOLEAN:
        return "boolean";
    case PRIM_VOID:
        return "void";
    case PRIM_UNKNOWN:
        return "<UNKNOWN>";
    default:
        return "<UNKNOWN>";
    }
}

const char *DTMDebug_primitiveDataTypeToCType(PrimitiveDataType type)
{
    switch (type)
    {
    case PRIM_INT:
        return "int";
    case PRIM_FLOAT:
        return "float";
    case PRIM_STRING:
        return "char *";
    case PRIM_BOOLEAN:
        return "bool";
    case PRIM_VOID:
        return "void";
    case PRIM_UNKNOWN:
        return "<UNKNOWN>";
    default:
        return "<UNKNOWN>";
    }
}

DTMDebug *createDTMDebug(void)
{
    DTMDebug *debug = (DTMDebug *)malloc(sizeof(DTMDebug));
    if (!debug)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Debug\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    debug->printDataType = DTMDebug_printDataType;

    debug->typeofDataTypeToString = DTMDebug_typeofDataTypeToString;
    debug->typeofObjectTypeToString = DTMDebug_typeofObjectTypeToString;
    debug->primitiveDataTypeToString = DTMDebug_primitiveDataTypeToString;
    debug->primitiveDataTypeToCType = DTMDebug_primitiveDataTypeToCType;

    return debug;
}

// --------------------------------------------------------------------------------------------------- //
// -------------------------------------- Property Data Types ---------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

DTPropertyTy *DTMPropertyTypes_createPropertyTemplate(void)
{
    DTPropertyTy *property = createDTProperty();
    return property;
}

DTPropertyTy *DTMPropertyTypes_createPropertyType(const char *name, DataType *type, ASTNode *node, bool isStatic, bool isConst, bool isPublic, bool isPrivate, bool isProtected)
{
    DTPropertyTy *property = createDTProperty();
    property->node = node;
    property->setName(property, name);
    property->setType(property, type);
    property->setStatic(property, isStatic);
    property->setConst(property, isConst);
    property->setPublic(property, isPublic);
    property->setPrivate(property, isPrivate);
    property->setProtected(property, isProtected);
    return property;
}

ASTNode *DTMPropertyTypes_findStructPropertyNode(DTStructTy *structNode, const char *propertyName)
{
    for (int i = 0; i < structNode->propertyCount; i++)
    {
        if (strcmp(structNode->properties[i]->name, propertyName) == 0)
        {
            return structNode->properties[i]->node;
        }
    }
    return NULL;
}

DTMPropertyTypes *createDTMPropertyTypes(void)
{
    DTMPropertyTypes *propertyTypes = (DTMPropertyTypes *)malloc(sizeof(DTMPropertyTypes));
    if (!propertyTypes)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Property Types\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    propertyTypes->createPropertyTemplate = DTMPropertyTypes_createPropertyTemplate;
    propertyTypes->createPropertyType = DTMPropertyTypes_createPropertyType;
    propertyTypes->findStructPropertyNode = DTMPropertyTypes_findStructPropertyNode;

    return propertyTypes;
}

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------- Struct Data Types ------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

DataType *DTMStructTypes_createStructTemplate(void)
{
    DTStructTy *structType = createDTStructTy();
    return DTM->dataTypes->wrapStructType(structType);
}

DataType *DTMStructTypes_createCompleteStructType(const char *structName, DTPropertyTy **properties, int propertyCount, DataType **methods, int methodCount, bool hasConstructor, DataType **ctorArgs, int *ctorArgCount)
{
    DTStructTy *structType = createDTStructTy();
    structType->name = structName;
    structType->properties = properties;
    structType->propertyCount = propertyCount;
    structType->methods = methods;
    structType->methodCount = methodCount;
    structType->hasConstructor = hasConstructor;
    structType->ctorParams = ctorArgs;
    structType->ctorParamCount = *ctorArgCount;
    return DTM->dataTypes->wrapStructType(structType);
}

DataType *DTMStructTypes_createStructType(const char *structName, DTPropertyTy **properties, int propertyCount, DataType **methods, int methodCount)
{
    DTStructTy *structType = createDTStructTy();
    structType->name = structName;
    structType->properties = properties;
    structType->propertyCount = propertyCount;
    structType->methods = methods;
    structType->methodCount = methodCount;
    return DTM->dataTypes->wrapStructType(structType);
}

DTMStructTypes *createDTMStructTypes(void)
{
    DTMStructTypes *structTypes = (DTMStructTypes *)malloc(sizeof(DTMStructTypes));
    if (!structTypes)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Struct Types\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    structTypes->createStructTemplate = DTMStructTypes_createStructTemplate;
    structTypes->createCompleteStructType = DTMStructTypes_createCompleteStructType;
    structTypes->createStructType = DTMStructTypes_createStructType;

    return structTypes;
}

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------- Class Data Types -------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

DTMClassTypes *createDTMClassTypes(void)
{
    DTMClassTypes *classTypes = (DTMClassTypes *)malloc(sizeof(DTMClassTypes));
    if (!classTypes)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Class Types\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    return classTypes;
}

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------- Function Data Types ----------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

DataType *DTMFunctionTypes_createFunctionTemplate(void)
{
    DTFunctionTy *function = createDTFunctionTy();
    return DTM->dataTypes->wrapFunctionType(function);
}

DataType *DTMFunctionTypes_createFunctionType(DataType **paramTypes, int paramCount, DataType *returnType)
{
    DTFunctionTy *function = createDTFunctionTy();
    function->setParams(function, paramTypes, paramCount);
    function->setReturnType(function, returnType);
    return DTM->dataTypes->wrapFunctionType(function);
}

DataType *DTMFunctionTypes_createMethodType(const char *methodName, DataType *returnType, DataType **paramTypes, int paramCount)
{
    DTFunctionTy *function = createDTFunctionTy();
    function->setParams(function, paramTypes, paramCount);
    function->setReturnType(function, returnType);
    return DTM->dataTypes->wrapFunctionType(function);
}

DTMFunctionTypes *createDTMFunctionTypes(void)
{
    DTMFunctionTypes *functionTypes = (DTMFunctionTypes *)malloc(sizeof(DTMFunctionTypes));
    if (!functionTypes)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Function Types\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    functionTypes->createFunctionTemplate = DTMFunctionTypes_createFunctionTemplate;
    functionTypes->createFunctionType = DTMFunctionTypes_createFunctionType;
    functionTypes->createMethodType = DTMFunctionTypes_createMethodType;

    return functionTypes;
}

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------- Generic Data Types ------------------------------------------ //
// --------------------------------------------------------------------------------------------------- //

DataType *DTMGenerics_createGenericTypeInstance(DataType *genericType, DataType **paramTypes, int paramCount)
{
    DTGenericTy *generic = DTM->generics->createEmptyGenericType();
    generic->paramCount = paramCount;
    generic->dimensions = 0;
    generic->constraint = NULL;
    generic->next = NULL;
    return DTM->dataTypes->wrapGenericType(generic);
}

DTGenericTy *DTMGenerics_createEmptyGenericType(void)
{
    DTGenericTy *generic = (DTGenericTy *)malloc(sizeof(DTGenericTy));
    if (!generic)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Generic Type\n");
        CONDITION_FAILED;
    }
    return generic;
}

DTMGenerics *createDTMGenerics(void)
{
    DTMGenerics *generics = (DTMGenerics *)malloc(sizeof(DTMGenerics));
    if (!generics)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Generics\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    generics->createGenericTypeInstance = DTMGenerics_createGenericTypeInstance;
    generics->createEmptyGenericType = DTMGenerics_createEmptyGenericType;

    return generics;
}

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------- Enum Data Types --------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

DTMEnums *createDTMEnums(void)
{
    DTMEnums *enums = (DTMEnums *)malloc(sizeof(DTMEnums));
    if (!enums)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Enums\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    return enums;
}

// --------------------------------------------------------------------------------------------------- //
// ---------------------------------- Data Types Implementation -------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

void DataTypes_isConst(DataType *type, bool isConst)
{
    type->isConst = isConst;
}

void DataTypes_isPointer(DataType *type, bool isPointer)
{
    type->isPointer = isPointer;
}

void DataTypes_isReference(DataType *type, bool isReference)
{
    type->isReference = isReference;
}

TypeContainer *DTMTypeContainerWrappers_createTypeContainer(void)
{
    logMessage(LMI, "INFO", "DTM", "Creating Type Container...");
    TypeContainer *container = (TypeContainer *)malloc(sizeof(TypeContainer));
    if (!container)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to create Type Container\n");
        CONDITION_FAILED;
    }

    return container;
}

DataType *DTMTypeContainerWrappers_wrapTypeContainer(TypeContainer *container)
{
    logMessage(LMI, "INFO", "DTM", "Wrapping Type Container...");
    DataType *type = (DataType *)malloc(sizeof(DataType));
    if (!type)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to wrap Type Container\n");
        CONDITION_FAILED;
    }

    type->container = (TypeContainer *)malloc(sizeof(TypeContainer));
    if (!type->container)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to wrap Type Container\n");
        CONDITION_FAILED;
    }
    type->container = container;

    logMessage(LMI, "INFO", "DTM", "Setting Type Properties...");

    type->setConst = DataTypes_isConst;
    type->setPointer = DataTypes_isPointer;
    type->setReference = DataTypes_isReference;

    logMessage(LMI, "INFO", "DTM", "Returning Wrapped Type.");

    return type;
}

DataType *DTMTypeContainerWrappers_wrapSimpleType(struct DTSimpleTy_t *simpleTy)
{
    TypeContainer *container = DTMTypeContainerWrappers_createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->objectType = NON_OBJECT;
    container->type.simpleType = simpleTy;

    return DTMTypeContainerWrappers_wrapTypeContainer(container);
}

DataType *DTMTypeContainerWrappers_wrapArrayType(struct DTArrayTy_t *arrayTy)
{
    TypeContainer *container = DTMTypeContainerWrappers_createTypeContainer();
    container->typeOf = ARRAY_TYPE;
    container->objectType = NON_OBJECT;
    container->type.arrayType = arrayTy;

    return DTMTypeContainerWrappers_wrapTypeContainer(container);
}

DataType *DTMTypeContainerWrappers_wrapEnumType(struct DTEnumTy_t *enumTy)
{
    TypeContainer *container = DTMTypeContainerWrappers_createTypeContainer();
    container->typeOf = ENUM_TYPE;
    container->objectType = NON_OBJECT;
    container->type.enumType = enumTy;

    return DTMTypeContainerWrappers_wrapTypeContainer(container);
}

DataType *DTMTypeContainerWrappers_wrapFunctionType(struct DTFunctionTy_t *functionTy)
{
    TypeContainer *container = DTMTypeContainerWrappers_createTypeContainer();
    container->typeOf = FUNCTION_TYPE;
    container->objectType = NON_OBJECT;
    container->type.functionType = functionTy;

    return DTMTypeContainerWrappers_wrapTypeContainer(container);
}

DataType *DTMTypeContainerWrappers_wrapStructType(struct DTStructTy_t *structTy)
{
    TypeContainer *container = DTMTypeContainerWrappers_createTypeContainer();
    container->typeOf = OBJECT_TYPE;
    container->objectType = STRUCT_OBJ;
    container->type.structType = structTy;

    return DTMTypeContainerWrappers_wrapTypeContainer(container);
}

DataType *DTMTypeContainerWrappers_wrapClassType(struct DTClassTy_t *classTy)
{
    TypeContainer *container = DTMTypeContainerWrappers_createTypeContainer();
    container->typeOf = OBJECT_TYPE;
    container->objectType = CLASS_OBJ;
    container->type.classType = classTy;

    return DTMTypeContainerWrappers_wrapTypeContainer(container);
}

DataType *DTMTypeContainerWrappers_wrapObjectType(struct DTObjectTy_t *objectTy)
{
    TypeContainer *container = DTMTypeContainerWrappers_createTypeContainer();
    container->typeOf = OBJECT_TYPE;
    container->objectType = OBJECT_OBJ;
    container->type.objectType = objectTy;

    return DTMTypeContainerWrappers_wrapTypeContainer(container);
}

DataType *DTMTypeContainerWrappers_wrapGenericType(struct DTGenericTy_t *genericTy)
{
    TypeContainer *container = DTMTypeContainerWrappers_createTypeContainer();
    container->typeOf = GENERIC_TYPE;
    container->type.genericType = genericTy;

    return DTMTypeContainerWrappers_wrapTypeContainer(container);
}

DTMDataTypes *createDTMDataTypes(void)
{
    DTMDataTypes *dataTypes = (DTMDataTypes *)malloc(sizeof(DTMDataTypes));
    if (!dataTypes)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Data Types\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    dataTypes->createTypeContainer = DTMTypeContainerWrappers_createTypeContainer;
    dataTypes->wrapTypeContainer = DTMTypeContainerWrappers_wrapTypeContainer;

    dataTypes->wrapSimpleType = DTMTypeContainerWrappers_wrapSimpleType;
    dataTypes->wrapArrayType = DTMTypeContainerWrappers_wrapArrayType;
    dataTypes->wrapEnumType = DTMTypeContainerWrappers_wrapEnumType;
    dataTypes->wrapFunctionType = DTMTypeContainerWrappers_wrapFunctionType;
    dataTypes->wrapStructType = DTMTypeContainerWrappers_wrapStructType;
    dataTypes->wrapClassType = DTMTypeContainerWrappers_wrapClassType;
    dataTypes->wrapObjectType = DTMTypeContainerWrappers_wrapObjectType;
    dataTypes->wrapGenericType = DTMTypeContainerWrappers_wrapGenericType;

    return dataTypes;
}

// --------------------------------------------------------------------------------------------------- //
// ---------------------------------- AST Interface Implementation ----------------------------------- //
// --------------------------------------------------------------------------------------------------- //

DataType *DTMastInterface_getTypeofASTNode(ASTNode *node)
{
    fprintf(stderr, "[Data Type Manager] Error: AST Interface not implemented\n");
    return NULL;
}

DataType **DTMastInterface_createTypeArrayFromAST(ASTNode *node)
{
    fprintf(stderr, "[Data Type Manager] Error: AST Interface not implemented\n");
    return NULL;
}

DataType **DTMastInterface_createTypeArrayFromASTArray(ASTNode **nodes, int count)
{
    fprintf(stderr, "[Data Type Manager] Error: AST Interface not implemented\n");
    return NULL;
}

DTPropertyTy **DTMastInterface_createPropertyArrayFromAST(ASTNode **nodes, int count)
{
    fprintf(stderr, "[Data Type Manager] Error: AST Interface not implemented\n");
    return NULL;
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

// --------------------------------------------------------------------------------------------------- //
// ---------------------------------- Data Type Manager Implementation ------------------------------- //
// --------------------------------------------------------------------------------------------------- //

void initGlobalDataTypeManagerInstance(void)
{
    globalDataTypeManager = createDataTypeManager();
    globalDataTypeManager->initialized = true;
}

void DataTypeManager_initDefinitions(const char *compilerRootPath, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    if (DTM->defsInitialized)
    {
        fprintf(stderr, "[Data Type Manager] Error: Definitions already initialized\n");
        CONDITION_FAILED;
        return;
    }

    const char *defsPath = fs->appendStrings(compilerRootPath, "/Std/Runtime/defs.cryo");
    if (!defsPath)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate memory for definitions path\n");
        CONDITION_FAILED;
    }

    ASTNode *defsNode = compileForASTNode(defsPath, state, globalTable);
    if (!defsNode)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to compile definitions\n");
        CONDITION_FAILED;
    }

    defsNode->print(defsNode);

    DTM->symbolTable->importRootNode(DTM->symbolTable, defsNode);

    DEBUG_BREAKPOINT;
    return;
}

DataTypeManager *createDataTypeManager(void)
{
    DataTypeManager *manager = (DataTypeManager *)malloc(sizeof(DataTypeManager));
    if (!manager)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate Data Type Manager\n");
        CONDITION_FAILED;
    }

    // ===================== [ Property Assignments ] ===================== //

    manager->initialized = false;
    manager->defsInitialized = false;

    manager->symbolTable = createDTMSymbolTable();
    manager->dataTypes = createDTMDataTypes();
    manager->validation = createDTMTypeValidation();

    manager->primitives = createDTMPrimitives();
    manager->propertyTypes = createDTMPropertyTypes();
    manager->astInterface = createDTMAstInterface();
    manager->structTypes = createDTMStructTypes();
    manager->classTypes = createDTMClassTypes();
    manager->functionTypes = createDTMFunctionTypes();
    manager->generics = createDTMGenerics();
    manager->enums = createDTMEnums();

    manager->helpers = createDTMHelpers();
    manager->debug = createDTMDebug();

    // ===================== [ Function Assignments ] ===================== //

    manager->initDefinitions = DataTypeManager_initDefinitions;
    manager->parseType = DTMParseType;
    manager->resolveType = DTMResolveType;

    return manager;
}
