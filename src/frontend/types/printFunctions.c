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
#include "frontend/dataTypes.h"
#include "tools/logger/logger_config.h"

void printTypeTable(TypeTable *table)
{
    DEBUG_PRINT_FILTER({
        printf("\n");
        printf(BOLD CYAN "╓────────────────────────── Type Table ──────────────────────────╖\n" COLOR_RESET);
        printf("  Type Table: %p\n", (void *)table);
        printf("  Type Count: %d\n", table->count);
        printf("  Type Capacity: %d\n", table->capacity);
        printf(BOLD CYAN "╟────────────────────────────────────────────────────────────────╢\n" COLOR_RESET);

        for (int i = 0; i < table->count; i++)
        {
            DataType *type = table->types[i];
            printFormattedType(type);
        }
        printf("   ────────────────────────────────────────────────────────────\n");
        printf(BOLD CYAN "╙────────────────────────────────────────────────────────────────╜\n" COLOR_RESET);
        printf("\n");
        if (table->count == 0)
        {
            printf("  No types in the type table.\n");
            printf(BOLD CYAN "╙────────────────────────────────────────────────────────────────╜\n" COLOR_RESET);
            printf("\n");
        }
    });
}

char *TypeofDataTypeToString(TypeofDataType type)
{
    switch (type)
    {
    case PRIMITIVE_TYPE:
        return "PRIMITIVE_TYPE";
    case ARRAY_TYPE:
        return "ARRAY_TYPE";
    case STRUCT_TYPE:
        return "STRUCT_TYPE";
    case ENUM_TYPE:
        return "ENUM_TYPE";
    case FUNCTION_TYPE:
        return "FUNCTION_TYPE";
    case GENERIC_TYPE:
        return "GENERIC_TYPE";
    case CLASS_TYPE:
        return "CLASS_TYPE";
    case UNKNOWN_TYPE:
        return "UNKNOWN_TYPE";
    default:
        return "<TYPE UNKNOWN>";
    }
}

char *PrimitiveDataTypeToString(PrimitiveDataType type)
{
    switch (type)
    {
    case PRIM_INT:
        return LIGHT_CYAN BOLD "int" COLOR_RESET;
    case PRIM_I8:
        return LIGHT_CYAN BOLD "i8" COLOR_RESET;
    case PRIM_I16:
        return LIGHT_CYAN BOLD "i16" COLOR_RESET;
    case PRIM_I32:
        return LIGHT_CYAN BOLD "i32" COLOR_RESET;
    case PRIM_I64:
        return LIGHT_CYAN BOLD "i64" COLOR_RESET;
    case PRIM_I128:
        return LIGHT_CYAN BOLD "i128" COLOR_RESET;

    case PRIM_FLOAT:
        return LIGHT_CYAN BOLD "float" COLOR_RESET;
    case PRIM_STRING:
        return LIGHT_CYAN BOLD "string" COLOR_RESET;
    case PRIM_BOOLEAN:
        return LIGHT_CYAN BOLD "boolean" COLOR_RESET;
    case PRIM_VOID:
        return LIGHT_CYAN BOLD "void" COLOR_RESET;
    case PRIM_NULL:
        return LIGHT_CYAN BOLD "null" COLOR_RESET;
    case PRIM_ANY:
        return LIGHT_CYAN BOLD "any" COLOR_RESET;
    case PRIM_CUSTOM:
        return LIGHT_CYAN BOLD "Custom Type" COLOR_RESET;
    case PRIM_UNKNOWN:
        return LIGHT_RED BOLD "<UNKNOWN>" COLOR_RESET;
    default:
        return LIGHT_RED BOLD "<PRIMITIVE UNKNOWN>" COLOR_RESET;
    }

    return "<UNKNOWN>";
}

