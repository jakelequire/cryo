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
#include "dataTypes/compilerDefs.h"
#include "dataTypes/dataTypeManager.h"

DataType *create_VA_ARGS(void)
{
    // Create struct type for VA_ARGS
    DTStructTy *va_args_struct = createDTStructTy();
    va_args_struct->name = "VA_ARGS";

    // Create the properties

    logMessage(LMI, "INFO", "CompilerDefs", "Creating VA_ARGS `count` property");
    // 1. Create count property (i32)
    DTPropertyTy *count_prop = DTM->propertyTypes->createPropertyType(
        "count",
        DTM->primitives->createI32(),
        NULL,  // No AST node
        false, // Not static
        false, // Not const
        true,  // Public
        false, // Not private
        false  // Not protected
    );

    logMessage(LMI, "INFO", "CompilerDefs", "Creating VA_ARGS `types` property");
    // 2. Create types property (pointer to array of type descriptors)
    DataType *types_type = DTM->primitives->createPointer();
    types_type->setTypeName(types_type, "types");
    DTPropertyTy *types_prop = DTM->propertyTypes->createPropertyType(
        "types",
        types_type,
        NULL,  // No AST node
        false, // Not static
        false, // Not const
        true,  // Public
        false, // Not private
        false  // Not protected
    );

    logMessage(LMI, "INFO", "CompilerDefs", "Creating VA_ARGS `values` property");
    // 3. Create values property (pointer to actual argument values)
    DataType *values_type = DTM->primitives->createPointer();
    values_type->setTypeName(values_type, "values");
    DTPropertyTy *values_prop = DTM->propertyTypes->createPropertyType(
        "values",
        values_type,
        NULL,  // No AST node
        false, // Not static
        false, // Not const
        true,  // Public
        false, // Not private
        false  // Not protected
    );

    // Add properties to the struct
    va_args_struct->addProperty(va_args_struct, count_prop);
    va_args_struct->addProperty(va_args_struct, types_prop);
    va_args_struct->addProperty(va_args_struct, values_prop);

    // Create methods for the VA_ARGS type

    logMessage(LMI, "INFO", "CompilerDefs", "Creating VA_ARGS `getInt` method");
    // 1. getInt method - retrieves an int argument by index
    DataType *getInt_return = DTM->primitives->createI32();
    DataType *getInt_param = DTM->primitives->createI32(); // index parameter
    DataType **getInt_params = (DataType **)malloc(sizeof(DataType *));
    getInt_params[0] = getInt_param;
    DataType *getInt_method = DTM->functionTypes->createMethodType(
        "getInt",
        getInt_return,
        getInt_params,
        1 // param count
    );
    va_args_struct->addMethod(va_args_struct, getInt_method);

    logMessage(LMI, "INFO", "CompilerDefs", "Creating VA_ARGS `getFloat` method");
    // 2. getFloat method
    DataType *getFloat_return = DTM->primitives->createFloat();
    DataType *getFloat_param = DTM->primitives->createI32(); // index parameter
    DataType **getFloat_params = (DataType **)malloc(sizeof(DataType *));
    getFloat_params[0] = getFloat_param;
    DataType *getFloat_method = DTM->functionTypes->createMethodType(
        "getFloat",
        getFloat_return,
        getFloat_params,
        1 // param count
    );
    va_args_struct->addMethod(va_args_struct, getFloat_method);

    logMessage(LMI, "INFO", "CompilerDefs", "Creating VA_ARGS `getString` method");
    // 3. getString method
    DataType *getString_return = DTM->primitives->createString();
    DataType *getString_param = DTM->primitives->createI32(); // index parameter
    DataType **getString_params = (DataType **)malloc(sizeof(DataType *));
    getString_params[0] = getString_param;
    DataType *getString_method = DTM->functionTypes->createMethodType(
        "getString",
        getString_return,
        getString_params,
        1 // param count
    );
    va_args_struct->addMethod(va_args_struct, getString_method);

    logMessage(LMI, "INFO", "CompilerDefs", "Creating VA_ARGS `getPointer` method");
    // 4. getPointer method
    DataType *getPointer_return = DTM->primitives->createPointer();
    DataType *getPointer_param = DTM->primitives->createI32(); // index parameter
    DataType **getPointer_params = (DataType **)malloc(sizeof(DataType *));
    getPointer_params[0] = getPointer_param;
    DataType *getPointer_method = DTM->functionTypes->createMethodType(
        "getPointer",
        getPointer_return,
        getPointer_params,
        1 // param count
    );
    va_args_struct->addMethod(va_args_struct, getPointer_method);

    logMessage(LMI, "INFO", "CompilerDefs", "Creating VA_ARGS `getAny` method");
    // 5. getAny method
    DataType *getAny_return = DTM->primitives->createAny();
    DataType *getAny_param = DTM->primitives->createI32(); // index parameter
    DataType **getAny_params = (DataType **)malloc(sizeof(DataType *));
    getAny_params[0] = getAny_param;
    DataType *getAny_method = DTM->functionTypes->createMethodType(
        "getAny",
        getAny_return,
        getAny_params,
        1 // param count
    );
    va_args_struct->addMethod(va_args_struct, getAny_method);

    // Type checking methods

    logMessage(LMI, "INFO", "CompilerDefs", "Creating VA_ARGS `isInt` method");
    // 6. isInt method
    DataType *isInt_return = DTM->primitives->createBoolean();
    DataType *isInt_param = DTM->primitives->createI32(); // index parameter
    DataType **isInt_params = (DataType **)malloc(sizeof(DataType *));
    isInt_params[0] = isInt_param;
    DataType *isInt_method = DTM->functionTypes->createMethodType(
        "isInt",
        isInt_return,
        isInt_params,
        1 // param count
    );
    va_args_struct->addMethod(va_args_struct, isInt_method);

    logMessage(LMI, "INFO", "CompilerDefs", "Creating VA_ARGS `isFloat` method");
    // 7. isFloat method
    DataType *isFloat_return = DTM->primitives->createBoolean();
    DataType *isFloat_param = DTM->primitives->createI32(); // index parameter
    DataType **isFloat_params = (DataType **)malloc(sizeof(DataType *));
    isFloat_params[0] = isFloat_param;
    DataType *isFloat_method = DTM->functionTypes->createMethodType(
        "isFloat",
        isFloat_return,
        isFloat_params,
        1 // param count
    );
    va_args_struct->addMethod(va_args_struct, isFloat_method);

    logMessage(LMI, "INFO", "CompilerDefs", "Creating VA_ARGS `isString` method");
    // 8. isString method
    DataType *isString_return = DTM->primitives->createBoolean();
    DataType *isString_param = DTM->primitives->createI32(); // index parameter
    DataType **isString_params = (DataType **)malloc(sizeof(DataType *));
    isString_params[0] = isString_param;
    DataType *isString_method = DTM->functionTypes->createMethodType(
        "isString",
        isString_return,
        isString_params,
        1 // param count
    );
    va_args_struct->addMethod(va_args_struct, isString_method);

    logMessage(LMI, "INFO", "CompilerDefs", "Creating VA_ARGS `isPointer` method");
    // 9. isPointer method
    DataType *isPointer_return = DTM->primitives->createBoolean();
    DataType *isPointer_param = DTM->primitives->createI32(); // index parameter
    DataType **isPointer_params = (DataType **)malloc(sizeof(DataType *));
    isPointer_params[0] = isPointer_param;
    DataType *isPointer_method = DTM->functionTypes->createMethodType(
        "isPointer",
        isPointer_return,
        isPointer_params,
        1 // param count
    );
    va_args_struct->addMethod(va_args_struct, isPointer_method);

    logMessage(LMI, "INFO", "CompilerDefs", "Wrapping VA_ARGS struct type");
    // Create the final DataType wrapper
    DataType *va_args = DTM->dataTypes->wrapStructType(va_args_struct);
    va_args->container->typeOf = OBJECT_TYPE;
    va_args->container->objectType = VA_ARGS_OBJ;
    va_args->container->primitive = PRIM_OBJECT;
    va_args->setTypeName(va_args, "VA_ARGS");

    return va_args;
}

// Function to register in CompilerDefs struct
void register_VA_ARGS_type(void)
{
    DTM->symbolTable->addEntry(DTM->symbolTable, "global", "VA_ARGS", DTM->compilerDefs->create_VA_ARGS());
    logMessage(LMI, "INFO", "CompilerDefs", "Registered VA_ARGS type");
}
