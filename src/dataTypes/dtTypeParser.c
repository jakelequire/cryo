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

DataType *DTMParseType(const char *typeStr)
{
    logMessage(LMI, "INFO", "DTM", "Parsing type string '%s'", typeStr);
    if (!typeStr)
    {
        fprintf(stderr, "[Data Type Manager] Error: Attempted to parse NULL type string\n");
        CONDITION_FAILED;
    }

    // Check if the type string is a primitive type
    // if (strcmp(typeStr, "int") == 0)
    // {
    //     DataType *int_ty = DTM->primitives->createInt();
    //     return int_ty;
    // }
    if (strcmp(typeStr, "float") == 0)
    {
        DataType *float_ty = DTM->primitives->createFloat();
        return float_ty;
    }
    else if (strcmp(typeStr, "char") == 0)
    {
        DataType *char_ty = DTM->primitives->createChar();
        return char_ty;
    }
    else if (strcmp(typeStr, "str") == 0)
    {
        DataType *str_ty = DTM->primitives->createStr();
        return str_ty;
    }
    else if (strcmp(typeStr, "boolean") == 0)
    {
        DataType *bool_ty = DTM->primitives->createBoolean();
        return bool_ty;
    }
    else if (strcmp(typeStr, "void") == 0)
    {
        DataType *void_ty = DTM->primitives->createVoid();
        DTM->debug->printDataType(void_ty);
        return void_ty;
    }
    else if (strcmp(typeStr, "null") == 0)
    {
        DataType *null_ty = DTM->primitives->createNull();
        return null_ty;
    }
    else if (strcmp(typeStr, "any") == 0)
    {
        DataType *any_ty = DTM->primitives->createAny();
        return any_ty;
    }
    else if (strcmp(typeStr, "undefined") == 0)
    {
        DataType *undefined_ty = DTM->primitives->createUndefined();
        return undefined_ty;
    }
    else if (strcmp(typeStr, "auto") == 0)
    {
        DataType *auto_ty = DTM->primitives->createAutoTy();
        return auto_ty;
    }
    else if (strcmp(typeStr, "i8") == 0)
    {
        DataType *i8_ty = DTM->primitives->createI8();
        return i8_ty;
    }
    else if (strcmp(typeStr, "i16") == 0)
    {
        DataType *i16_ty = DTM->primitives->createI16();
        return i16_ty;
    }
    else if (strcmp(typeStr, "i32") == 0)
    {
        DataType *i32_ty = DTM->primitives->createI32();
        return i32_ty;
    }
    else if (strcmp(typeStr, "i64") == 0)
    {
        DataType *i64_ty = DTM->primitives->createI64();
        return i64_ty;
    }
    else if (strcmp(typeStr, "i128") == 0)
    {
        DataType *i128_ty = DTM->primitives->createI128();
        return i128_ty;
    }
    else
    {
        // Attempt to lookup the type in the symbol table
        logMessage(LMI, "INFO", "DTM", "Looking up type string '%s' in symbol table", typeStr);
        DataType *type = DTM->symbolTable->lookup(DTM->symbolTable, typeStr);
        if (type)
        {
            return type;
        }
        // If the type is not found in the symbol table, return NULL
        logMessage(LMI, "ERROR", "DTM", "Failed to parse type string '%s'", typeStr);
        fprintf(stderr, "[Data Type Manager] Error: Failed to parse type string '%s'\n", typeStr);
        DTM->symbolTable->printTable(DTM->symbolTable);
        CONDITION_FAILED;
        return NULL;
    }

    return NULL;
}

DataType *DTMResolveType(DataTypeManager *self, const char *typeStr)
{
    logMessage(LMI, "INFO", "DTM", "Resolving type string '%s'", typeStr);
    if (!typeStr)
    {
        fprintf(stderr, "[Data Type Manager] Error: Attempted to resolve NULL type string\n");
        CONDITION_FAILED;
    }

    // We will look up the string directly in the symbol table
    DataType *type = self->symbolTable->lookup(self->symbolTable, typeStr);
    if (type)
    {
        return type;
    }

    DataType *primitive = DTMParseType(typeStr);
    if (primitive)
    {
        return primitive;
    }

    // If we reach here, the type is not a primitive or a symbol table entry
    logMessage(LMI, "ERROR", "DTM", "Failed to resolve type string '%s'", typeStr);
    CONDITION_FAILED;

    return NULL;
}