char *PrimitiveDataTypeToString_UF(PrimitiveDataType type)
{
    switch (type)
    {
    case PRIM_INT:
        return "int";
    case PRIM_I8:
        return "i8";
    case PRIM_I16:
        return "i16";
    case PRIM_I32:
        return "i32";
    case PRIM_I64:
        return "i64";
    case PRIM_I128:
        return "i128";

    case PRIM_FLOAT:
        return "float";
    case PRIM_STRING:
        return "string";
    case PRIM_BOOLEAN:
        return "boolean";
    case PRIM_VOID:
        return "void";
    case PRIM_NULL:
        return "null";
    case PRIM_ANY:
        return "any";
    case PRIM_CUSTOM:
        return "Custom Type";
    case PRIM_UNKNOWN:
        return "<UNKNOWN>";
    default:
        return "<PRIMITIVE UNKNOWN>";
    }

    return "<UNKNOWN>";
}

char *DataTypeToStringUnformatted(DataType *type)
{
    if (!type)
        return "<NULL DATATYPE>";

    char *typeString = (char *)malloc(128);
    if (!typeString)
    {
        fprintf(stderr, "[DataTypes] Error: Failed to allocate memory for type string.\n");
        return NULL;
    }

    switch (type->container->baseType)
    {
    case PRIMITIVE_TYPE:
        if (type->container->isArray)
        {
            sprintf(typeString, "%s[]", PrimitiveDataTypeToString_UF(type->container->primitive));
            return typeString;
        }
        sprintf(typeString, "%s", PrimitiveDataTypeToString_UF(type->container->primitive));
        break;

    case STRUCT_TYPE:
        sprintf(typeString, "%s", type->container->custom.structDef->name);
        break;

    case CLASS_TYPE:
        sprintf(typeString, "%s", type->container->custom.classDef->name);
        break;

    case FUNCTION_TYPE:
        sprintf(typeString, "%s", getFunctionTypeStr_UF(type->container->custom.funcDef));
        break;

    default:
        sprintf(typeString, "<?>");
        break;
    }

    sprintf(typeString, "%s ", typeString);

    return typeString;
}

char *DataTypeToString(DataType *dataType)
{
    if (!dataType)
        return "<NULL DATATYPE>";

    char *typeString = (char *)malloc(128);
    if (!typeString)
    {
        fprintf(stderr, "[DataTypes] Error: Failed to allocate memory for type string.\n");
        return NULL;
    }

    switch (dataType->container->baseType)
    {
    case PRIMITIVE_TYPE:
        if (dataType->container->isArray)
        {
            sprintf(typeString, LIGHT_CYAN BOLD "%s[]" COLOR_RESET,
                    PrimitiveDataTypeToString(dataType->container->primitive));
            return typeString;
        }
        sprintf(typeString, LIGHT_CYAN BOLD "%s" COLOR_RESET, PrimitiveDataTypeToString(dataType->container->primitive));
        break;

    case STRUCT_TYPE:
        sprintf(typeString, LIGHT_CYAN BOLD "%s" COLOR_RESET, dataType->container->custom.structDef->name);
        break;

    case CLASS_TYPE:
        sprintf(typeString, LIGHT_CYAN BOLD "%s" COLOR_RESET, dataType->container->custom.classDef->name);
        break;

    case FUNCTION_TYPE:
    {
        // (param1Type, param2Type, ...) → returnType
        sprintf(typeString, LIGHT_CYAN BOLD "(" COLOR_RESET);
        int paramCount = dataType->container->custom.funcDef->paramCount;
        for (int i = 0; i < paramCount; i++)
        {
            DataType *paramType = dataType->container->custom.funcDef->paramTypes[i];
            char *paramTypeStr = DataTypeToString(paramType);
            sprintf(typeString, "%s%s", typeString, paramTypeStr);
            if (i < dataType->container->custom.funcDef->paramCount - 1)
            {
                sprintf(typeString, "%s, ", typeString);
            }
        }
        if (paramCount == 0)
        {
            char *voidStr = (char *)malloc(16);
            if (!voidStr)
            {
                fprintf(stderr, "[DataTypes] Error: Failed to allocate memory for void string.\n");
                return NULL;
            }
            // Add void to the string and make it light cyan
            sprintf(voidStr, LIGHT_CYAN BOLD "void" COLOR_RESET);
            sprintf(typeString, "%s%s", typeString, voidStr);

            // Free the void string
            free(voidStr);
        }

        sprintf(typeString, "%s" LIGHT_CYAN BOLD ") → " COLOR_RESET, typeString);
        sprintf(typeString, "%s%s", typeString, DataTypeToString(dataType->container->custom.funcDef->returnType));
        // End with a color reset
        sprintf(typeString, "%s" COLOR_RESET, typeString);
        break;
    }
    default:
        sprintf(typeString, LIGHT_RED BOLD "<UNKNOWN>" COLOR_RESET);
        break;
    }

    return typeString;
}

