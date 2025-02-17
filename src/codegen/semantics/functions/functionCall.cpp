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
#include "tools/logger/logger_config.h"

namespace Cryo
{
    llvm::Value *Functions::createFunctionCall(ASTNode *node)
    {
        Generator &generator = compiler.getGenerator();
        Arrays &arrays = compiler.getArrays();
        Variables &variables = compiler.getVariables();
        OldTypes &types = compiler.getTypes();
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Function Call");
        std::string nodeTypeStr = CryoNodeTypeToString(node->metaData->type);
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Function Call Node Type: " + nodeTypeStr);

        FunctionCallNode *functionCallNode = node->data.functionCall;
        assert(functionCallNode != nullptr);

        // Get the function name
        char *functionName = functionCallNode->name;
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Function Name: " + std::string(functionName));

        // get the current modules name
        std::string moduleName = compiler.getContext().module->getName().str();
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Module Name: " + moduleName);

        // Get the symbol table
        SymTableNode symbolTable = compiler.getSymTable().getSymTableNode(moduleName);

        // Get the function arguments
        int argCount = functionCallNode->argCount;
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Function Call Argument Count: " + std::to_string(argCount));
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Function Callee Name: " + std::string(functionName));

        // STFunction *stFunction = compiler.getSymTable().getFunction(compiler.getContext().currentNamespace, functionName);

        // Get the argument values
        std::vector<llvm::Value *> argValues;
        for (int i = 0; i < argCount; ++i)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Functions", "Processing Argument " + std::to_string(i + 1) + " of " + std::to_string(argCount));
            ASTNode *argNode = functionCallNode->args[i];
            CryoNodeType argNodeType = argNode->metaData->type;
            std::string funcName = std::string(functionName);
            // Callee's name:
            llvm::Function *calleeF = compiler.getContext().module->getFunction(funcName);
            if (!calleeF)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Function not found: @" + funcName);
                std::cout << "Module Name: " << moduleName << std::endl;
                DEBUG_PRINT_FILTER({
                    // compiler.getContext().module->print(llvm::errs(), nullptr);
                    // Dump LLVM's function list
                    for (auto &F : compiler.getContext().module->functions())
                    {
                        std::string funcName = F.getName().str();
                        std::cout << "--------------------------------------------" << std::endl;
                        std::cout << "Function Name: " << funcName << std::endl;
                        std::cout << "--------------------------------------------" << std::endl;
                    }
                });
                compiler.dumpModule();
                CONDITION_FAILED;
            }

            // Get the argument type values
            llvm::FunctionType *calleeFT = calleeF->getFunctionType();
            llvm::Type *expectedType = calleeFT->getParamType(i);

            // Get the current callee function return type
            llvm::Type *returnType = calleeF->getReturnType();

            // Get the argument value
            switch (argNodeType)
            {
            case NODE_VAR_NAME:
            {
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Argument is a variable name");
                VariableNameNode *varNameNode = argNode->data.varName;
                assert(varNameNode != nullptr);

                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Argument Variable Name: " + std::string(varNameNode->varName));

                llvm::Value *argNode = createVarNameCall(varNameNode);
                if (!argNode)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Argument value not found");
                    CONDITION_FAILED;
                }

                argValues.push_back(argNode);
                break;
            }
            case NODE_LITERAL_EXPR:
            {
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Argument is a literal expression");
                LiteralNode *literalNode = argNode->data.literal;
                assert(literalNode != nullptr);

                llvm::Value *argNode = createLiteralCall(literalNode);
                if (!argNode)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Argument value not found");
                    CONDITION_FAILED;
                }

