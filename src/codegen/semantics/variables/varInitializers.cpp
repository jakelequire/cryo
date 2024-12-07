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
#include "codegen/oldCodeGen.hpp"

namespace Cryo
{
    /// ### ============================================================================= ###
    /// ###
    /// ### Specialized Functions
    /// ### These functions are used to create variables with specific initializers
    /// ###
    /// ### ============================================================================= ###

    llvm::Value *Variables::createPropertyAccessVariable(PropertyAccessNode *propAccessNode, std::string varName, DataType *varType)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Variables", "Creating Property Access Variable");

        std::string propertyName = std::string(propAccessNode->propertyName);
        ASTNode *objNode = propAccessNode->object;
        DataType *objType = nullptr;
        logASTNode(objNode);

        std::string accessorName;
        if (objNode->metaData->type == NODE_VAR_DECLARATION)
        {
            CryoVariableNode *varNode = objNode->data.varDecl;
            accessorName = std::string(varNode->name);
            objType = varNode->type;
        }
        else
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables",
                                    "Accessor node is not a variable declaration");
            std::string nodeTypeStr = CryoNodeTypeToString(objNode->metaData->type);
            DevDebugger::logMessage("ERROR", __LINE__, "Variables",
                                    "Node Type: " + nodeTypeStr);
            CONDITION_FAILED;
        }

        llvm::Value *accessorValue = compiler.getContext().namedValues[accessorName];
        if (!accessorValue)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables",
                                    "Accessor value not found: " + accessorName);
            CONDITION_FAILED;
        }

        int propIndex = propAccessNode->propertyIndex;
        std::cout << "Property Index: " << propIndex << std::endl;

        llvm::Type *objIRType = compiler.getTypes().getType(objType, 0);
        std::string structTypeName = objType->container->custom.structDef->name;
        llvm::StructType *structType = compiler.getContext().getStruct(structTypeName);
        if (!structType)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions",
                                    "Struct type not found");
            CONDITION_FAILED;
        }

        // Get the property
        llvm::Value *propertyValue = compiler.getContext().builder.CreateStructGEP(
            objIRType,
            accessorValue,
            propIndex,
            varName + ".prop");

        llvm::Value *loadedValue = compiler.getContext().builder.CreateLoad(
            structType->getElementType(propIndex),
            propertyValue,
            varName + ".load");

        // Register in symbol table
        compiler.getContext().namedValues[varName] = propertyValue;
        compiler.getSymTable().updateVariableNode(
            compiler.getContext().currentNamespace,
            varName,
            loadedValue,
            propertyValue->getType());

        // Return the variable
        return propertyValue;
    }

    llvm::Value *Variables::createMethodCallVariable(MethodCallNode *methodCall, std::string varName, DataType *varType)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Variables", "Creating Method Call Variable");

        std::string methodDefName = std::string(methodCall->name);
        std::string namespaceName = compiler.getContext().currentNamespace;
        DataType *instanceType = methodCall->instanceType;

        std::string instanceName = std::string(methodCall->instanceName);
        DataType *returnType = methodCall->returnType;
        logDataType(instanceType);

        std::cout << "\nReturn Type: " << std::endl;
        logDataType(returnType);

        ASTNode *accessorNode = methodCall->accessorObj;
        logASTNode(accessorNode);

        std::string accessorName;
        if (accessorNode->metaData->type == NODE_VAR_DECLARATION)
        {
            CryoVariableNode *varNode = accessorNode->data.varDecl;
            accessorName = std::string(varNode->name);
        }
        else
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables",
                                    "Accessor node is not a variable declaration");
            std::string nodeTypeStr = CryoNodeTypeToString(accessorNode->metaData->type);
            DevDebugger::logMessage("ERROR", __LINE__, "Variables",
                                    "Node Type: " + nodeTypeStr);
            CONDITION_FAILED;
        }
        std::cout << "Method Def Name: " << methodDefName << std::endl;
        std::cout << "Instance Name: " << instanceName << std::endl;
        std::cout << "Accessor Name: " << accessorName << std::endl;

        llvm::Value *accessorValue = compiler.getContext().namedValues[accessorName];
        if (!accessorValue)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables",
                                    "Accessor value not found: " + accessorName);
            CONDITION_FAILED;
        }

        // Combine instance name and method name to get the function name
        std::string methodName = instanceName + "." + methodDefName;

        // Get the function
        llvm::Function *function = compiler.getContext().module->getFunction(methodName);
        if (!function)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables",
                                    "Function not found: " + methodName);
            CONDITION_FAILED;
        }

        // initialize the variable
        llvm::Type *varIRType = compiler.getTypes().getType(varType, 0);
        llvm::Value *variablePtr = compiler.getContext().builder.CreateAlloca(
            varIRType,
            nullptr,
            varName + ".ptr");
        if (!variablePtr)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables",
                                    "Failed to allocate memory for variable: " + varName);
            CONDITION_FAILED;
        }

        // Set the accessor object as the first argument
        std::vector<llvm::Value *> args = {accessorValue};
        // Get the arguments (after the first argument, which is the instance)
        int argCount = methodCall->argCount;
        for (int i = 0; i < argCount; ++i)
        {
            ASTNode *argNode = methodCall->args[i];
            llvm::Value *argValue = compiler.getGenerator().getInitilizerValue(argNode);
            if (!argValue)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "Variables",
                                        "Argument value not found");
                CONDITION_FAILED;
            }
            args.push_back(argValue);
        }

        // Call the function
        llvm::Value *functionCall = compiler.getContext().builder.CreateCall(function, args);
        if (!functionCall)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables",
                                    "Function call not created");
            CONDITION_FAILED;
        }

        // Store the return value in the variable
        llvm::StoreInst *storeInst = compiler.getContext().builder.CreateStore(functionCall, variablePtr);
        if (!storeInst)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables",
                                    "Failed to store function call value in variable");
            CONDITION_FAILED;
        }

        std::cout << "\n\n === Variable Created === \n\n";
        DevDebugger::logLLVMValue(variablePtr);
        DevDebugger::logLLVMValue(functionCall);
        std::cout << "\n\n === Variable Created === \n\n";

        // Register in symbol table
        compiler.getContext().namedValues[varName] = variablePtr;
        compiler.getSymTable().updateVariableNode(
            namespaceName,
            varName,
            variablePtr,
            variablePtr->getType());
        compiler.getSymTable().addFunctionToVar(namespaceName, varName, functionCall);

        // Return the variable
        return variablePtr;
    }

    llvm::Value *Variables::createStructVariable(ASTNode *varDecl)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Variables", "Creating Struct Variable");
        CryoVariableNode *varNode = varDecl->data.varDecl;
        std::string varName = std::string(varNode->name);
        std::string namespaceName = compiler.getContext().currentNamespace;

        DevDebugger::logMessage("INFO", __LINE__, "Variables", "Variable Name: " + varName);

        // Create struct instance using the dedicated method
        Structs &structs = compiler.getStructs();
        llvm::Value *structPtr = structs.createStructInstance(varDecl);

        // Register in symbol table
        compiler.getContext().namedValues[varName] = structPtr;
        compiler.getSymTable().updateVariableNode(
            namespaceName,
            varName,
            structPtr,
            compiler.getContext().structTypes[varNode->type->container->custom.structDef->name]);
        compiler.getSymTable().addDataTypeToVar(namespaceName, varName, varNode->type);

        return structPtr;
    }

    llvm::Value *Variables::getStructFieldValue(const std::string &structVarName,
                                                const std::string &fieldName)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Variables",
                                "Getting struct field: " + structVarName + "." + fieldName);

        std::string namespaceName = compiler.getContext().currentNamespace;

        // Get the struct variable
        STVariable *var = compiler.getSymTable().getVariable(namespaceName, structVarName);
        if (!var)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables",
                                    "Struct variable not found: " + structVarName);
            CONDITION_FAILED;
        }

        llvm::Value *structPtr = var->LLVMValue;
        llvm::StructType *structType = llvm::dyn_cast<llvm::StructType>(structPtr->getType());

        if (!structType)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables",
                                    "Not a struct type");
            CONDITION_FAILED;
        }

        // Get the struct definition
        STStruct *structDef = compiler.getSymTable().getStruct(
            namespaceName,
            structType->getName().str());

        // Find field index
        StructType *structDataType = structDef->structType->container->custom.structDef;
        int fieldIndex = -1;
        for (int i = 0; i < structDataType->propertyCount; ++i)
        {
            PropertyNode *prop = structDataType->properties[i]->data.property;
            if (std::string(prop->name) == fieldName)
            {
                fieldIndex = i;
                break;
            }
        }

        if (fieldIndex == -1)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables",
                                    "Field not found: " + fieldName);
            CONDITION_FAILED;
        }

        // Create GEP for field access
        llvm::Value *fieldPtr = compiler.getContext().builder.CreateStructGEP(
            structType,
            structPtr,
            fieldIndex,
            structVarName + "." + fieldName);

        // Load and return field value
        return compiler.getContext().builder.CreateLoad(
            structType->getElementType(fieldIndex),
            fieldPtr,
            structVarName + "." + fieldName + ".load");
    }

    ///
    /// @brief Create a variable with a literal expression initializer
    /// @param literalNode The literal node
    /// @param varName The name of the variable
    /// @return `llvm::Value *` The LLVM value of the variable
    ///
    /// ---
    ///
    /// ```cryo
    ///
    /// \// Example
    ///
    /// const foo: int = 10;
    ///
    /// const bar: string = "Hello, World!";
    ///
    /// ```
    llvm::Value *Variables::createLiteralExprVariable(LiteralNode *literalNode, std::string varName, DataType *type)
    {
        IRSymTable &symTable = compiler.getSymTable();
        OldTypes &types = compiler.getTypes();
        DevDebugger::logMessage("INFO", __LINE__, "Variables", "Creating Literal Expression Variable");

        std::string namespaceName = compiler.getContext().currentNamespace;
        llvm::Value *llvmValue = nullptr;
        llvm::Type *llvmType = nullptr;
        llvm::Constant *llvmConstant = nullptr;

        DataType *dataType = literalNode->type;
        switch (dataType->container->primitive)
        {
        case PRIM_INT:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Variables", "Variable is an int literal");
            int intValue = literalNode->value.intValue;
            llvm::Type *ty = compiler.getTypes().getType(type, 0);
            llvm::Value *varValue = compiler.getGenerator().getLiteralValue(literalNode);
            if (!varValue)
            {
                CONDITION_FAILED;
            }
            llvm::AllocaInst *ptrValue = compiler.getContext().builder.CreateAlloca(ty, nullptr, varName + ".ptr");
            ptrValue->setAlignment(llvm::Align(8));
            llvm::StoreInst *storeInst = compiler.getContext().builder.CreateStore(varValue, ptrValue);
            storeInst->setAlignment(llvm::Align(8));

            compiler.getContext().namedValues[varName] = ptrValue;

            symTable.updateVariableNode(namespaceName, varName, ptrValue, ty);
            symTable.addStoreInstToVar(namespaceName, varName, storeInst);

            return ptrValue;
        }
        case PRIM_FLOAT:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Variables", "Creating Float Variable");
            llvmType = types.getType(dataType, 0);
            llvmConstant = llvm::ConstantFP::get(llvmType, literalNode->value.floatValue);
            llvmValue = llvm::dyn_cast<llvm::Value>(llvmConstant);
            break;
        }
        case PRIM_BOOLEAN:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Variables", "Creating Boolean Variable");
            llvmType = types.getType(dataType, 0);
            llvmConstant = llvm::ConstantInt::get(llvmType, literalNode->value.booleanValue);
            llvmValue = llvm::dyn_cast<llvm::Value>(llvmConstant);
            break;
        }
        case PRIM_STRING:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Variables", "Variable is a string literal");
            int _len = compiler.getTypes().getLiteralValLength(literalNode) + 1;
            llvmType = compiler.getTypes().getType(dataType, _len);
            DevDebugger::logMessage("INFO", __LINE__, "Variables", "Type: " + std::string(DataTypeToString(dataType)));

            llvm::Value *varValue = compiler.getGenerator().getLiteralValue(literalNode);
            if (!varValue)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Variable value not found");
                CONDITION_FAILED;
            }
            llvmValue = compiler.getContext().builder.CreateAlloca(llvmType, nullptr, varName);
            llvm::StoreInst *storeInst = compiler.getContext().builder.CreateStore(varValue, llvmValue);
            storeInst->setAlignment(llvm::Align(8));

            compiler.getContext().namedValues[varName] = llvmValue;

            llvm::Type *strType = types.getType(createPrimitiveStringType(_len), _len);
            symTable.updateVariableNode(namespaceName, varName, llvmValue, strType);
            symTable.addStoreInstToVar(namespaceName, varName, storeInst);

            return llvmValue;
        }
        default:
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Unknown data type");
            CONDITION_FAILED;
        }
        }

        return llvmValue;
    }

    ///
    /// @brief Create a variable with a variable name / reference initializer
    /// @param node The AST node
    /// @param varName The name of the variable
    /// @param refVarName The name of the variable being referenced
    /// @return `llvm::Value *` The LLVM value of the variable
    ///
    /// ---
    ///
    /// ```cryo
    ///
    /// \// Example
    ///
    /// const foo: int = 34;
    ///
    /// const bar: int = foo;
    ///
    /// ```
    ///
    llvm::Value *Variables::createVarNameInitializer(VariableNameNode *varNameNode, std::string varName, std::string refVarName)
    {
        IRSymTable &symTable = compiler.getSymTable();
        OldTypes &types = compiler.getTypes();
        std::string namespaceName = compiler.getContext().currentNamespace;
        DevDebugger::logMessage("INFO", __LINE__, "Variables", "Creating Variable Name Initializer");
        std::cout << "Variable Name: " << varName << std::endl;
        std::cout << "Referenced Variable Name: " << refVarName << std::endl;

        DataType *nodeDataType = varNameNode->type;
        std::cout << "Node Type: " << DataTypeToString(nodeDataType) << std::endl;

        // Create the variable alloca
        llvm::Value *llvmValue = nullptr;
        llvm::Type *llvmType = nullptr;

        switch (nodeDataType->container->baseType)
        {
        case PRIMITIVE_TYPE:
        {
            switch (nodeDataType->container->primitive)
            {
            case PRIM_INT:
            {
                DevDebugger::logMessage("INFO", __LINE__, "Variables", "Creating Int Variable");
                std::cout << "Variable Name (Int): " << varName << std::endl;
                llvmType = types.getType(nodeDataType, 0);

                STVariable *var = symTable.getVariable(namespaceName, refVarName);
                if (!var)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Variable not found");
                    CONDITION_FAILED;
                }
                llvm::Value *stValue = var->LLVMValue;
                if (!stValue)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Variable value not found");
                    compiler.dumpModule();
                    CONDITION_FAILED;
                }

                llvm::Value *ptrValue = compiler.getContext().builder.CreateAlloca(llvmType, nullptr, varName);
                llvm::LoadInst *loadInst = compiler.getContext().builder.CreateLoad(llvmType, stValue, varName + ".load.var");
                llvm::Value *loadValue = llvm::dyn_cast<llvm::Value>(loadInst);
                llvm::StoreInst *storeValue = compiler.getContext().builder.CreateStore(loadValue, ptrValue);
                storeValue->setAlignment(llvm::Align(8));
                // Add the variable to the named values map & symbol table
                compiler.getContext().namedValues[varName] = ptrValue;
                symTable.updateVariableNode(namespaceName, varName, ptrValue, llvmType);
                symTable.addStoreInstToVar(namespaceName, varName, storeValue);
                symTable.addLoadInstToVar(namespaceName, varName, loadInst);

                break;
            }
            case PRIM_STRING:
            {
                DevDebugger::logMessage("INFO", __LINE__, "Variables", "Creating String Variable");
                llvmValue = compiler.getContext().namedValues[refVarName];
                if (!llvmValue)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Variable not found");
                    CONDITION_FAILED;
                }

                STVariable *var = symTable.getVariable(namespaceName, refVarName);
                if (!var)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Variable not found");
                    CONDITION_FAILED;
                }

                llvm::Value *ST_Value = var->LLVMValue;
                if (!ST_Value)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Variable value not found");
                    CONDITION_FAILED;
                }

                llvm::StoreInst *storeInst = var->LLVMStoreInst;
                if (!storeInst)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Store instruction not found");
                    CONDITION_FAILED;
                }

                // We get the store instruction and the type of the store instruction
                // This is because `alloc` just returns a pointer, and we need the type of the pointer
                llvm::Instruction *storeInstruction = llvm::dyn_cast<llvm::Instruction>(storeInst);
                llvm::Type *storeType = types.parseInstForType(storeInstruction);

                llvm::Value *ptrValue = compiler.getContext().builder.CreateAlloca(storeType, nullptr, varName);
                llvm::LoadInst *loadValue = compiler.getContext().builder.CreateLoad(storeType, llvmValue, varName + ".load.var");
                llvm::StoreInst *storeValue = compiler.getContext().builder.CreateStore(loadValue, ptrValue);
                storeValue->setAlignment(llvm::Align(8));

                // Add the variable to the named values map & symbol table
                compiler.getContext().namedValues[varName] = ptrValue;
                symTable.updateVariableNode(namespaceName, varName, ptrValue, storeType);
                symTable.addStoreInstToVar(namespaceName, varName, storeInst);
                symTable.addLoadInstToVar(namespaceName, varName, loadValue);

                break;
            }
            default:
            {
                DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Unknown data type");
                CONDITION_FAILED;
            }
            }
        default:
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Unknown data type");
            CONDITION_FAILED;
        }
        }
        }

        // Add the variable to the named values map & symbol table
        compiler.getContext().namedValues[varName] = llvmValue;
        symTable.updateVariableNode(namespaceName, varName, llvmValue, llvmType);

        return llvmValue;
    }

    ///
    /// @brief Create a variable with an array literal initializer. (e.g. [1, 2, 3])
    /// @param node The AST node
    /// @return `llvm::Value *` The LLVM value of the variable
    ///
    /// ---
    ///
    /// ```cryo
    ///
    /// \// Example
    ///
    /// const foo: int[] = [1, 2, 3];
    ///
    /// ```
    ///
    llvm::Value *Variables::createArrayLiteralInitializer(CryoArrayNode *arrayNode, DataType *dataType, std::string varName)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Variables", "Creating Array Literal Initializer");

        llvm::Value *llvmValue = nullptr;
        llvm::Type *llvmType = nullptr;
        llvm::Constant *llvmConstant = nullptr;

        if (dataType->container->isArray)
        {
            switch (dataType->container->baseType)
            {
            case PRIMITIVE_TYPE:
            {
                switch (dataType->container->primitive)
                {
                case PRIM_INT:
                {
                    DevDebugger::logMessage("INFO", __LINE__, "Variables", "Creating Int Array Literal");
                    llvmType = compiler.getTypes().getType(dataType, 0);
                    llvmValue = compiler.getArrays().createArrayLiteral(arrayNode, varName);
                    break;
                }
                case PRIM_STRING:
                {
                    DevDebugger::logMessage("INFO", __LINE__, "Variables", "Creating String Array Literal");
                    llvmType = compiler.getTypes().getType(dataType, 0);
                    llvmValue = compiler.getArrays().createArrayLiteral(arrayNode, varName);
                    break;
                }
                default:
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Unknown data type");
                    CONDITION_FAILED;
                }
                }
            }
            }
        }

        // Add the variable to the named values map & symbol table
        compiler.getContext().namedValues[varName] = llvmValue;
        compiler.getSymTable().updateVariableNode(
            compiler.getContext().currentNamespace,
            varName,
            llvmValue,
            llvmType);

        return llvmValue;
    }

    ///
    /// @brief Creates a variable that has an index expression in its initializer.
    /// @param indexExprNode The index expression node
    /// @param nodeType The node type
    /// @param varName The name of the variable
    /// @return `llvm::Value *` A pointer to the LLVM Value representing the newly created variable
    ///
    /// @note UNTESTED
    ///
    /// ---
    ///
    /// ```cryo
    ///
    /// \// Example
    ///
    /// const foo: int = bar[0];
    ///
    /// ```
    ///
    llvm::Value *Variables::createIndexExprInitializer(IndexExprNode *indexExprNode, CryoNodeType nodeType, std::string varName)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Variables", "Creating Index Expression Initializer");
        OldTypes &types = compiler.getTypes();

        llvm::Value *llvmValue = nullptr;
        llvm::Type *llvmType = nullptr;
        llvm::Constant *llvmConstant = nullptr;

        std::string arrayName = std::string(indexExprNode->name);
        std::cout << "Array Name: " << arrayName << std::endl;
        std::cout << "Variable Name: " << varName << std::endl;
        ASTNode *indexNode = indexExprNode->index;
        DevDebugger::logNode(indexNode);
        STVariable *var = compiler.getSymTable().getVariable(compiler.getContext().currentNamespace, arrayName);
        if (!var)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Variable not found");
            CONDITION_FAILED;
        }

        llvm::Value *namedVal = compiler.getContext().namedValues[arrayName];
        if (!namedVal)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Named value not found");
            CONDITION_FAILED;
        }

        llvm::Value *arrayPtr = var->LLVMValue;
        if (!arrayPtr)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Array pointer not found");
            CONDITION_FAILED;
        }

        DataType *dataType = var->dataType;
        if (!dataType)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Data type not found: " + arrayName);
            CONDITION_FAILED;
        }

        bool isStringType = isStringDataType(dataType);
        if (isStringType)
        {
            // If the index expression is trying to index a string, we need to handle it differently.
            return createStringIndexExpr(indexExprNode, varName);
        }

        llvm::Type *arrayType = var->LLVMType;
        if (!arrayType)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Array type not found");
            CONDITION_FAILED;
        }

        llvm::StoreInst *arrStoreInst = var->LLVMStoreInst;
        if (!arrStoreInst)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Store instruction not found");
            CONDITION_FAILED;
        }

        llvm::Instruction *inst = llvm::dyn_cast<llvm::Instruction>(arrStoreInst);
        llvm::Type *arrInstType = compiler.getTypes().parseInstForType(inst);

        // Get the index value
        llvm::Value *indexValue = compiler.getGenerator().getInitilizerValue(indexExprNode->index);
        if (!indexValue)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Index value not found");
            CONDITION_FAILED;
        }

        // Create GEP for array access
        llvm::Value *arrayValue = compiler.getContext().builder.CreateGEP(
            arrInstType,
            arrayPtr,
            indexValue,
            varName + ".array");

        // Load and return array value
        llvm::LoadInst *loadedArr = compiler.getContext().builder.CreateLoad(
            arrInstType,
            arrayValue,
            varName + ".load");

        // Store the array value in the variable
        llvm::Value *varValue = compiler.getContext().builder.CreateAlloca(arrInstType, nullptr, varName);
        llvm::StoreInst *storeInst = compiler.getContext().builder.CreateStore(loadedArr, varValue);
        storeInst->setAlignment(llvm::Align(8));

        // Update the symbol table
        DevDebugger::logMessage("INFO", __LINE__, "Variables", "Updating symbol table with index expr: varName: " + varName);
        compiler.getContext().namedValues[varName] = loadedArr;
        compiler.getSymTable().updateVariableNode(
            compiler.getContext().currentNamespace,
            varName,
            loadedArr,
            arrInstType);

        // Add load inst to var with `addLoadInstToVar`
        compiler.getSymTable().addLoadInstToVar(
            compiler.getContext().currentNamespace,
            varName,
            loadedArr);

        return loadedArr;
    }

    llvm::Value *Variables::createStringIndexExpr(IndexExprNode *indexExprNode, std::string varName)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Variables", "Creating String Index Expression");
        OldTypes &types = compiler.getTypes();

        std::string arrayName = std::string(indexExprNode->name);
        std::cout << "Array Name: " << arrayName << std::endl;
        std::cout << "Variable Name: " << varName << std::endl;
        ASTNode *indexNode = indexExprNode->index;

        STVariable *var = compiler.getSymTable().getVariable(compiler.getContext().currentNamespace, arrayName);
        if (!var)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Variable not found");
            CONDITION_FAILED;
        }

        llvm::Value *arrayPtr = var->LLVMValue;
        if (!arrayPtr)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Array pointer not found");
            CONDITION_FAILED;
        }

        DataType *dataType = var->dataType;
        if (!dataType)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Data type not found");
            CONDITION_FAILED;
        }

        bool isStringType = isStringDataType(dataType);
        if (!isStringType)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Array is not a string type");
            CONDITION_FAILED;
        }

        llvm::Type *arrayType = var->LLVMType;
        if (!arrayType)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Array type not found");
            CONDITION_FAILED;
        }

        llvm::StoreInst *arrStoreInst = var->LLVMStoreInst;
        if (!arrStoreInst)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Store instruction not found");
            CONDITION_FAILED;
        }

        llvm::Instruction *inst = llvm::dyn_cast<llvm::Instruction>(arrStoreInst);
        llvm::Type *arrInstType = compiler.getTypes().parseInstForType(inst);

        // Get the index value
        llvm::Value *indexValue = compiler.getGenerator().getInitilizerValue(indexExprNode->index);
        if (!indexValue)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Index value not found");
            CONDITION_FAILED;
        }

        std::vector<llvm::Value *> indices = {
            llvm::ConstantInt::get(compiler.getContext().context, llvm::APInt(32, 0)),
            indexValue};

        // Create GEP for array access
        llvm::Value *elementPtr = compiler.getContext().builder.CreateGEP(
            arrInstType,
            arrayPtr,
            indices,
            varName + ".array");

        DevDebugger::logMessage("INFO", __LINE__, "Variables", "String GEP Value");
        DevDebugger::logLLVMValue(elementPtr);

        // Load the character value
        llvm::LoadInst *charValue = compiler.getContext().builder.CreateLoad(
            llvm::Type::getInt8Ty(compiler.getContext().context),
            elementPtr,
            varName + ".char");

        DevDebugger::logMessage("INFO", __LINE__, "Variables", "String Load Value");
        DevDebugger::logLLVMValue(charValue);

        // Allocate space for the character and store it
        llvm::AllocaInst *charPtr = compiler.getContext().builder.CreateAlloca(
            llvm::Type::getInt8Ty(compiler.getContext().context),
            nullptr,
            varName + ".char.ptr");

        llvm::StoreInst *storeInst = compiler.getContext().builder.CreateStore(charValue, charPtr);

        // Update the symbol table
        DevDebugger::logMessage("INFO", __LINE__, "Variables", "Updating symbol table with index expr: varName: " + varName);
        compiler.getContext().namedValues[varName] = charPtr;
        compiler.getSymTable().updateVariableNode(
            compiler.getContext().currentNamespace,
            varName,
            charPtr,
            llvm::Type::getInt8Ty(compiler.getContext().context));

        return charPtr;
    }

    ///
    /// @brief Creates a variable with a function call initializer.
    /// @param node The AST node
    /// @return `llvm::Value *` The LLVM value of the variable
    ///
    /// ---
    ///
    /// ```cryo
    ///
    /// \// Example
    ///
    /// const foo: int = bar();
    ///
    /// ```
    ///
    llvm::Value *Variables::createVarWithFuncCallInitilizer(ASTNode *node)
    {
        Functions &functions = compiler.getFunctions();
        IRSymTable &symTable = compiler.getSymTable();
        DevDebugger::logMessage("INFO", __LINE__, "Variables", "Creating Variable with Function Call Initializer");

        // Should be the function call node
        ASTNode *initializer = node->data.varDecl->initializer;
        // The variable node
        ASTNode *variable = node;
        assert(initializer != nullptr);

        std::string moduleName = compiler.getContext().currentNamespace;

        // Create the variable
        std::string varName = std::string(variable->data.varDecl->name);
        llvm::Type *varType = compiler.getTypes().getType(variable->data.varDecl->type, 0);
        llvm::Value *varValue = compiler.getContext().builder.CreateAlloca(varType, nullptr, varName);

        // Get the function call
        if (initializer->metaData->type != NODE_FUNCTION_CALL)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Initializer is not a function call");
            DEBUG_BREAKPOINT;
        }

        std::string functionName = std::string(initializer->data.functionCall->name);
        std::cout << "Function Name: " << functionName << std::endl;

        llvm::Value *functionCall = functions.createFunctionCall(initializer);
        if (!functionCall)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Function call not created");
            CONDITION_FAILED;
        }
        DevDebugger::logMessage("INFO", __LINE__, "Variables", "Function Call Created, Storing in Variable");

        // Store the call into the variable
        llvm::StoreInst *storeInst = compiler.getContext().builder.CreateStore(functionCall, varValue);
        storeInst->setAlignment(llvm::Align(8));

        // Add the variable to the named values map & symbol table
        compiler.getContext().namedValues[varName] = varValue;
        symTable.updateVariableNode(moduleName, varName, varValue, varType);
        symTable.addStoreInstToVar(moduleName, varName, storeInst);

        DevDebugger::logMessage("INFO", __LINE__, "Variables", "Function Call Created");
        return functionCall;
    }

    llvm::Value *Variables::createVarWithBinOpInitilizer(ASTNode *node, std::string varName)
    {
        OldTypes &types = compiler.getTypes();
        IRSymTable &symTable = compiler.getSymTable();
        DevDebugger::logMessage("INFO", __LINE__, "Variables", "Creating Variable with Binary Operation Initializer");

        // llvm::Value * llvmValue = compiler.getBinaryExpressions().handleComplexBinOp(initializer);
        // compiler.getContext().namedValues[varName] = llvmValue;
        // llvmValue->setName(varName);

        std::cout << "Variable Name: " << varName << std::endl;
        llvm::Value *initValue = compiler.getBinaryExpressions().handleComplexBinOp(node);
        if (!initValue)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Initializer value not found");
            CONDITION_FAILED;
        }

        llvm::Value *initializer = compiler.getContext().builder.CreateAlloca(initValue->getType(), nullptr, varName);
        llvm::StoreInst *storeInst = compiler.getContext().builder.CreateStore(initValue, initializer);
        storeInst->setAlignment(llvm::Align(8));

        compiler.getContext().namedValues[varName] = initializer;

        std::string namespaceName = compiler.getContext().currentNamespace;
        symTable.updateVariableNode(namespaceName, varName, initializer, initValue->getType());
        symTable.addStoreInstToVar(namespaceName, varName, storeInst);

        return initializer;
    }
} // namespace Cryo