char *VerboseStructTypeToString(StructType *type)
{
    if (!type)
        return "<NULL STRUCT>";

    char *typeString = (char *)malloc(128);
    if (!typeString)
    {
        fprintf(stderr, "[DataTypes] Error: Failed to allocate memory for type string.\n");
        return NULL;
    }

    sprintf(typeString, LIGHT_CYAN BOLD "%s" COLOR_RESET, (char *)type->name);

    if (type->propertyCount > 0)
    {
        for (int i = 0; i < type->propertyCount; i++)
        {
            ASTNode *property = type->properties[i];
            char *propType = DataTypeToString(property->data.property->type);
            sprintf(typeString, "%s %s: %s", typeString, property->data.property->name, propType);
        }
    }

    if (type->methodCount > 0)
    {
        for (int i = 0; i < type->methodCount; i++)
        {
            ASTNode *method = type->methods[i];
            char *methodType = DataTypeToString(method->data.method->type);
            sprintf(typeString, "%s %s  %s", typeString, method->data.method->name, methodType);
            if (method->data.method->paramCount > 0)
            {
                for (int j = 0; j < method->data.method->paramCount; j++)
                {
                    ASTNode *param = method->data.method->params[j];
                    char *paramType = DataTypeToString(param->data.param->type);
                    sprintf(typeString, "%s %s: %s", typeString, param->data.param->name, paramType);
                }
            }
        }
    }

    return typeString;
}

char *VerboseClassTypeToString(ClassType *type)
{
    if (!type)
        return "<NULL CLASS>";

    char *typeString = (char *)malloc(512);
    if (!typeString)
    {
        fprintf(stderr, "[DataTypes] Error: Failed to allocate memory for type string.\n");
        return NULL;
    }

    sprintf(typeString, LIGHT_CYAN BOLD "%s" COLOR_RESET " |", (char *)type->name);

    if (type->publicMembers->propertyCount > 0)
    {
        for (int i = 0; i < type->publicMembers->propertyCount; i++)
        {
            DataType *property = type->publicMembers->properties[i];
            const char *propertyType = DataTypeToString(property);
            sprintf(typeString, "%s %s: %s |", typeString, property->container->custom.name, propertyType);
        }
    }

    if (type->publicMembers->methodCount > 0)
    {
        for (int i = 0; i < type->publicMembers->methodCount; i++)
        {
            DataType *method = type->publicMembers->methods[i];
            const char *methodType = DataTypeToString(method);
            sprintf(typeString, "%s %s: %s", typeString, method->container->custom.name, methodType);
        }
    }

    return typeString;
}

char *VerboseFunctionTypeToString(FunctionType *type)
{
    if (!type)
        return "<NULL FUNCTION>";

    char *typeString = (char *)malloc(512);
    if (!typeString)
    {
        fprintf(stderr, "[DataTypes] Error: Failed to allocate memory for type string.\n");
        return NULL;
    }

    sprintf(typeString, LIGHT_CYAN BOLD "%s" COLOR_RESET, (char *)type->name);

    if (type->paramCount > 0)
    {
        for (int i = 0; i < type->paramCount; i++)
        {
            DataType *param = type->paramTypes[i];
            const char *paramType = DataTypeToString(param);
            sprintf(typeString, "%s %s: %s |", typeString, param->container->custom.name, paramType);
        }
    }

    // Add the return type
    const char *returnType = DataTypeToString(type->returnType);
    sprintf(typeString, "%s → %s", typeString, returnType);

    return typeString;
}

