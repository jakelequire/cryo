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
#include "tools/logger/logger_config.h"
#include "diagnostics/diagnostics.h"

void FrontendSymbol_printFrontendSymbol(FrontendSymbol *symbol)
{
    DEBUG_PRINT_FILTER({
        if (!symbol)
        {
            printf("(NULL SYMBOL)\n");
            return;
        }

        printf("\n");
        printf("+----------------------------------------- [ Frontend Symbol ] -----------------------------------------+\n");
        printf("| Name: %-20s | ID: %-15s | Type: %-20s | Scope: %-14s \n",
               symbol->name, symbol->id, symbol->type->debug->toString(symbol->type), getScopeTypeString(symbol->scopeType));
        printf("+-------------------------------------------------------------------------------------------------------+\n");
        printf("| Line: %-8zu | Column: %-8zu | Is Defined: %-8s |\n",
               symbol->lineNumber, symbol->columnNumber, symbol->isDefined ? "true" : "false");
        printf("+-------------------------------------------------------------------------------------------------------+\n");
        symbol->node->print(symbol->node);
        printf("+-------------------------------------------------------------------------------------------------------+\n");
        printf("\n");
    });
}

FrontendSymbol *p_createSymbol(const char *name,
                               const char *id,
                               ASTNode *node,
                               DataType *type,
                               ScopeType scopeType,
                               size_t lineNumber,
                               size_t columnNumber,
                               bool isDefined)
{
    FrontendSymbol *symbol = (FrontendSymbol *)malloc(sizeof(FrontendSymbol));
    if (!symbol)
    {
        fprintf(stderr, "Error: Failed to allocate memory for symbol\n");
        return NULL;
    }

    symbol->name = (const char *)strdup(name);
    symbol->id = (const char *)strdup(id);
    symbol->node = node;
    symbol->type = type;
    symbol->scopeType = scopeType;
    symbol->lineNumber = lineNumber;
    symbol->columnNumber = columnNumber;
    symbol->isDefined = isDefined;

    symbol->print = FrontendSymbol_printFrontendSymbol;

    return symbol;
}

FrontendSymbol *p_createFunctionDeclarationSymbol(ASTNode *node)
{
    if (node == NULL)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "ASTNode is NULL");
        return NULL;
    }

    if (node->metaData->type != NODE_FUNCTION_DECLARATION)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "ASTNode is not a function declaration");
        return NULL;
    }

    const char *funcName = node->data.functionDecl->name;
    const char *funcID = "undef";
    ASTNode *funcNode = node;
    DataType *funcType = node->data.functionDecl->type;
    ScopeType scopeType = SCOPE_GLOBAL; // TODO: Determine the correct scope type
    size_t lineNumber = node->metaData->line;
    size_t columnNumber = node->metaData->column;
    bool isDefined = true; // TODO: Determine if the function is defined

    FrontendSymbol *symbol = p_createSymbol(funcName, funcID, funcNode, funcType, scopeType, lineNumber, columnNumber, isDefined);
    if (!symbol)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "Failed to create function symbol");
        return NULL;
    }

    logMessage(LMI, "INFO", "SymbolTable", "Created function symbol: %s", funcName);
    return symbol;
}

FrontendSymbol *p_createVariableDeclarationSymbol(ASTNode *node)
{
    if (node == NULL)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "ASTNode is NULL");
        return NULL;
    }
    if (node->metaData->type != NODE_VAR_DECLARATION)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "ASTNode is not a variable declaration");
        return NULL;
    }

    const char *varName = node->data.varDecl->name;
    const char *varID = "undef";
    ASTNode *varNode = node;
    DataType *varType = node->data.varDecl->type;
    ScopeType scopeType = SCOPE_GLOBAL; // TODO: Determine the correct scope type
    size_t lineNumber = node->metaData->line;
    size_t columnNumber = node->metaData->column;
    bool isDefined = true; // TODO: Determine if the variable is defined

    FrontendSymbol *symbol = p_createSymbol(varName, varID, varNode, varType, scopeType, lineNumber, columnNumber, isDefined);
    if (!symbol)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "Failed to create variable symbol");
        return NULL;
    }

    logMessage(LMI, "INFO", "SymbolTable", "Created variable symbol: %s", varName);
    return symbol;
}

FrontendSymbol *p_createTypeDeclarationSymbol(ASTNode *node)
{
    if (node == NULL)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "ASTNode is NULL");
        return NULL;
    }

    if (node->metaData->type != NODE_TYPE)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "ASTNode is not a type declaration");
        return NULL;
    }

    const char *typeName = node->data.typeDecl->name;
    const char *typeID = "undef";
    ASTNode *typeNode = node;
    DataType *typeData = node->data.typeDecl->type;
    ScopeType scopeType = SCOPE_GLOBAL; // TODO: Determine the correct scope type
    size_t lineNumber = node->metaData->line;
    size_t columnNumber = node->metaData->column;
    bool isDefined = true; // TODO: Determine if the type is defined

    FrontendSymbol *symbol = p_createSymbol(typeName, typeID, typeNode, typeData, scopeType, lineNumber, columnNumber, isDefined);
    if (!symbol)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "Failed to create type symbol");
        return NULL;
    }

    logMessage(LMI, "INFO", "SymbolTable", "Created type symbol: %s", typeName);
    return symbol;
}

