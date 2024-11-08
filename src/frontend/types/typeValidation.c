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
#include "frontend/typeTable.h"

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

bool isValidType(TypeContainer *type, TypeTable *typeTable)
{
    if (!type)
        return false;

    switch (type->baseType)
    {
    case PRIMITIVE_TYPE:
        return type->primitive != PRIM_UNKNOWN;

    case STRUCT_TYPE:
        return type->custom.structDef != NULL;

    case FUNCTION_TYPE:
        return type->custom.funcDef != NULL;

    default:
        return false;
    }
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