                argValues.push_back(argNode);
                break;
            }
            case NODE_VAR_DECLARATION:
            {
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Argument is a variable declaration");
                CryoVariableNode *varNode = argNode->data.varDecl;
                assert(varNode != nullptr);

                llvm::Value *argNode = createVarDeclCall(varNode);
                if (!argNode)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Argument value not found");
                    CONDITION_FAILED;
                }

                argValues.push_back(argNode);
                break;
            }
            case NODE_ARRAY_LITERAL:
            {
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Argument is an array literal");
                CryoArrayNode *arrayNode = argNode->data.array;
                assert(arrayNode != nullptr);

                llvm::Value *argNode = createArrayCall(arrayNode);
                if (!argNode)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Argument value not found");
                    CONDITION_FAILED;
                }

                argValues.push_back(argNode);
                break;
            }
            case NODE_INDEX_EXPR:
            {
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Argument is an index expression");
                IndexExprNode *indexNode = argNode->data.indexExpr;
                assert(indexNode != nullptr);

                llvm::Value *argNode = createIndexExprCall(indexNode);
                if (!argNode)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Argument value not found");
                    CONDITION_FAILED;
                }

                argNode->getType()->getPointerTo();

                argValues.push_back(argNode);
                break;
            }
            case NODE_PROPERTY_ACCESS:
            {
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Argument is a property access");
                PropertyAccessNode *propNode = argNode->data.propertyAccess;
                assert(propNode != nullptr);

                llvm::Value *argNode = createPropertyAccessCall(propNode);
                if (!argNode)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Argument value not found");
                    CONDITION_FAILED;
                }

                argValues.push_back(argNode);
                break;
            }
            case NODE_PROPERTY:
            {
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Argument is a property");
                PropertyNode *propNode = argNode->data.property;
                assert(propNode != nullptr);

                llvm::Value *argNode = createPropertyCall(propNode);
                if (!argNode)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Argument value not found");
                    CONDITION_FAILED;
                }

                argValues.push_back(argNode);
                break;
            }
            case NODE_METHOD_CALL:
            {
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Argument is a method call");
                MethodCallNode *methodNode = argNode->data.methodCall;
                assert(methodNode != nullptr);

                llvm::Value *argNode = createMethodCall(methodNode);
                if (!argNode)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Argument value not found");
                    CONDITION_FAILED;
                }

                argValues.push_back(argNode);
                break;
            }
            case NODE_TYPEOF:
            {
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Argument is a typeof expression");
                TypeofNode *typeofNode = argNode->data.typeofNode;
                assert(typeofNode != nullptr);

                llvm::Value *argNode = createTypeofCall(typeofNode);
                if (!argNode)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Argument value not found");
                    CONDITION_FAILED;
                }

                argValues.push_back(argNode);
                break;
            }
            case NODE_UNARY_EXPR:
            {
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Argument is a unary expression");
                CryoUnaryOpNode *unaryNode = argNode->data.unary_op;
                assert(unaryNode != nullptr);
                ASTNode *_unaryNode = argNode;

                llvm::Value *argNode = createUnaryExprCall(_unaryNode);
                if (!argNode)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Argument value not found");
                    CONDITION_FAILED;
                }

                argValues.push_back(argNode);
                break;
            }
            default:
            {
                DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Unknown argument type");
                std::cout << "Received: " << CryoNodeTypeToString(argNodeType) << std::endl;
                compiler.dumpModule();
                CONDITION_FAILED;
            }
            }
        }

        // If there are no arguments, just create the function call
        if (argCount == 0)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Function Call with no arguments");
            llvm::Function *function = compiler.getContext().module->getFunction(functionName);
            if (!function)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Function not found: " + std::string(functionName));
                CONDITION_FAILED;
            }

            llvm::Value *functionCall = compiler.getContext().builder.CreateCall(function, argValues);
            if (!functionCall)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Function call not created");
                CONDITION_FAILED;
            }

            DevDebugger::logMessage("INFO", __LINE__, "Functions", "Function Call Created");

            return functionCall;
        }

        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Function Call with arguments");
        // If there are arguments, create the function call with the arguments
        // We will verify the arguments in the function call
        llvm::Function *function = compiler.getContext().module->getFunction(functionName);
        std::vector<llvm::Value *> verifiedArgs = verifyCalleeArguments(function, argValues);

        llvm::Value *functionCall = compiler.getContext().builder.CreateCall(function, verifiedArgs);
        if (!functionCall)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Function call not created");
            CONDITION_FAILED;
        }

        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Function Call Created");

        return functionCall;
    }

    llvm::Value *Functions::createPropertyCall(PropertyNode *property)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Property Call");

        std::string propertyName = std::string(property->name);
        std::string namespaceName = compiler.getContext().currentNamespace;
        std::string parentName = std::string(property->parentName);
        CryoNodeType parentNodeType = property->parentNodeType;
        std::string parentNodeTypeStr = CryoNodeTypeToString(parentNodeType);

        DevDebugger::logMessage("INFO", __LINE__, "Functions",
                                "Property: " + propertyName + " Parent Struct: " + parentName);

        // Get the struct type
        llvm::StructType *structType = compiler.getContext().structTypes[parentName];
        if (!structType)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions",
                                    "LLVM struct type not found for: " + parentName);
            compiler.dumpModule();

            CONDITION_FAILED;
        }

        DataType *structDataType = compiler.getContext().structDataTypes[parentName];
        logVerboseDataType(structDataType);

        int propertyCount = structDataType->container->custom.structDef->propertyCount;

        // Find the property index
        int propertyIndex = -1;
        DataType *propertyType = nullptr;
        for (int i = 0; i < propertyCount; i++)
        {
            PropertyNode *prop = structDataType->container->custom.structDef->properties[i]->data.property;
            if (std::string(prop->name) == propertyName)
            {
                propertyIndex = i;
                propertyType = prop->type;
                break;
            }
        }
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Property Index: " + std::to_string(propertyIndex));

        if (propertyIndex == -1)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions",
                                    "Property not found: " + propertyName);
            CONDITION_FAILED;
        }

        // Get the property's type
        DataType *propType = propertyType;
        llvm::Type *llvmPropType = compiler.getTypes().getType(propType, 0);

        // Get the current function
        llvm::Function *currentFunction = compiler.getContext().currentFunction;
        if (!currentFunction)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions",
                                    "No current function context");
            compiler.dumpModule();
            CONDITION_FAILED;
        }

        llvm::Argument *structInstance = &*currentFunction->arg_begin();
        if (!structInstance)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions",
                                    "Failed to get struct instance from current function");
            compiler.dumpModule();

            CONDITION_FAILED;
        }

        // Create GEP instruction to get property address
        std::vector<llvm::Value *> indices = {
            llvm::ConstantInt::get(compiler.getContext().context, llvm::APInt(32, 0)),
            llvm::ConstantInt::get(compiler.getContext().context, llvm::APInt(32, propertyIndex))};

        llvm::Value *propPtr = compiler.getContext().builder.CreateInBoundsGEP(
            structType,
            structInstance,
            indices,
            "prop." + propertyName + ".ptr");

        // Load and return the property value
        return compiler.getContext().builder.CreateLoad(
            llvmPropType,
            propPtr,
            "prop." + propertyName + ".load");
    }

    llvm::Value *Functions::createPropertyAccessCall(PropertyAccessNode *propAccess)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Handling property access");

        ASTNode *objNode = propAccess->object;
        std::string structVarName;
        CryoNodeType objNodeType = objNode->metaData->type;
        switch (objNodeType)
        {
        case NODE_VAR_DECLARATION:
        {
            structVarName = std::string(objNode->data.varDecl->name);
            break;
        }
        case NODE_PARAM:
        {
            structVarName = std::string(objNode->data.param->name);
            break;
        }
        default:
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions",
                                    "Unknown object node type");
            std::string nodeTypeStr = CryoNodeTypeToString(objNodeType);
            DevDebugger::logMessage("ERROR", __LINE__, "Functions",
                                    "Node Type: " + nodeTypeStr);
            CONDITION_FAILED;
        }
        }
        std::string fieldName = std::string(propAccess->propertyName);
        std::string namespaceName = compiler.getContext().currentNamespace;

        DevDebugger::logMessage("INFO", __LINE__, "Functions",
                                "Accessing " + structVarName + "." + fieldName);

        // Get the struct variable from symbol table
        STVariable *var = compiler.getSymTable().getVariable(namespaceName, structVarName);
        if (!var)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions",
                                    "Struct variable not found: " + structVarName);
            CONDITION_FAILED;
        }

        DevDebugger::logMessage("INFO", __LINE__, "Functions",
                                "Struct variable found: " + structVarName);

        // Get the struct pointer and type
        llvm::Value *structPtr = var->LLVMValue;

        DataType *structDataType = var->dataType;
        StructType *structDef = structDataType->container->custom.structDef;
        std::string structTypeName = std::string(structDataType->container->custom.structDef->name);
        llvm::StructType *structType = compiler.getContext().getStruct(structTypeName);
        if (!structType)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions",
                                    "Struct type not found");
            CONDITION_FAILED;
        }

        DevDebugger::logMessage("INFO", __LINE__, "Functions",
                                "Struct type found: " + structType->getName().str());

        // Find field index
        int fieldIndex = -1;
        int propertyCount = structDef->propertyCount;
        for (int i = 0; i < propertyCount; i++)
        {
            PropertyNode *prop = structDef->properties[i]->data.property;
            if (std::string(prop->name) == fieldName)
            {
                fieldIndex = i;
                break;
            }
        }

        if (fieldIndex == -1)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions",
                                    "Field not found: " + fieldName);
            CONDITION_FAILED;
        }

        // Get field pointer using GEP
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

    llvm::Value *Functions::createVarNameCall(VariableNameNode *varNameNode)
    {
        Variables &variables = compiler.getVariables();
        OldTypes &types = compiler.getTypes();
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Variable Name Call");

        std::string varName = std::string(varNameNode->varName);
        std::string namespaceName = compiler.getContext().currentNamespace;

        STVariable *var = compiler.getSymTable().getVariable(namespaceName, varName);
        if (!var)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Variable not found");
            CONDITION_FAILED;
        }
        DataType *varDataType = var->dataType;
        if (!varDataType)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Variable data type not found for: " + varName);
            CONDITION_FAILED;
        }

        bool isStructType = varDataType->container->baseType == STRUCT_TYPE;

        llvm::Value *varValue = var->LLVMValue;
        if (!varValue)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Variable value not found");
            // Attempt to check for the variable in the named values map
            llvm::Value *namedValue = compiler.getContext().namedValues[varName];
            if (!namedValue)
            {
                // Check if it's a parameter in the symbol table
                STParameter *param = compiler.getSymTable().getParameter(namespaceName, varName);
                if (param)
                {
                    DevDebugger::logMessage("INFO", __LINE__, "Functions", "Parameter found");
                    param->ASTNode->print(param->ASTNode);
                    varValue = param->LLVMValue;
                }
                else
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Named value not found: " + varName);
                    compiler.dumpModule();
                    CONDITION_FAILED;
                }
            }
            std::cout << "Named Value: " << namedValue->getName().str() << std::endl;
            DevDebugger::logMessage("INFO", __LINE__, "Functions", "Named value found");
            varValue = namedValue;
        }

        DevDebugger::logLLVMValue(varValue);
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Variable Value Found");
        llvm::Type *varType = varValue->getType();
        DevDebugger::logLLVMType(varType);

        if (varDataType->container->baseType == STRUCT_TYPE)
        {
            llvm::Type *structType = compiler.getTypes().getType(varDataType, 0);
            if (!structType)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Struct type not found");
                CONDITION_FAILED;
            }
            DevDebugger::logMessage("INFO", __LINE__, "Functions", "Struct Type Found");
            logDataType(varDataType);
            varType = structType;
        }

        llvm::StoreInst *storeInst = var->LLVMStoreInst;
        if (!storeInst)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Store instruction not found");
            // Check if the varName is a function call reference
            if (var->LLVMFunctionPtr)
            {
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Variable is a function reference");
                DevDebugger::logLLVMValue(var->LLVMFunctionPtr);
                return var->LLVMFunctionPtr;
            }

            if (var->LLVMValue)
            {
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Variable is a value reference");
                DevDebugger::logLLVMValue(var->LLVMValue);
                if (isStructType)
                {
                    DevDebugger::logMessage("INFO", __LINE__, "Functions", "Struct Type Found");
                    llvm::Value *structPtr = compiler.getContext().builder.CreateAlloca(varType->getPointerTo(), nullptr, varName + ".ptr");
                    if (!structPtr)
                    {
                        DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Struct pointer not created");
                        CONDITION_FAILED;
                    }

                    llvm::Value *structStore = compiler.getContext().builder.CreateStore(var->LLVMValue, structPtr);
                    if (!structStore)
                    {
                        DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Struct value not stored");
                        CONDITION_FAILED;
                    }

                    llvm::LoadInst *structLoad = compiler.getContext().builder.CreateLoad(varType->getPointerTo(), structPtr, varName + ".load");
                    if (!structLoad)
                    {
                        DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Struct value not loaded");
                        CONDITION_FAILED;
                    }

                    return structLoad;
                }

                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Variable Value Found");
                return var->LLVMValue;
            }

            compiler.dumpModule();
            CONDITION_FAILED;
        }

        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Store Instruction Found");
        llvm::Instruction *inst = llvm::dyn_cast<llvm::Instruction>(storeInst);
        llvm::Type *varInstType = types.parseInstForType(inst);

        // This is to dereference an integer type
        bool isIntType = varInstType->isIntegerTy();
        if (varValue->getType()->isPointerTy() && isIntType)
        {
            llvm::Instruction *inst = llvm::dyn_cast<llvm::Instruction>(storeInst);
            std::cout << "Instruction: " << std::endl;
            DevDebugger::logLLVMInst(inst);
            llvm::Type *varInstType = types.parseInstForType(inst);
            llvm::LoadInst *varLoadValue = compiler.getContext().builder.CreateLoad(varInstType, varValue, varName + ".load.funcCall");
            if (!varLoadValue)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Variable value not loaded");
                CONDITION_FAILED;
            }
            varLoadValue->setAlignment(llvm::Align(8));

            DevDebugger::logMessage("INFO", __LINE__, "Functions", "Var Name Call Created with dereference. For Variable: " + varName);
            DevDebugger::logLLVMValue(varLoadValue);
            return varLoadValue;
        }

        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Var Name Call Created without dereference. For Variable: " + varName);
        DevDebugger::logLLVMValue(varValue);
        return varValue;
    }

    llvm::Value *Functions::createVarDeclCall(CryoVariableNode *varDeclNode)
    {
        IRSymTable &symTable = compiler.getSymTable();
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Variable Declaration Call");

        std::string varName = std::string(varDeclNode->name);

        std::string namespaceName = compiler.getContext().currentNamespace;

        STVariable *var = compiler.getSymTable().getVariable(namespaceName, varName);
        if (!var)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Variable not found");
            CONDITION_FAILED;
        }

        std::cout << "@createVarDeclCall Variable Name: " << varName << std::endl;
        STVariable *varValueNode = symTable.getVariable(namespaceName, varName);
        if (!varValueNode)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Variable not found");
            CONDITION_FAILED;
        }
        llvm::Value *varValue = varValueNode->LLVMValue;
        if (!varValue)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Variable value not found, creating variable");
            CryoNodeType nodeType = varDeclNode->initializer->metaData->type;
            std::cout << "Node Type of VarDecl: " << CryoNodeTypeToString(nodeType) << std::endl;
            DataType *dataType = varDeclNode->type;
            std::cout << "Data Type of VarDecl: " << DataTypeToString(dataType) << std::endl;

            if (nodeType == NODE_LITERAL_EXPR)
            {
                llvm::Value *varValue = compiler.getGenerator().getLiteralValue(varDeclNode->initializer->data.literal);
                if (!varValue)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Variable value not created");
                    CONDITION_FAILED;
                }
                return varValue;
            }
            return varValue;
        }
        return varValue;
    }

    llvm::Value *Functions::createLiteralCall(LiteralNode *literalNode)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Literal Call");

        llvm::Value *literalValue = compiler.getGenerator().getLiteralValue(literalNode);
        if (!literalValue)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Literal value not found");
            CONDITION_FAILED;
        }

        DataType *dataType = literalNode->type;
        if (dataType->container->baseType == PRIMITIVE_TYPE)
        {
            switch (dataType->container->primitive)
            {
            case PRIM_INT:
            {
                // Create the integer literal
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Integer Literal");
                llvm::Type *literalType = compiler.getTypes().getType(createPrimitiveIntType(), 0);
                int literalValue = literalNode->value.intValue;
                llvm::Value *literalInt = llvm::ConstantInt::get(literalType, literalValue, true);
                llvm::Value *literalVarPtr = compiler.getContext().builder.CreateAlloca(literalType, nullptr, "literal.int.ptr");
                if (!literalVarPtr)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Literal variable not created");
                    CONDITION_FAILED;
                }

                llvm::Value *literalVarStore = compiler.getContext().builder.CreateStore(literalInt, literalVarPtr);

                llvm::LoadInst *literalVar = compiler.getContext().builder.CreateLoad(literalType, literalVarPtr, "lit.int.load.funcCall");
                if (!literalVar)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Literal variable not loaded");
                    CONDITION_FAILED;
                }
                literalVar->setAlignment(llvm::Align(8));

                // Add the literal to the named values
                std::string literalName = literalVarPtr->getName().str();
                compiler.getContext().namedValues[literalName] = literalVarPtr;

                return literalVar;
            }
            case PRIM_I8:
            {
                // Create the integer literal
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Integer Literal");
                llvm::Type *literalType = compiler.getTypes().getType(createPrimitiveI8Type(), 0);
                int literalValue = literalNode->value.intValue;
                llvm::Value *literalInt = llvm::ConstantInt::get(literalType, literalValue, true);
                llvm::Value *literalVarPtr = compiler.getContext().builder.CreateAlloca(literalType, nullptr, "literal.int.ptr");
                if (!literalVarPtr)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Literal variable not created");
                    CONDITION_FAILED;
                }

                llvm::Value *literalVarStore = compiler.getContext().builder.CreateStore(literalInt, literalVarPtr);

                llvm::LoadInst *literalVar = compiler.getContext().builder.CreateLoad(literalType, literalVarPtr, "lit.int.load.funcCall");
                if (!literalVar)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Literal variable not loaded");
                    CONDITION_FAILED;
                }
                literalVar->setAlignment(llvm::Align(8));

                // Add the literal to the named values
                std::string literalName = literalVarPtr->getName().str();
                compiler.getContext().namedValues[literalName] = literalVarPtr;

                return literalVar;
            }
            case PRIM_I16:
            {
                // Create the integer literal
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Integer Literal");
                llvm::Type *literalType = compiler.getTypes().getType(createPrimitiveI16Type(), 0);
                int literalValue = literalNode->value.intValue;
                llvm::Value *literalInt = llvm::ConstantInt::get(literalType, literalValue, true);
                llvm::Value *literalVarPtr = compiler.getContext().builder.CreateAlloca(literalType, nullptr, "literal.int.ptr");
                if (!literalVarPtr)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Literal variable not created");
                    CONDITION_FAILED;
                }

                llvm::Value *literalVarStore = compiler.getContext().builder.CreateStore(literalInt, literalVarPtr);

                llvm::LoadInst *literalVar = compiler.getContext().builder.CreateLoad(literalType, literalVarPtr, "lit.int.load.funcCall");
                if (!literalVar)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Literal variable not loaded");
                    CONDITION_FAILED;
                }
                literalVar->setAlignment(llvm::Align(8));

                // Add the literal to the named values
                std::string literalName = literalVarPtr->getName().str();
                compiler.getContext().namedValues[literalName] = literalVarPtr;

                return literalVar;
            }
            case PRIM_I32:
            {
                // Create the integer literal
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Integer Literal");
                llvm::Type *literalType = compiler.getTypes().getType(createPrimitiveI32Type(), 0);
                int literalValue = literalNode->value.intValue;
                llvm::Value *literalInt = llvm::ConstantInt::get(literalType, literalValue, true);
                llvm::Value *literalVarPtr = compiler.getContext().builder.CreateAlloca(literalType, nullptr, "literal.int.ptr");
                if (!literalVarPtr)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Literal variable not created");
                    CONDITION_FAILED;
                }

                llvm::Value *literalVarStore = compiler.getContext().builder.CreateStore(literalInt, literalVarPtr);

                llvm::LoadInst *literalVar = compiler.getContext().builder.CreateLoad(literalType, literalVarPtr, "lit.int.load.funcCall");
                if (!literalVar)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Literal variable not loaded");
                    CONDITION_FAILED;
                }
                literalVar->setAlignment(llvm::Align(8));

                // Add the literal to the named values
                std::string literalName = literalVarPtr->getName().str();
                compiler.getContext().namedValues[literalName] = literalVarPtr;

                return literalVar;
            }
            case PRIM_I64:
            {
                // Create the integer literal
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Integer Literal");
                llvm::Type *literalType = compiler.getTypes().getType(createPrimitiveI64Type(), 0);
                int literalValue = literalNode->value.intValue;
                llvm::Value *literalInt = llvm::ConstantInt::get(literalType, literalValue, true);
                llvm::Value *literalVarPtr = compiler.getContext().builder.CreateAlloca(literalType, nullptr, "literal.int.ptr");
                if (!literalVarPtr)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Literal variable not created");
                    CONDITION_FAILED;
                }

                llvm::Value *literalVarStore = compiler.getContext().builder.CreateStore(literalInt, literalVarPtr);

                llvm::LoadInst *literalVar = compiler.getContext().builder.CreateLoad(literalType, literalVarPtr, "lit.int.load.funcCall");
                if (!literalVar)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Literal variable not loaded");
                    CONDITION_FAILED;
                }
                literalVar->setAlignment(llvm::Align(8));

                // Add the literal to the named values
                std::string literalName = literalVarPtr->getName().str();
                compiler.getContext().namedValues[literalName] = literalVarPtr;

                return literalVar;
            }
            case PRIM_STRING:
            {
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating String Literal");

                // Get the string content from the literal node
                std::string strContent = literalNode->value.stringValue;
                const std::string &strContentRef = strContent;

                // Get or create the global string constant
                llvm::GlobalVariable *globalStr = compiler.getContext().getOrCreateGlobalString(strContentRef);
                if (!globalStr)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Failed to create global string");
                    CONDITION_FAILED;
                }

                // Create GEP to get pointer to the first character
                std::vector<llvm::Value *> indices = {
                    llvm::ConstantInt::get(llvm::Type::getInt64Ty(compiler.getContext().context), 0),
                    llvm::ConstantInt::get(llvm::Type::getInt64Ty(compiler.getContext().context), 0)};

                llvm::Value *strPtr = compiler.getContext().builder.CreateInBoundsGEP(
                    globalStr->getValueType(),
                    globalStr,
                    indices,
                    "str.arg.ptr");

                if (!strPtr)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Failed to create string pointer");
                    CONDITION_FAILED;
                }

                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Successfully created string argument");
                return strPtr;
            }
            case PRIM_BOOLEAN:
            {
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Boolean Literal");
                bool literalValue = literalNode->value.booleanValue;
                llvm::Type *literalType = compiler.getTypes().getType(createPrimitiveBooleanType(literalValue), 0);
                llvm::Value *literalBool = llvm::ConstantInt::get(literalType, literalValue, true);
                llvm::Value *literalVarPtr = compiler.getContext().builder.CreateAlloca(literalType, nullptr, "literal.bool.ptr");
                if (!literalVarPtr)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Literal variable not created");
                    CONDITION_FAILED;
                }

                llvm::Value *literalVarStore = compiler.getContext().builder.CreateStore(literalBool, literalVarPtr);

                llvm::LoadInst *literalVar = compiler.getContext().builder.CreateLoad(literalType, literalVarPtr, "lit.bool.load.funcCall");
                if (!literalVar)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Literal variable not loaded");
                    CONDITION_FAILED;
                }
                literalVar->setAlignment(llvm::Align(8));

                // Add the literal to the named values
                std::string literalName = literalVarPtr->getName().str();
                compiler.getContext().namedValues[literalName] = literalVarPtr;

                return literalVar;
            }
            default:
            {
                DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Unknown literal type");
                CONDITION_FAILED;
            }
            }
        }

        return nullptr;
    }

    llvm::Value *Functions::createIndexExprCall(IndexExprNode *indexNode)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Index Expression Call");
        ErrorHandler &errorHandler = compiler.getErrorHandler();
        // Retrieve the array node and name
        ASTNode *arrayNode = indexNode->array;
        std::string arrayName = std::string(indexNode->name);
        std::cout << "Array Name: " << arrayName << std::endl;
        std::string namespaceName = compiler.getContext().currentNamespace;

        // Get the array variable from the symbol table
        STVariable *var = compiler.getSymTable().getVariable(namespaceName, arrayName);
        if (!var)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Array variable not found");
            CONDITION_FAILED;
        }

        DataType *varDataType = var->dataType;
        if (!varDataType)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Array data type not found");
            CONDITION_FAILED;
        }

        bool isStringType = isStringDataType(varDataType);
        if (isStringType)
        {
            // If the index expression is trying to index a string, we need to handle it differently.
            return compiler.getVariables().createStringIndexExpr(indexNode, arrayName);
        }

        // Get the array pointer and type
        llvm::Value *arrayPtr = var->LLVMValue;
        if (!arrayPtr)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Array pointer not found");
            CONDITION_FAILED;
        }

        // Get the array type
        llvm::Type *arrayType = var->LLVMType;
        if (!arrayType)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Array type not found");
            CONDITION_FAILED;
        }

        llvm::StoreInst *arrStoreInst = var->LLVMStoreInst;
        if (!arrStoreInst)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Store instruction not found");
            CONDITION_FAILED;
        }

        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Getting Data Type from Store Instruction");
        llvm::Instruction *inst = llvm::dyn_cast<llvm::Instruction>(arrStoreInst);
        llvm::Type *arrInstType = compiler.getTypes().parseInstForType(inst);

        // Get the index value
        ASTNode *indexValueNode = indexNode->index;
        llvm::Value *indexValue = compiler.getGenerator().getInitilizerValue(indexValueNode);
        if (!indexValue)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Index value not found");
            CONDITION_FAILED;
        }

        // Get the array type
        DataType *arrayDataType = var->dataType;
        if (!arrayType)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Array type not found");
            CONDITION_FAILED;
        }

        // Create GEP instruction to get array element address
        std::vector<llvm::Value *> indices = {
            llvm::ConstantInt::get(compiler.getContext().context, llvm::APInt(32, 0)),
            indexValue};

        llvm::Value *elementPtr = compiler.getContext().builder.CreateInBoundsGEP(
            arrInstType,
            arrayPtr,
            indices,
            arrayName + ".index.ptr");

        DevDebugger::logLLVMType(arrInstType);

        // Check for an out of bounds error
        errorHandler.IsOutOfBoundsException(indexValue, arrayName, arrInstType);

        if (arrInstType->isArrayTy())
        {
            // If the type is an array, we need to check what kind of array it is (int, string, etc)
            llvm::Type *elementType = arrInstType->getArrayElementType();
            DevDebugger::logLLVMType(elementType);
            if (elementType->isArrayTy())
            {
                // This is a string array
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "String array type");
                arrInstType = arrInstType->getPointerTo();
            }
            else if (elementType->isIntegerTy())
            {
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Integer array type");
                arrInstType = elementType;
            }
        }

        // Load and return the array element value
        return compiler.getContext().builder.CreateLoad(
            arrInstType,
            elementPtr,
            arrayName + ".index.load");
    }

    llvm::Value *Functions::createFunctionCallCall(FunctionCallNode *functionCallNode)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Function Call Call");

        DEBUG_BREAKPOINT;
    }

    llvm::Value *Functions::createArrayCall(CryoArrayNode *arrayNode)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Array Call");

        DEBUG_BREAKPOINT;
    }

    llvm::Value *Functions::createMethodCall(MethodCallNode *node)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Method Call");

        MethodCallNode *methodCallNode = node;
        assert(methodCallNode != nullptr);

        // Get the method name
        const char *methodDefName = methodCallNode->name;
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Method Name: " + std::string(methodDefName));

        // Get the method arguments
        int argCount = methodCallNode->argCount;
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Argument Count: " + std::to_string(argCount));

        // Get the first argument which is the accessor object
        ASTNode *accessorNode = methodCallNode->accessorObj;
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

        llvm::Value *accessorValue = compiler.getContext().namedValues[accessorName];
        if (!accessorValue)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables",
                                    "Accessor value not found: " + accessorName);
            CONDITION_FAILED;
        }

        // Get the argument values
        std::vector<llvm::Value *> argValues = {accessorValue};
        for (int i = 0; i < argCount; ++i)
        {
            ASTNode *argNode = methodCallNode->args[i];
            llvm::Value *argValue = compiler.getGenerator().getInitilizerValue(argNode);
            if (!argValue)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Argument value not found");
                CONDITION_FAILED;
            }

            argValues.push_back(argValue);
        }

        // Combine instance name and method name to get the function name
        std::string instanceName = methodCallNode->instanceName;
        std::string methodName = instanceName + "." + methodDefName;

        // Get the function
        llvm::Function *function = compiler.getContext().module->getFunction(methodName);
        if (!function)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables",
                                    "Function not found: " + methodName);
            CONDITION_FAILED;
        }

        // If there are no arguments, just create the function call
        llvm::Value *functionCall = compiler.getContext().builder.CreateCall(function, argValues);
        if (!functionCall)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Function call not created");
            CONDITION_FAILED;
        }

        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Method Call Created");

        return functionCall;
    }

    void Functions::createScopedFunctionCall(ASTNode *node)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Scoped Function Call");

        ScopedFunctionCallNode *functionCallNode = node->data.scopedFunctionCall;
        assert(functionCallNode != nullptr);

        // Get the function name
        char *functionName = functionCallNode->functionName;
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Function Name: " + std::string(functionName));

        // Get the function arguments
        int argCount = functionCallNode->argCount;
        std::cout << "Argument Count: " << argCount << std::endl;

        // Get the argument values
        std::vector<llvm::Value *> argValues;
        for (int i = 0; i < argCount; ++i)
        {
            ASTNode *argNode = functionCallNode->args[i];
            CryoNodeType argType = argNode->metaData->type;
            switch (argType)
            {
            case NODE_VAR_DECLARATION:
            {
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Variable Declaration");
                llvm::Value *argValue = compiler.getGenerator().getInitilizerValue(argNode);
                if (!argValue)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Argument value not found");
                    CONDITION_FAILED;
                }
                argValues.push_back(argValue);
                break;
            }
            case NODE_LITERAL_EXPR:
            {
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Literal Expression");
                llvm::Value *argValue = compiler.getGenerator().getInitilizerValue(argNode);
                if (!argValue)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Argument value not found");
                    CONDITION_FAILED;
                }
                // Create a new variable for the literal
                llvm::Value *literalVar = compiler.getVariables().createLocalVariable(argNode);
                if (!literalVar)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Literal variable not created");
                    CONDITION_FAILED;
                }

                // Store the literal value in the variable
                compiler.getContext().builder.CreateStore(argValue, literalVar);
                argValues.push_back(literalVar);
                break;
            }
            default:
            {
                DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Unknown argument type");
                CONDITION_FAILED;
            }
            }
        }

        // Get the function
        llvm::Function *function = compiler.getContext().module->getFunction(functionName);
        if (!function)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Function not found: @" + std::string(functionName));
            CONDITION_FAILED;
        }

        // If there are no arguments, just create the function call
        llvm::Value *functionCall = compiler.getContext().builder.CreateCall(function, argValues);
        if (!functionCall)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Function call not created");
            CONDITION_FAILED;
        }

        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Scoped Function Call Created");

        return;
    }

    void Functions::handleMethodCall(ASTNode *node)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Handling Method Call");

        MethodCallNode *methodCallNode = node->data.methodCall;
        assert(methodCallNode != nullptr);

        // Get the method name
        const char *methodDefName = methodCallNode->name;
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Method Name: " + std::string(methodDefName));

        // Get the method arguments
        int argCount = methodCallNode->argCount;
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Argument Count: " + std::to_string(argCount));

        bool isStaticMethod = methodCallNode->isStatic;
        if (isStaticMethod)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Functions", "Handling Static Method Call");
            handleStaticMethodCall(node);
            return;
        }

        // This was for added for structs using dot notation accessing methods / properties
        // Now with `{name}::{method}` syntax, we need to implement a new path for this type of syntax.
        // ------------------------------------------------

        // Get the first argument which is the accessor object
        ASTNode *accessorNode = methodCallNode->accessorObj;
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

        llvm::Value *accessorValue = compiler.getContext().namedValues[accessorName];
        if (!accessorValue)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables",
                                    "Accessor value not found: " + accessorName);
            CONDITION_FAILED;
        }

        // Get the argument values
        std::vector<llvm::Value *> argValues = {accessorValue};

        for (int i = 0; i < argCount; ++i)
        {
            ASTNode *argNode = methodCallNode->args[i];
            llvm::Value *argValue = compiler.getGenerator().getInitilizerValue(argNode);
            if (!argValue)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Argument value not found");
                CONDITION_FAILED;
            }

            argValues.push_back(argValue);
        }

        // Combine instance name and method name to get the function name
        std::string instanceName = methodCallNode->instanceName;
        std::string methodName = instanceName + "." + methodDefName;

        // Get the function
        llvm::Function *function = compiler.getContext().module->getFunction(methodName);
        if (!function)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables",
                                    "Function not found: " + methodName);
            CONDITION_FAILED;
        }

        // If there are no arguments, just create the function call
        llvm::Value *functionCall = compiler.getContext().builder.CreateCall(function, argValues);
        if (!functionCall)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Function call not created");
            CONDITION_FAILED;
        }

        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Method Call Handled");

        return;
    }

    void Functions::handleStaticMethodCall(ASTNode *node)
    {
        // A static method call assumes that it is calling a method from a class statically
        // Such as `IO::print("Hello, World!")`, no instance is needed to call the method
        // The accessor object in the ASTNode is undefined since it is a static reference to a call
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Handling Static Method Call");

        MethodCallNode *methodCallNode = node->data.methodCall;
        assert(methodCallNode != nullptr);

        // Get the method name
        std::string methodDefName = std::string(methodCallNode->name);
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Method Name: " + std::string(methodDefName));

        // Get the method arguments
        int argCount = methodCallNode->argCount;
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Argument Count: " + std::to_string(argCount));

        // Get the argument values
        std::vector<llvm::Value *> argValues;
        for (int i = 0; i < argCount; ++i)
        {
            ASTNode *argNode = methodCallNode->args[i];
            DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Argument Value");
            DevDebugger::logNode(argNode);
            DataType *nodeType = getDataTypeFromASTNode(argNode);
            if (!nodeType)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Node type not found");
                CONDITION_FAILED;
            }
            logDataType(nodeType);

            bool isStringType = isStringDataType(nodeType);
            if (isStringType)
            {
                llvm::Value *argValue = compiler.getVariables().createStringVariable(argNode);
                if (!argValue)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Argument value not created");
                    CONDITION_FAILED;
                }

                argValues.push_back(argValue);
                continue;
            }

            llvm::Value *argValue = compiler.getGenerator().getInitilizerValue(argNode);
            if (!argValue)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Argument value not found");
                CONDITION_FAILED;
            }

            argValues.push_back(argValue);
        }

        // Get the function (class.className.methodName)
        std::string instanceName = methodCallNode->instanceName;
        std::string classFnName = "class"
                                  "." +
                                  instanceName + "." + methodDefName;
        llvm::Function *function = compiler.getContext().module->getFunction(classFnName);
        if (!function)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables",
                                    "Function not found: " + classFnName);
            compiler.dumpModule();
            CONDITION_FAILED;
        }

        // If there are no arguments, just create the function call
        llvm::Value *functionCall = compiler.getContext().builder.CreateCall(function, argValues);
        if (!functionCall)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Function call not created");
            CONDITION_FAILED;
        }

        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Static Method Call Handled");

        return;
    }

    llvm::Value *Functions::createTypeofCall(TypeofNode *node)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Typeof Call");

        ASTNode *exprNode = node->expression;
        DataType *exprType = getDataTypeFromASTNode(exprNode);
        if (!exprType)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Expression type not found");
            CONDITION_FAILED;
        }
        std::string exprTypeStr = DataTypeToStringUnformatted(exprType);
        std::cout << "Typeof Expression Type: " << exprTypeStr << std::endl;

        // Always return the type as a string
        llvm::Value *typeValue = compiler.getContext().builder.CreateGlobalStringPtr(exprTypeStr, "typeof");

        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Typeof Call Created");
        return typeValue;
    }

    llvm::Value *Functions::createUnaryExprCall(ASTNode *node)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Unary Expression Call");
        CryoUnaryOpNode *unaryNode = node->data.unary_op;
        node->print(node);

        ASTNode *exprNode = unaryNode->expression;
        exprNode->print(exprNode);

        llvm::Value *exprValue = compiler.getGenerator().getInitilizerValue(exprNode);
        if (!exprValue)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Expression value not found");
            CONDITION_FAILED;
        }

        DevDebugger::logLLVMValue(exprValue);

        std::string exprName = exprValue->getName().str();
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Expression Name: " + exprName);

        llvm::Value *unaryValue = nullptr;
        switch (unaryNode->op)
        {
        case TOKEN_ADDRESS_OF:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Address Of Call");
            // Find the variable in the current function
            llvm::Function *function = compiler.getContext().currentFunction;
            compiler.getContext().printNamedValues();
            std::cout << "Seeking for variable: " << exprName << std::endl;
            llvm::Value *varAddr = compiler.getContext().namedValues[exprName];
            if (!varAddr)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Variable address not found: " + exprName);
                // Debug print the `getValueSymbolTable`
                std::cout << "\n\n";
                function->getValueSymbolTable()->dump();
                std::cout << "\n\n";

                // Debug print the symbol table
                CONDITION_FAILED;
            }
            DevDebugger::logMessage("INFO", __LINE__, "Functions", "Address Of Call Created");
            return varAddr;
        }
        case TOKEN_DEREFERENCE:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Dereference Call");
            llvm::Instruction *inst = llvm::dyn_cast<llvm::Instruction>(exprValue);
            std::cout << "Instruction: " << std::endl;
            DevDebugger::logLLVMInst(inst);
            llvm::Type *varInstType = compiler.getTypes().parseInstForType(inst);
            llvm::LoadInst *varLoadValue = compiler.getContext().builder.CreateLoad(varInstType, exprValue, "deref");
            if (!varLoadValue)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Variable value not loaded");
                CONDITION_FAILED;
            }
            varLoadValue->setAlignment(llvm::Align(8));

            DevDebugger::logMessage("INFO", __LINE__, "Functions", "Dereference Call Created");
            return varLoadValue;
        }
        default:
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Unknown unary operator");
            CONDITION_FAILED;
        }
        }

        DEBUG_BREAKPOINT;
    }

} // namespace Cryo