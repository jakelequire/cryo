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
// -------------------------------------- Property Data Types ---------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

DTPropertyTy *DTMPropertyTypes_createPropertyTemplate(void)
{
    DTPropertyTy *property = createDTProperty();
    if (!property)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Property Template\n");
        CONDITION_FAILED;
    }
    return property;
}

DTPropertyTy *DTMPropertyTypes_createPropertyType(const char *name, DataType *type, ASTNode *node, bool isStatic, bool isConst, bool isPublic, bool isPrivate, bool isProtected)
{
    DTPropertyTy *property = createDTProperty();
    if (!property)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Property Type\n");
        CONDITION_FAILED;
    }
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
        logMessage(LMI, "INFO", "DTM", "Checking Property: %s == %s", structNode->properties[i]->name, propertyName);
        if (strcmp(structNode->properties[i]->name, propertyName) == 0)
        {
            logMessage(LMI, "INFO", "DTM", "Found Property: %s == %s", structNode->properties[i]->name, propertyName);
            return structNode->properties[i]->node;
        }
    }
    logMessage(LMI, "INFO", "DTM", "Property not found: %s", propertyName);
    return NULL;
}

int DTMPropertyTypes_getStructPropertyIndex(DataType *structType, const char *propertyName)
{
    if (!structType)
    {
        fprintf(stderr, "[Data Type Manager] Error: Invalid Struct Type\n");
        structType->debug->printType(structType);
        DEBUG_BREAKPOINT;
        return -99;
    }
    if (structType->container->objectType != STRUCT_OBJ)
    {
        fprintf(stderr, "[Data Type Manager] Error: Invalid Struct Type\n");
        structType->debug->printType(structType);
        DEBUG_BREAKPOINT;
        return -99;
    }

    DataType *structDataType = structType;

    if (structType->container->typeOf == POINTER_TYPE)
    {
        DataType *baseType = structType->container->type.pointerType->baseType;
        if (baseType->container->objectType != STRUCT_OBJ)
        {
            fprintf(stderr, "[Data Type Manager] Error: Invalid Struct Type\n");
            baseType->debug->printType(baseType);
            DEBUG_BREAKPOINT;
            return -99;
        }

        structDataType = baseType;
    }

    logMessage(LMI, "INFO", "DTM", "Getting Struct Property Index: %s", propertyName);
    logMessage(LMI, "INFO", "DTM", "Struct Type: %s", structDataType->typeName);
    structDataType->debug->printType(structDataType);

    DTStructTy *structNode = structDataType->container->type.structType;
    for (int i = 0; i < structNode->propertyCount; i++)
    {
        if (strcmp(structNode->properties[i]->name, propertyName) == 0)
        {
            logMessage(LMI, "INFO", "DTM", "Found Property: %s == %s", structNode->properties[i]->name, propertyName);
            return i;
        }
    }

    logMessage(LMI, "INFO", "DTM", "Property not found: %s in %s", propertyName, structDataType->typeName);
    return -99;
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
    propertyTypes->getStructPropertyIndex = DTMPropertyTypes_getStructPropertyIndex;

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
    DataType *structDataType = DTM->dataTypes->wrapStructType(structType);
    structDataType->setTypeName(structDataType, structName);
    return structDataType;
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

DataType *DTMClassTypes_createClassTemplate(void)
{
    DTClassTy *classType = createDTClassTy();
    return DTM->dataTypes->wrapClassType(classType);
}

DataType *DTMClassTypes_createClassType(const char *className, DTPropertyTy **properties, int propertyCount, DataType **methods, int methodCount)
{
    DTClassTy *classType = createDTClassTy();
    classType->name = className;
    classType->properties = properties;
    classType->propertyCount = propertyCount;
    classType->methods = methods;
    classType->methodCount = methodCount;
    return DTM->dataTypes->wrapClassType(classType);
}

DataType *DTMClassTypes_createClassTypeWithMembers(
    const char *className,
    ASTNode **publicProperties, int publicPropertyCount,
    ASTNode **privateProperties, int privatePropertyCount,
    ASTNode **protectedProperties, int protectedPropertyCount,
    ASTNode **publicMethods, int publicMethodCount,
    ASTNode **privateMethods, int privateMethodCount,
    ASTNode **protectedMethods, int protectedMethodCount,
    bool hasConstructor,
    ASTNode **constructors, int ctorCount)
{
    DTClassTy *classType = createDTClassTy();
    classType->name = className;

    DTPropertyTy **publicProps = DTM->astInterface->createPropertyArrayFromAST(publicProperties, publicPropertyCount);
    DTPropertyTy **privateProps = DTM->astInterface->createPropertyArrayFromAST(privateProperties, privatePropertyCount);
    DTPropertyTy **protectedProps = DTM->astInterface->createPropertyArrayFromAST(protectedProperties, protectedPropertyCount);

    classType->publicMembers->properties = publicProps;
    classType->publicMembers->propertyCount = publicPropertyCount;
    classType->privateMembers->properties = privateProps;
    classType->privateMembers->propertyCount = privatePropertyCount;
    classType->protectedMembers->properties = protectedProps;
    classType->protectedMembers->propertyCount = protectedPropertyCount;

    classType->publicMembers->methods = DTM->astInterface->createTypeArrayFromASTArray(publicMethods, publicMethodCount);
    classType->publicMembers->methodCount = publicMethodCount;
    classType->privateMembers->methods = DTM->astInterface->createTypeArrayFromASTArray(privateMethods, privateMethodCount);
    classType->privateMembers->methodCount = privateMethodCount;
    classType->protectedMembers->methods = DTM->astInterface->createTypeArrayFromASTArray(protectedMethods, protectedMethodCount);
    classType->protectedMembers->methodCount = protectedMethodCount;

    classType->hasConstructor = hasConstructor;
    classType->constructors = DTM->astInterface->createTypeArrayFromASTArray(constructors, ctorCount);
    classType->ctorCount = ctorCount;

    DataType *classDataType = DTM->dataTypes->wrapClassType(classType);
    classDataType->setTypeName(classDataType, className);

    return classDataType;
}

DTMClassTypes *createDTMClassTypes(void)
{
    DTMClassTypes *classTypes = (DTMClassTypes *)malloc(sizeof(DTMClassTypes));
    if (!classTypes)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Class Types\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //
    classTypes->createClassTemplate = DTMClassTypes_createClassTemplate;
    classTypes->createClassType = DTMClassTypes_createClassType;
    classTypes->createClassTypeWithMembers = DTMClassTypes_createClassTypeWithMembers;

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
    if (!function)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Function Type\n");
        CONDITION_FAILED;
    }

    function->setParams(function, paramTypes, paramCount);
    function->setReturnType(function, returnType);

    DataType *functionType = DTM->dataTypes->wrapFunctionType(function);
    functionType->debug->printType(functionType);

    return functionType;
}