FrontendSymbol *p_createClassDeclarationSymbol(ASTNode *node)
{
    if (node == NULL)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "ASTNode is NULL");
        return NULL;
    }

    if (node->metaData->type != NODE_CLASS)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "ASTNode is not a class declaration");
        return NULL;
    }

    const char *className = node->data.classNode->name;
    const char *classID = "undef";
    ASTNode *classNode = node;
    DataType *classType = node->data.classNode->type;
    ScopeType scopeType = SCOPE_GLOBAL; // TODO: Determine the correct scope type
    size_t lineNumber = node->metaData->line;
    size_t columnNumber = node->metaData->column;
    bool isDefined = true; // TODO: Determine if the class is defined

    FrontendSymbol *symbol = p_createSymbol(className, classID, classNode, classType, scopeType, lineNumber, columnNumber, isDefined);
    if (!symbol)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "Failed to create class symbol");
        return NULL;
    }

    logMessage(LMI, "INFO", "SymbolTable", "Created class symbol: %s", className);
    return symbol;
}

FrontendSymbol *p_createStructDeclarationSymbol(ASTNode *node)
{
    if (node == NULL)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "ASTNode is NULL");
        return NULL;
    }

    if (node->metaData->type != NODE_STRUCT_DECLARATION)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "ASTNode is not a struct declaration");
        return NULL;
    }

    const char *structName = node->data.structNode->name;
    const char *structID = "undef";
    ASTNode *structNode = node;
    DataType *structType = node->data.structNode->type;
    ScopeType scopeType = SCOPE_GLOBAL; // TODO: Determine the correct scope type
    size_t lineNumber = node->metaData->line;
    size_t columnNumber = node->metaData->column;
    bool isDefined = true; // TODO: Determine if the struct is defined

    FrontendSymbol *symbol = p_createSymbol(structName, structID, structNode, structType, scopeType, lineNumber, columnNumber, isDefined);
    if (!symbol)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "Failed to create struct symbol");
        return NULL;
    }

    logMessage(LMI, "INFO", "SymbolTable", "Created struct symbol: %s", structName);
    return symbol;
}

FrontendSymbol *p_createEnumDeclarationSymbol(ASTNode *node)
{
    if (node == NULL)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "ASTNode is NULL");
        return NULL;
    }
}

FrontendSymbol *p_createPropertyDeclarationSymbol(ASTNode *node)
{
    if (node == NULL)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "ASTNode is NULL");
        return NULL;
    }
}

FrontendSymbol *p_createMethodDeclarationSymbol(ASTNode *node)
{
    if (node == NULL)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "ASTNode is NULL");
        return NULL;
    }

    if (node->metaData->type != NODE_METHOD)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "ASTNode is not a method declaration");
        return NULL;
    }

    const char *methodName = node->data.method->name;
    const char *methodID = "undef";
    ASTNode *methodNode = node;
    DataType *methodType = node->data.method->type;
    ScopeType scopeType = SCOPE_GLOBAL; // TODO: Determine the correct scope type
    size_t lineNumber = node->metaData->line;
    size_t columnNumber = node->metaData->column;
    bool isDefined = true; // TODO: Determine if the method is defined

    FrontendSymbol *symbol = p_createSymbol(methodName, methodID, methodNode, methodType, scopeType, lineNumber, columnNumber, isDefined);
    if (!symbol)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "Failed to create method symbol");
        return NULL;
    }

    logMessage(LMI, "INFO", "SymbolTable", "Created method symbol: %s", methodName);
    return symbol;
}

FrontendSymbol *p_createImportDeclarationSymbol(ASTNode *node)
{
    if (node == NULL)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "ASTNode is NULL");
        return NULL;
    }
}
FrontendSymbol *p_createExternDeclarationSymbol(ASTNode *node)
{
    if (node == NULL)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "ASTNode is NULL");
        return NULL;
    }
}