void logDataType(DataType *type)
{
    DEBUG_PRINT_FILTER({
        if (!type)
            return;

        printf(BOLD CYAN "───────────────────────────────────────────────────────────────\n" COLOR_RESET);
        printf(BOLD GREEN "   DATATYPE" COLOR_RESET " | Const: %s | Ref: %s\n", type->isConst ? "true" : "false", type->isReference ? "true" : "false");
        printTypeContainer(type->container);
        printf(BOLD CYAN "───────────────────────────────────────────────────────────────\n" COLOR_RESET);
        printf(COLOR_RESET);
    });
}

void logVerboseDataType(DataType *type)
{
    DEBUG_PRINT_FILTER({
        if (!type)
            return;

        printf(BOLD CYAN "───────────────────────────────────────────────────────────────\n" COLOR_RESET);
        printf(BOLD GREEN "   (v)DATATYPE" COLOR_RESET " | Const: %s | Ref: %s\n", type->isConst ? "true" : "false", type->isReference ? "true" : "false");
        printVerboseTypeContainer(type->container);
        printf(BOLD CYAN "───────────────────────────────────────────────────────────────\n" COLOR_RESET);
        printf(COLOR_RESET);
    });
}

void logStructType(StructType *type)
{
    DEBUG_PRINT_FILTER({
        if (!type)
        {
            printf("   ────────────────────────────────────────────────────────────\n");
            printf(BOLD GREEN "   STRUCT_TYPE" COLOR_RESET " | <NULL>\n");
            return;
        }

        printf("   ────────────────────────────────────────────────────────────\n");
        printf(BOLD GREEN "   STRUCT_TYPE" COLOR_RESET " | Size: %d | Prop Count: %d | Method Count: %d\n",
               type->size, type->propertyCount, type->methodCount);

        printf("   Name: %s | HDV: %s | Has Constructor: %s\n",
               type->name, type->hasDefaultValue ? "true" : "false",
               type->hasConstructor ? "true" : "false");

        printf(COLOR_RESET);
    });
}

void printFormattedStructType(StructType *type)
{
    DEBUG_PRINT_FILTER({
        printf("   ────────────────────────────────────────────────────────────\n");
        printf(BOLD GREEN "   STRUCT_TYPE" COLOR_RESET " | Size: %d | Prop Count: %d | Method Count: %d\n", type->size, type->propertyCount, type->methodCount);
        printf("   Name: %s | HDV: %s | Has Constructor: %s\n", type->name, type->hasDefaultValue ? "true" : "false", type->hasConstructor ? "true" : "false");

        if (type->propertyCount > 0)
        {
            printf("   Properties:\n");
            for (int i = 0; i < type->propertyCount; i++)
            {
                ASTNode *property = type->properties[i];
                printf("     %s: %s\n", property->data.property->name, DataTypeToString(property->data.property->type));
            }
        }

        if (type->methodCount > 0)
        {
            printf("   Methods:\n");
            for (int i = 0; i < type->methodCount; i++)
            {
                ASTNode *method = type->methods[i];
                printf("     %s →  %s\n", method->data.method->name, DataTypeToString(method->data.method->type));
                if (method->data.method->paramCount > 0)
                {
                    for (int j = 0; j < method->data.method->paramCount; j++)
                    {
                        ASTNode *param = method->data.method->params[j];
                        printf("       %s: %s\n", param->data.param->name, DataTypeToString(param->data.param->type));
                    }
                }
            }
        }

        printf(COLOR_RESET);
    });
}

void printFormattedPrimitiveType(PrimitiveDataType type)
{
    DEBUG_PRINT_FILTER({
        printf("   ────────────────────────────────────────────────────────────\n");
        printf(BOLD GREEN "   PRIMITIVE_TYPE" COLOR_RESET " | %s\n", PrimitiveDataTypeToString(type));
        printf(COLOR_RESET);
    });
}

