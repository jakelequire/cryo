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

DataType *DTMParsePrimitive(const char *typeStr)
{
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
        // Do not fail, just return NULL
        logMessage(LMI, "WARN", "DTM", "Type '%s' is not a primitive type", typeStr);
        DTM->symbolTable->printTable(DTM->symbolTable);
        return NULL;
    }
    return NULL;
}

// Forward declaration for the array parsing function
DataType *DTMParseArrayType(const char *typeStr, int *pos);

bool isArrayType(const char *typeStr)
{
    // Check if the type string contains an array declaration
    return (strchr(typeStr, '[') != NULL);
}

// Helper function to parse array types
DataType *DTMParseArrayType(const char *__typeStr__, int *pos)
{
    char *typeStr = (char *)strdup(__typeStr__);
    char baseTypeBuf[256] = {0};
    int i = 0;

    // Extract the base type (everything before '[')
    while (typeStr[i] && typeStr[i] != '[')
    {
        baseTypeBuf[i] = typeStr[i];
        i++;
    }
    baseTypeBuf[i] = '\0';

    // If no '[' was found, it's not an array type
    if (!typeStr[i])
    {
        return NULL;
    }

    logMessage(LMI, "INFO", "DTM", "Parsing array type '%s'", baseTypeBuf);

    // Parse the base type
    DataType *baseType = DTMParsePrimitive(baseTypeBuf);
    if (!baseType)
    {
        // Try to look it up in the symbol table
        baseType = DTM->symbolTable->lookup(DTM->symbolTable, baseTypeBuf);
        if (!baseType)
        {
            logMessage(LMI, "ERROR", "DTM", "Failed to parse base type '%s'", baseTypeBuf);
            return NULL;
        }
    }

    logMessage(LMI, "INFO", "DTM", "Parsed base type '%s'", baseType->typeName);
    // Now we're at '[', move past it
    i++; // Skip '['

    // Check if it's a dynamic array '[]'
    if (typeStr[i] == ']')
    {
        logMessage(LMI, "INFO", "DTM", "Parsing dynamic array type '%s'", baseType->typeName);
        // Dynamic array type
        DataType *arrayType = DTM->arrayTypes->createMonomorphicArrayDecl(baseType);

        // Update position to after ']'
        logMessage(LMI, "INFO", "DTM", "Parsed dynamic array type '%s'", arrayType->typeName);
        return arrayType;
    }

    // It's a static array or an array with a type as the size specifier
    // Check if it's a numeric size
    if (isdigit(typeStr[i]))
    {
        logMessage(LMI, "INFO", "DTM", "Parsing static array type '%s'", baseType->typeName);
        // Parse the size
        char sizeBuf[64] = {0};
        int j = 0;
        while (typeStr[i] && typeStr[i] != ']')
        {
            sizeBuf[j++] = typeStr[i++];
        }
        sizeBuf[j] = '\0';

        if (typeStr[i] != ']')
        {
            logMessage(LMI, "ERROR", "DTM", "Expected ']' after array size");
            return NULL;
        }

        // Convert size to integer
        int size = atoi(sizeBuf);
        if (size <= 0)
        {
            logMessage(LMI, "ERROR", "DTM", "Invalid array size: %d", size);
            return NULL;
        }

        // Create static array type
        DataType *arrayType = DTM->dataTypes->createArrayType(baseType, size);
        arrayType->isArray = true;
        arrayType->container->type.arrayType->isDynamic = false;
        arrayType->container->type.arrayType->size = size;

        // Update position to after ']'
        *pos = i + 1;
        logMessage(LMI, "INFO", "DTM", "Parsed static array type '%s' with size %d", arrayType->typeName, size);
        return arrayType;
    }
    else
    {
        logMessage(LMI, "INFO", "DTM", "Parsing array type with type-based size '%s'", baseType->typeName);
        // This is a complex type specification like int[int[]]
        // Recursively parse the type inside brackets
        int nestedPos = 0;
        DataType *sizeType = DTMParseType(typeStr + i);

        // Skip past the nested type
        while (typeStr[i] && typeStr[i] != ']')
        {
            i++;
        }

        if (typeStr[i] != ']')
        {
            logMessage(LMI, "ERROR", "DTM", "Expected ']' after array size specification");
            return NULL;
        }

        // Create array type with type-based size
        DataType *arrayType = DTM->dataTypes->createArrayType(baseType, 1); // Default size 1
        arrayType->isArray = true;
        arrayType->container->type.arrayType->isDynamic = false;
        arrayType->container->type.arrayType->sizeType = sizeType; // Store the type that specifies the size

        // Update position to after ']'
        *pos = i + 1;
        logMessage(LMI, "INFO", "DTM", "Parsed array type with type-based size '%s'", arrayType->typeName);
        return arrayType;
    }
}

// Updated main type parsing function
DataType *DTMParseType(const char *typeStr)
{
    logMessage(LMI, "INFO", "DTM", "Parsing type string '%s'", typeStr);
    if (!typeStr)
    {
        fprintf(stderr, "[Data Type Manager] Error: Attempted to parse NULL type string\n");
        return NULL;
    }

    // First check if it's an array type
    int pos = 0;
    DataType *arrayType = DTMParseArrayType(typeStr, &pos);
    if (arrayType)
    {
        // If we found an array type, but there's more string after it,
        // we might need to handle additional array dimensions
        if (typeStr[pos] == '[')
        {
            // Handle multi-dimensional arrays
            DataType *currentType = arrayType;
            while (typeStr[pos] == '[')
            {
                int subPos = pos;
                DataType *subArrayType = DTMParseArrayType(typeStr + pos - 1, &subPos);
                if (!subArrayType)
                {
                    logMessage(LMI, "ERROR", "DTM", "Failed to parse multi-dimensional array");
                    return NULL;
                }
                // Update the base type of subArrayType to be the previously parsed arrayType
                subArrayType->container->type.arrayType->baseType = currentType;
                currentType = subArrayType;
                pos += subPos;
            }
            return currentType;
        }
        return arrayType;
    }

    // If it's not an array type, try as a primitive
    DataType *primitive = DTMParsePrimitive(typeStr);
    if (primitive != NULL)
    {
        logMessage(LMI, "INFO", "DTM", "Parsed primitive type '%s'", typeStr);
        return primitive;
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

    if (isArrayType(typeStr))
    {
        // If it's an array type, parse it
        DataType *arrayType = DTMParseArrayType(typeStr, NULL);
        if (arrayType != NULL)
        {
            return arrayType;
        }
    }

    // We will look up the string directly in the symbol table
    DataType *type = self->symbolTable->lookup(self->symbolTable, typeStr);
    if (type != NULL)
    {
        return type;
    }

    DataType *primitive = DTMParseType(typeStr);
    if (primitive != NULL)
    {
        return primitive;
    }

    // If we reach here, the type is not a primitive or a symbol table entry
    logMessage(LMI, "ERROR", "DTM", "Failed to resolve type string '%s'", typeStr);
    return NULL;
}
