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

bool typeAlreadyExists(TypeTable *table, const char *name)
{
    for (int i = 0; i < table->count; i++)
    {
        DataType *type = table->types[i];
        if (type->container->custom.name &&
            strcmp(type->container->custom.name, name) == 0)
        {
            return true;
        }
    }
    return false;
}

bool isValidType(DataType *type)
{
    if (!type)
    {
        logMessage("ERROR", __LINE__, "TypeTable", "Type is null");
        return false;
    }

    if (!type->container)
    {
        logMessage("ERROR", __LINE__, "TypeTable", "Type container is null");
        return false;
    }

    if (type->container->baseType == UNKNOWN_TYPE)
    {
        logMessage("ERROR", __LINE__, "TypeTable", "Base Type is unknown");
        return false;
    }

    if (type->container->baseType == PRIMITIVE_TYPE)
    {
        if (type->container->primitive == PRIM_UNKNOWN)
        {
            logMessage("ERROR", __LINE__, "TypeTable", "Primitive Type is unknown");
            return false;
        }
        if (type->container->primitive == PRIM_VOID)
        {
            logMessage("ERROR", __LINE__, "TypeTable", "Primitive Type is void");
            return false;
        }
        if (type->container->primitive == PRIM_NULL)
        {
            logMessage("ERROR", __LINE__, "TypeTable", "Primitive Type is null");
            return false;
        }

        logMessage("INFO", __LINE__, "TypeTable", "Primitive Type is valid: %s", DataTypeToString(type));
        return true;
    }

    if (type->container->baseType == STRUCT_TYPE)
    {
        if (!type->container->custom.structDef)
        {
            logMessage("ERROR", __LINE__, "TypeTable", "Struct Type is inaccesible");
            return false;
        }
        if (strcmp(type->container->custom.structDef->name, "<UNKNOWN>") == 0)
        {
            logMessage("ERROR", __LINE__, "TypeTable", "Struct Type is unknown");
            return false;
        }

        logMessage("INFO", __LINE__, "TypeTable", "Struct Type is valid: %s", DataTypeToString(type));
        return true;
    }

    if (type->container->baseType == FUNCTION_TYPE)
        return type->container->custom.funcDef != NULL;

    logMessage("INFO", __LINE__, "TypeTable", "Type is valid: %s", DataTypeToString(type));

    return true;
}

bool areTypesCompatible(TypeContainer *left, TypeContainer *right)
{
    if (!left || !right)
        return false;

    // Check base type match
    if (left->baseType != right->baseType)
        return false;

    // Check array dimensions
    if (left->isArray != right->isArray)
        return false;
    if (left->isArray && left->arrayDimensions != right->arrayDimensions)
        return false;

    // Check specific type details
    switch (left->baseType)
    {
    case PRIMITIVE_TYPE:
        return left->primitive == right->primitive;

    case STRUCT_TYPE:
        return strcmp(left->custom.name, right->custom.name) == 0;

    default:
        return false;
    }
}