DataType *DTMFunctionTypes_createMethodType(const char *methodName, DataType *returnType, DataType **paramTypes, int paramCount)
{
    DTFunctionTy *function = createDTFunctionTy();
    function->setParams(function, paramTypes, paramCount);
    function->setReturnType(function, returnType);
    DataType *methodType = DTM->dataTypes->wrapFunctionType(function);
    methodType->setTypeName(methodType, methodName);

    return methodType;
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
// ---------------------------------- AST Interface Implementation ----------------------------------- //
// --------------------------------------------------------------------------------------------------- //

DataType *DTMastInterface_getTypeofASTNode(ASTNode *node)
{
    if (!node || node == NULL)
    {
        logMessage(LMI, "ERROR", "DTM", "Node is Null or Undefined!");
        fprintf(stderr, "Node is Null or Undefined!\n");
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
        break;
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
        DataType *stringType = DTM->primitives->createString();
        return stringType;
        break;
    }
    case NODE_STRING_EXPRESSION:
    {
        DataType *stringType = DTM->primitives->createString();
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

// --------------------------------------------------------------------------------------------------- //
// ---------------------------------- Data Type Manager Implementation ------------------------------- //
// --------------------------------------------------------------------------------------------------- //

void initGlobalDataTypeManagerInstance(void)
{
    globalDataTypeManager = createDataTypeManager();
    globalDataTypeManager->initialized = true;
}

void DataTypeManager_initDefinitions(void)
{
    register_VA_ARGS_type();
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
    manager->arrayTypes = createDTMArrayTypes();

    manager->compilerDefs = createDTMCompilerDefs();

    manager->helpers = createDTMHelpers();
    manager->debug = createDTMDebug();

    // ===================== [ Function Assignments ] ===================== //

    manager->initDefinitions = DataTypeManager_initDefinitions;
    manager->parseType = DTMParseType;
    manager->resolveType = DTMResolveType;
    manager->getTypeofASTNode = DTMastInterface_getTypeofASTNode;

    return manager;
}