void printFormattedType(DataType *type)
{
    DEBUG_PRINT_FILTER({
        if (!type)
            return;

        switch (type->container->baseType)
        {
        case PRIMITIVE_TYPE:
            if (type->container->isArray)
            {
                printf("   ────────────────────────────────────────────────────────────\n");
                printf(BOLD GREEN "   PRIMITIVE_TYPE" COLOR_RESET " | %s[%d]\n", PrimitiveDataTypeToString(type->container->primitive), type->container->arrayDimensions);
            }
            else
            {
                printFormattedPrimitiveType(type->container->primitive);
            }
            break;

        case STRUCT_TYPE:
            printFormattedStructType(type->container->custom.structDef);
            break;

        case CLASS_TYPE:
            printClassType(type->container->custom.classDef);
            break;

        case FUNCTION_TYPE:
            printFunctionType(type->container->custom.funcDef);

        default:
            printf("  ────────────────────────────────────────────────────────────\n");
            printf(BOLD GREEN "  UNKNOWN_TYPE" COLOR_RESET "\n");
            break;
        }

        printf(COLOR_RESET);
    });
}

void printTypeContainer(TypeContainer *type)
{
    if (!type)
        return;

    printf("Type: %s", TypeofDataTypeToString(type->baseType));

    if (type->isArray)
    {
        printf("[%d]", type->arrayDimensions);
    }

    switch (type->baseType)
    {
    case PRIMITIVE_TYPE:
        printf(" (%s)", PrimitiveDataTypeToString(type->primitive));
        break;

    case STRUCT_TYPE:
        printf(" (%s)", type->custom.structDef->name);
        break;

    case CLASS_TYPE:
        printf(" (%s)", type->custom.classDef->name);
        break;

    case FUNCTION_TYPE:
        printf(" %s", VerboseFunctionTypeToString(type->custom.funcDef));
        break;

    default:
        break;
    }
    printf("\n");
}

void printVerboseTypeContainer(TypeContainer *type)
{
    DEBUG_PRINT_FILTER({
        if (!type)
        {
            printf("   ────────────────────────────────────────────────────────────\n");
            printf(BOLD GREEN "   TYPE_CONTAINER" COLOR_RESET " | <NULL>\n");
            return;
        }

        printf("Type: %s", TypeofDataTypeToString(type->baseType));

        if (type->isArray)
        {
            printf("[%d]", type->arrayDimensions);
        }

        switch (type->baseType)
        {
        case PRIMITIVE_TYPE:
            printf(" (%s)", PrimitiveDataTypeToString(type->primitive));
            break;

        case STRUCT_TYPE:
            printf(" (%s)", VerboseStructTypeToString(type->custom.structDef));
            break;
        case CLASS_TYPE:
            printf(" (%s)", VerboseClassTypeToString(type->custom.classDef));
            break;
        case FUNCTION_TYPE:
            printf(" (%s)", VerboseFunctionTypeToString(type->custom.funcDef));
            break;
        default:
            printf(" <UNKNOWN>");
            break;
        }
        printf("\n");
    });
}