FrontendSymbol *p_createParamDeclarationSymbol(ASTNode *node)
{
    if (node == NULL)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "ASTNode is NULL");
        return NULL;
    }

    if (node->metaData->type != NODE_PARAM)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "ASTNode is not a parameter declaration");
        return NULL;
    }

    const char *paramName = node->data.param->name;
    logMessage(LMI, "INFO", "SymbolTable", "Creating parameter symbol: %s", paramName);
    const char *paramID = "undef";
    ASTNode *paramNode = node;
    DataType *paramType = node->data.param->type;
    paramType->debug->printVerbosType(paramType);
    ScopeType scopeType = SCOPE_GLOBAL;
    logMessage(LMI, "INFO", "SymbolTable", "Scope Type: %s", getScopeTypeString(scopeType));
    size_t lineNumber = node->metaData->line;
    size_t columnNumber = node->metaData->column;
    logMessage(LMI, "INFO", "SymbolTable", "Line: %zu, Column: %zu", lineNumber, columnNumber);
    bool isDefined = true; // TODO: Determine if the parameter is defined

    FrontendSymbol *symbol = p_createSymbol(paramName, paramID, paramNode, paramType, scopeType, lineNumber, columnNumber, isDefined);
    if (!symbol)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "Failed to create parameter symbol");
        return NULL;
    }

    logMessage(LMI, "INFO", "SymbolTable", "Created parameter symbol: %s", paramName);
    return symbol;
}

// =========================================================================================================== //
//                                                                                                             //
//                                          ASTNode to Symbol Conversion                                       //
//                                                                                                             //
// =========================================================================================================== //

FrontendSymbol *astNodeToSymbol(ASTNode *node)
{
    if (node == NULL)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "ASTNode is NULL");
        return NULL;
    }

    switch (node->metaData->type)
    {
    case NODE_FUNCTION_DECLARATION:
        return p_createFunctionDeclarationSymbol(node);
    case NODE_VAR_DECLARATION:
        return p_createVariableDeclarationSymbol(node);
    case NODE_CLASS:
        return p_createClassDeclarationSymbol(node);
    case NODE_TYPE:
        return p_createTypeDeclarationSymbol(node);
    case NODE_ENUM:
        return p_createEnumDeclarationSymbol(node);
    case NODE_EXTERN_FUNCTION:
        return p_createExternDeclarationSymbol(node);
    case NODE_STRUCT_DECLARATION:
        return p_createStructDeclarationSymbol(node);
    case NODE_METHOD:
        return p_createMethodDeclarationSymbol(node);
    case NODE_PARAM:
        return p_createParamDeclarationSymbol(node);

    case NODE_PROGRAM:
    case NODE_STATEMENT:
    case NODE_EXPRESSION:
    case NODE_BINARY_EXPR:
    case NODE_UNARY_EXPR:
    case NODE_LITERAL_EXPR:
    case NODE_VAR_NAME:
    case NODE_FUNCTION_CALL:
    case NODE_IF_STATEMENT:
    case NODE_WHILE_STATEMENT:
    case NODE_FOR_STATEMENT:
    case NODE_RETURN_STATEMENT:
    case NODE_BLOCK:
    case NODE_FUNCTION_BLOCK:
    case NODE_EXPRESSION_STATEMENT:
    case NODE_ASSIGN:
    case NODE_PARAM_LIST:
    case NODE_STRING_LITERAL:
    case NODE_STRING_EXPRESSION:
    case NODE_BOOLEAN_LITERAL:
    case NODE_ARRAY_LITERAL:
    case NODE_IMPORT_STATEMENT:
    case NODE_EXTERN_STATEMENT:
    case NODE_ARG_LIST:
    case NODE_NAMESPACE:
    case NODE_INDEX_EXPR:
    case NODE_VAR_REASSIGN:
    case NODE_PROPERTY:
    case NODE_CUSTOM_TYPE:
    case NODE_SCOPED_FUNCTION_CALL:
    case NODE_EXTERNAL_SYMBOL:
    case NODE_STRUCT_CONSTRUCTOR:
    case NODE_PROPERTY_ACCESS:
    case NODE_THIS:
    case NODE_THIS_ASSIGNMENT:
    case NODE_PROPERTY_REASSIGN:
    case NODE_IDENTIFIER:
    case NODE_METHOD_CALL:
    case NODE_GENERIC_DECL:
    case NODE_GENERIC_INST:
    case NODE_CLASS_CONSTRUCTOR:
    case NODE_OBJECT_INST:
    case NODE_NULL_LITERAL:
    case NODE_TYPEOF:
    case NODE_USING:
    case NODE_MODULE:
    case NODE_ANNOTATION:
    case NODE_TYPE_CAST:
    case NODE_DISCARD:
    case NODE_IMPLEMENTATION:
    case NODE_UNKNOWN:
    {
        const char *nodeTypeStr = CryoNodeTypeToString(node->metaData->type);
        logMessage(LMI, "WARN", "SymbolTable", "Unhandled ASTNode type: %s", nodeTypeStr);
        CONDITION_FAILED;
        break;
    }
    default:
    {
        // Handle other node types
        const char *nodeTypeStr = CryoNodeTypeToString(node->metaData->type);
        logMessage(LMI, "ERROR", "SymbolTable", "Unhandled ASTNode type: %s", nodeTypeStr);
        CONDITION_FAILED;
        break;
    }
    }

    return NULL;
}