#define PRINT_MEMBERS_FN(TYPE)                                                                     \
    printf("\n");                                                                                  \
    if (type->TYPE##Members)                                                                       \
    {                                                                                              \
        printf("   " #TYPE " properties:\n");                                                      \
        for (int i = 0; i < type->TYPE##Members->propertyCount; i++)                               \
        {                                                                                          \
            DataType *property = type->TYPE##Members->properties[i];                               \
            printf("     %s: %s\n", property->container->custom.name, DataTypeToString(property)); \
        }                                                                                          \
        printf("   " #TYPE " methods:\n");                                                         \
        for (int i = 0; i < type->TYPE##Members->methodCount; i++)                                 \
        {                                                                                          \
            DataType *method = type->TYPE##Members->methods[i];                                    \
            printf("     %s: %s\n", method->container->custom.name, DataTypeToString(method));     \
        }                                                                                          \
    }
void printClassType(ClassType *type)
{
    DEBUG_PRINT_FILTER({
        if (!type)
        {
            printf("   ────────────────────────────────────────────────────────────\n");
            printf(BOLD GREEN "   CLASS_TYPE" COLOR_RESET " | <NULL>\n");
            return;
        }

        printf("   ────────────────────────────────────────────────────────────\n");
        printf(BOLD GREEN "   CLASS_TYPE" COLOR_RESET " | Size: %d | Prop Count: %d | Method Count: %d\n",
               sizeof(ClassType), type->propertyCount, type->methodCount);

        printf("   Name: %s | Static: %s | Has Constructor: %s\n",
               type->name, type->isStatic ? "true" : "false",
               type->hasConstructor ? "true" : "false");

        // Use the macro to print members for each access level
        PRINT_MEMBERS_FN(public);
        PRINT_MEMBERS_FN(private);
        PRINT_MEMBERS_FN(protected);

        printf(COLOR_RESET);
    });
}
#undef PRINT_MEMBERS_FN

void printFunctionType(FunctionType *funcType)
{
    // Make it look like a function signature (e.g., `function (int, float) -> string`)

    // (param1Type, param2Type, ...) → returnType
    char typeString[128];

    sprintf(typeString, LIGHT_CYAN BOLD "(" COLOR_RESET);
    for (int i = 0; i < funcType->paramCount; i++)
    {
        DataType *paramType = funcType->paramTypes[i];
        char *paramTypeStr = DataTypeToString(paramType);
        sprintf(typeString, "%s%s", typeString, paramTypeStr);
        if (i < funcType->paramCount - 1)
        {
            sprintf(typeString, "%s, ", typeString);
        }
    }
    sprintf(typeString, "%s" LIGHT_CYAN BOLD ") → " COLOR_RESET, typeString);
    sprintf(typeString, "%s%s", typeString, DataTypeToString(funcType->returnType));
    // End with a color reset
    sprintf(typeString, "%s" COLOR_RESET, typeString);

    printf(COLOR_RESET);
}

void printFunctionType_UF(FunctionType *funcType)
{
    // Make it look like a function signature (e.g., `function (int, float) -> string`)

    // (param1Type, param2Type, ...) → returnType
    char typeString[128];

    sprintf(typeString, "(");
    for (int i = 0; i < funcType->paramCount; i++)
    {
        DataType *paramType = funcType->paramTypes[i];
        char *paramTypeStr = DataTypeToString(paramType);
        sprintf(typeString, "%s%s", typeString, paramTypeStr);
        if (i < funcType->paramCount - 1)
        {
            sprintf(typeString, "%s, ", typeString);
        }
    }
    sprintf(typeString, "%s) → ", typeString);
    sprintf(typeString, "%s%s", typeString, DataTypeToString(funcType->returnType));

    printf(COLOR_RESET);
}

char *getFunctionTypeStr_UF(FunctionType *funcType)
{
    // Make it look like a function signature (e.g., `function (int, float) -> string`)

    // (param1Type, param2Type, ...) → returnType
    char typeString[128];

    sprintf(typeString, "(");
    for (int i = 0; i < funcType->paramCount; i++)
    {
        DataType *paramType = funcType->paramTypes[i];
        char *paramTypeStr = DataTypeToStringUnformatted(paramType);
        sprintf(typeString, "%s%s", typeString, paramTypeStr);
        if (i < funcType->paramCount - 1)
        {
            sprintf(typeString, "%s,", typeString);
        }
    }
    sprintf(typeString, "%s) -> ", typeString);
    sprintf(typeString, "%s%s", typeString, DataTypeToStringUnformatted(funcType->returnType));

    return strdup(typeString);
}

char *getFunctionArgTypeArrayStr(ASTNode *functionNode)
{
    if (!functionNode)
        return NULL;

    char *typeString = (char *)malloc(128);
    if (!typeString)
    {
        fprintf(stderr, "[DataTypes] Error: Failed to allocate memory for type string.\n");
        return NULL;
    }

    sprintf(typeString, "(");
    for (int i = 0; i < functionNode->data.functionDecl->paramCount; i++)
    {
        ASTNode *param = functionNode->data.functionDecl->params[i];
        char *paramTypeStr = DataTypeToStringUnformatted(param->data.param->type);
        sprintf(typeString, "%s%s", typeString, paramTypeStr);
        if (i < functionNode->data.functionDecl->paramCount - 1)
        {
            sprintf(typeString, "%s, ", typeString);
        }
    }
    sprintf(typeString, "%s)", typeString);

    return typeString;
}