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
#include "cpp/codegen.h"

namespace Cryo
{
    void Functions::handleFunction(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Functions", "Handling Function");

        CryoNodeType nodeType = node->metaData->type;
        switch (nodeType)
        {
        case NODE_FUNCTION_DECLARATION:
        {
            debugger.logMessage("INFO", __LINE__, "Functions", "Handling Function Declaration");
            createFunctionDeclaration(node);
            break;
        }
        case NODE_FUNCTION_BLOCK:
        {
            debugger.logMessage("INFO", __LINE__, "Functions", "Handling Function Block");
            createFunctionBlock(node);
            break;
        }
        case NODE_FUNCTION_CALL:
        {
            debugger.logMessage("INFO", __LINE__, "Functions", "Handling Function Call");
            createFunctionCall(node);
            break;
        }
        case NODE_RETURN_STATEMENT:
        {
            debugger.logMessage("INFO", __LINE__, "Functions", "Handling Return Statement");
            createReturnStatement(node);
            break;
        }
        case NODE_EXTERN_FUNCTION:
        {
            debugger.logMessage("INFO", __LINE__, "Functions", "Handling Extern Function");
            createExternFunction(node);
            break;
        }
        default:
            debugger.logMessage("ERROR", __LINE__, "Functions", "Unknown node type");
            CONDITION_FAILED;
        }
    }

    // -----------------------------------------------------------------------------------------------

    void Functions::createFunctionDeclaration(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        CryoContext &cryoContext = compiler.getContext();
        Variables &variables = compiler.getVariables();
        Generator &generator = compiler.getGenerator();
        Types &types = compiler.getTypes();
        debugger.logMessage("INFO", __LINE__, "Functions", "Creating Function Declaration");

        FunctionDeclNode *functionNode = node->data.functionDecl;
        assert(functionNode != nullptr);

        // Get the function name
        char *functionName = functionNode->name;
        debugger.logMessage("INFO", __LINE__, "Functions", "Function Name: " + std::string(functionName));

        // Get the return type
        CryoDataType returnType = functionNode->returnType;
        debugger.logMessage("INFO", __LINE__, "Functions", "Return Type: " + std::string(CryoDataTypeToString(returnType)));

        // Get the function arguments
        int argCount = functionNode->paramCount;
        debugger.logMessage("INFO", __LINE__, "Functions", "Argument Count: " + std::to_string(argCount));

        std::vector<llvm::Type *> argTypes;
        for (int i = 0; i < argCount; ++i)
        {
            CryoParameterNode *argNode = functionNode->params[i]->data.param;
            assert(argNode != nullptr);
            CryoDataType _argType = argNode->type;

            if (_argType == DATA_TYPE_STRING)
            {
                debugger.logMessage("INFO", __LINE__, "Functions", "Converting string to LLVM type");
                // int _len = types.getLiteralValLength(argNode->initializer);
                // Need to find a solution to get the length of the string, setting the type as a pointer might be a temp thing.
                llvm::Type *argType = types.getType(_argType, 32);
                argTypes.push_back(argType->getPointerTo());
                continue;
            }

            if (_argType == DATA_TYPE_INT)
            {
                debugger.logMessage("INFO", __LINE__, "Functions", "Converting int to LLVM type");
                llvm::Type *argType = types.getType(_argType, 0);
                argTypes.push_back(argType);
                continue;
            }

            // llvm::Type *argType = compiler.getTypes().getType(argNode->type, 0);
            // argTypes.push_back(argType);
        }

        // Get the function Body
        ASTNode *functionBody = functionNode->body;
        assert(functionBody != nullptr);

        llvm::Type *funcRetType = nullptr;
        CryoFunctionBlock *functionBlock = functionBody->data.functionBlock;

        // Traverse the function block to get the return type
        funcRetType = traverseBlockReturnType(functionBody->data.functionBlock);

        // Create the function type
        llvm::FunctionType *functionType = llvm::FunctionType::get(funcRetType, argTypes, false);

        std::cout << "Function Type: " << std::endl;

        // Create the function
        llvm::Function *function = llvm::Function::Create(
            functionType,
            llvm::Function::ExternalLinkage,
            llvm::Twine(functionName),
            *cryoContext.module);

        // Add the function to the symbol table
        cryoContext.namedValues[functionName] = function;

        // Create the entry block
        llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(compiler.getContext().context, "entry", function);
        compiler.getContext().builder.SetInsertPoint(entryBlock);
        compiler.getContext().currentFunction = function;
        compiler.getContext().inGlobalScope = false;

        // Set the function arguments
        int i = 0;
        for (auto &arg : function->args())
        {
            // We are storing the aguments in the named values map
            std::string paramName = functionNode->params[i]->data.param->name;
            arg.setName(paramName);
            llvm::Value *param = createParameter(&arg, argTypes[i]);
            compiler.getContext().namedValues[paramName] = &arg;
            ++i;
        }

        // Note to self: This is the block that will loop through the function body & statement
        // I need to find a better and most consistent way to handle this.
        // It seems like I need something like the `parseTree` method but that returns a `llvm::Value *` without removing `parseTree`.
        // These statements needs to be in the same block as the function body.

        for (int i = 0; i < functionBody->data.functionBlock->statementCount; ++i)
        {
            debugger.logMessage("INFO", __LINE__, "Functions", "Parsing Statement " + std::to_string(i + 1) + " of " + std::to_string(functionBody->data.functionBlock->statementCount));
            ASTNode *statement = functionBody->data.functionBlock->statements[i];
            CryoNodeType nodeType = statement->metaData->type;
            std::cout << "Statement: " << CryoNodeTypeToString(statement->metaData->type) << std::endl;

            if (nodeType == NODE_VAR_DECLARATION)
            {
                debugger.logMessage("INFO", __LINE__, "Functions", "Creating Variable Declaration");
                variables.createLocalVariable(statement);
                continue;
            }

            // if (nodeType == NODE_FUNCTION_CALL)
            // {
            //     debugger.logMessage("INFO", __LINE__, "Functions", "Creating Function Call");
            //     createFunctionCall(statement);
            //     continue;
            // }

            if (nodeType == NODE_RETURN_STATEMENT)
            {
                llvm::Type *returnLLVMType = types.getReturnType(returnType);
                switch (returnType)
                {
                case DATA_TYPE_VOID:
                {
                    debugger.logMessage("INFO", __LINE__, "Functions", "Returning void");
                    compiler.getContext().builder.CreateRet(nullptr);
                    break;
                }
                case DATA_TYPE_INT:
                {
                    debugger.logMessage("INFO", __LINE__, "Functions", "Returning int");
                    ASTNode *returnStatement = statement->data.returnStatement->expression;
                    llvm::Value *returnValue = generator.getInitilizerValue(returnStatement);
                    compiler.getContext().builder.CreateRet(returnValue);
                    break;
                }
                case DATA_TYPE_STRING:
                {
                    debugger.logMessage("INFO", __LINE__, "Functions", "Returning string");
                    int _len = types.getLiteralValLength(statement);
                    llvm::Type *returnType = types.getType(DATA_TYPE_STRING, _len);
                    llvm::Value *returnValue = generator.getInitilizerValue(statement);
                    compiler.getContext().builder.CreateRet(returnValue);

                    break;
                }
                default:
                {
                    debugger.logMessage("ERROR", __LINE__, "Functions", "Unknown return type");
                    std::cout << "Received: " << CryoDataTypeToString(returnType) << std::endl;
                    CONDITION_FAILED;
                }
                }
                continue;
            }

            else
            {
                compiler.getGenerator().parseTree(statement);
            }
        }

        // Check if the current block is already terminated
        llvm::BasicBlock *currentBlock = compiler.getContext().builder.GetInsertBlock();
        if (!currentBlock->getTerminator())
        {
            debugger.logMessage("ERROR", __LINE__, "Functions", "Adding terminator to function");
            CONDITION_FAILED;
        }
        else
        {
            debugger.logMessage("INFO", __LINE__, "Functions", "Function already has a terminator");
        }

        debugger.logMessage("INFO", __LINE__, "Functions", "Function Declaration Created");

        // Exit the scope of the function
        compiler.getContext().builder.ClearInsertionPoint();
        compiler.getContext().currentFunction = nullptr;
        compiler.getContext().inGlobalScope = true;

        return;
    }

    // -----------------------------------------------------------------------------------------------

    void Functions::createFunctionBlock(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Types &types = compiler.getTypes();
        debugger.logMessage("INFO", __LINE__, "Functions", "Creating Function Block");

        CryoFunctionBlock *functionBlockNode = node->data.functionBlock;

        debugger.logMessage("INFO", __LINE__, "Functions", "Getting Function Block");

        ASTNode **statements = functionBlockNode->statements;
        int statementCount = functionBlockNode->statementCount;

        for (int i = 0; i < statementCount; ++i)
        {
            debugger.logMessage("INFO", __LINE__, "Functions", "Processing Statement " + std::to_string(i + 1) + " of " + std::to_string(statementCount));
            ASTNode *statement = statements[i];
            if (statement->metaData->type == NODE_RETURN_STATEMENT)
            {
                debugger.logMessage("INFO", __LINE__, "Functions", "Creating Return Statement");
                return;
            }
            compiler.getGenerator().parseTree(statement);
        }

        debugger.logMessage("INFO", __LINE__, "Functions", "Function Block Created");

        return;
    }

    // -----------------------------------------------------------------------------------------------

    void Functions::createReturnStatement(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Generator &generator = compiler.getGenerator();
        CryoContext &cryoContext = compiler.getContext();
        if (node->metaData->type != NODE_RETURN_STATEMENT)
        {
            debugger.logMessage("ERROR", __LINE__, "Functions", "Node is not a return statement");
            CONDITION_FAILED;
        }
        Types &types = compiler.getTypes();
        debugger.logMessage("INFO", __LINE__, "Functions", "Creating Return Statement");

        CryoReturnNode *returnNode = node->data.returnStatement;
        assert(returnNode != nullptr);

        CryoDataType returnType = returnNode->returnType;
        debugger.logMessage("INFO", __LINE__, "Functions", "Return Type: " + std::string(CryoDataTypeToString(returnType)));

        switch (returnType)
        {
        case DATA_TYPE_VOID:
        {
            debugger.logMessage("INFO", __LINE__, "Functions", "Returning void");
            cryoContext.builder.CreateRet(nullptr);
            break;
        }
        }
        // This is being handled in the function block
        // Nothing to do in this function
        // I might migrate the logic within the `createFunctionDeclaration` function to this function in the future.

        return;
    }

    // -----------------------------------------------------------------------------------------------

    /**
     * @brief Traverse the function block to get the return type for the function declaration.
     * (Not the return statement terminator)
     */
    llvm::Type *Functions::traverseBlockReturnType(CryoFunctionBlock *blockNode)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Types &types = compiler.getTypes();
        debugger.logMessage("INFO", __LINE__, "Functions", "Traversing Function Block");

        ASTNode **statements = blockNode->statements;
        llvm::Type *returnType = nullptr;

        for (int i = 0; i < blockNode->statementCount; ++i)
        {
            ASTNode *statement = statements[i];
            if (statement->metaData->type == NODE_RETURN_STATEMENT)
            {
                CryoDataType nodeDataType = statement->data.returnStatement->returnType;

                switch (nodeDataType)
                {
                case DATA_TYPE_INT:
                {

                    debugger.logMessage("INFO", __LINE__, "Functions", "Returning int");
                    returnType = types.getReturnType(DATA_TYPE_INT);
                    break;
                }
                case DATA_TYPE_STRING:
                {
                    debugger.logMessage("INFO", __LINE__, "Functions", "Returning string");
                    int _len = types.getLiteralValLength(statement->data.returnStatement->expression);
                    // +1 for the null terminator
                    returnType = types.getType(DATA_TYPE_STRING, _len + 1);
                    break;
                }
                case DATA_TYPE_VOID:
                {
                    debugger.logMessage("INFO", __LINE__, "Functions", "Returning void");
                    returnType = types.getReturnType(DATA_TYPE_VOID);
                    break;
                }
                default:
                {
                    debugger.logMessage("ERROR", __LINE__, "Functions", "Unknown return type");
                    std::cout << "Received: " << CryoDataTypeToString(nodeDataType) << std::endl;
                    CONDITION_FAILED;
                }
                }
            }
            if (statement->metaData->type == NODE_BINARY_EXPR)
            {
                debugger.logMessage("INFO", __LINE__, "Functions", "");
                returnType = types.getReturnType(DATA_TYPE_INT);
            }
        }
        debugger.logMessage("INFO", __LINE__, "Functions", "Function Block Traversed");
        return returnType;
    }

    // -----------------------------------------------------------------------------------------------

    /**
     * @brief Create an extern function.
     * This should only declare the function in the IR, it will not create the function body.
     */
    void Functions::createExternFunction(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Functions", "Creating Extern Function");

        ExternFunctionNode *functionNode = node->data.externFunction;

        // Get the function name
        char *functionName = functionNode->name;
        debugger.logMessage("INFO", __LINE__, "Functions", "Function Name: " + std::string(functionName));

        // Get the return type
        CryoDataType returnType = functionNode->returnType;
        debugger.logMessage("INFO", __LINE__, "Functions", "Return Type: " + std::string(CryoDataTypeToString(returnType)));

        // Get the function arguments
        int argCount = functionNode->paramCount;
        debugger.logMessage("INFO", __LINE__, "Functions", "Argument Count: " + std::to_string(argCount));

        std::vector<llvm::Type *> argTypes;
        for (int i = 0; i < argCount; ++i)
        {
            CryoParameterNode *argNode = functionNode->params[i]->data.param;
            assert(argNode != nullptr);
            CryoDataType _argType = argNode->type;

            if (_argType == DATA_TYPE_STRING)
            {
                debugger.logMessage("INFO", __LINE__, "Functions", "Converting string to LLVM type");
                llvm::Type *argType = compiler.getTypes().getType(_argType, 0);
                argTypes.push_back(argType);
                continue;
            }

            llvm::Type *argType = compiler.getTypes().getType(argNode->type, 0);
            argTypes.push_back(argType);
        }

        // Create the function type
        llvm::FunctionType *functionType = llvm::FunctionType::get(compiler.getTypes().getReturnType(returnType), argTypes, false);

        // Create the function
        llvm::Function *function = llvm::Function::Create(
            functionType,
            llvm::Function::ExternalLinkage,
            llvm::Twine(functionName),
            *compiler.getContext().module);

        // Set the function arguments
        int i = 0;
        for (auto &arg : function->args())
        {
            arg.setName(functionNode->params[i]->data.param->name);
            ++i;
        }

        debugger.logMessage("INFO", __LINE__, "Functions", "Extern Function Created");

        return;
    }

    // -----------------------------------------------------------------------------------------------

    llvm::Value *Functions::createFunctionCall(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Generator &generator = compiler.getGenerator();
        Arrays &arrays = compiler.getArrays();
        Variables &variables = compiler.getVariables();
        Types &types = compiler.getTypes();
        debugger.logMessage("INFO", __LINE__, "Functions", "Creating Function Call");

        FunctionCallNode *functionCallNode = node->data.functionCall;
        assert(functionCallNode != nullptr);

        // Get the function name
        char *functionName = functionCallNode->name;
        debugger.logMessage("INFO", __LINE__, "Functions", "Function Name: " + std::string(functionName));

        // get the current modules name
        std::string moduleName = compiler.getContext().module->getName().str();
        debugger.logMessage("INFO", __LINE__, "Functions", "Module Name: " + moduleName);

        // Get the symbol table
        SymTableNode symbolTable = compiler.getSymTable().getSymTableNode(moduleName);

        // Get the function arguments
        int argCount = functionCallNode->argCount;
        debugger.logMessage("INFO", __LINE__, "Functions", "Function Call Argument Count: " + std::to_string(argCount));
        debugger.logMessage("INFO", __LINE__, "Functions", "Function Callee Name: " + std::string(functionName));

        // Get the argument values
        std::vector<llvm::Value *> argValues;
        for (int i = 0; i < argCount; ++i)
        {
            debugger.logMessage("INFO", __LINE__, "Functions", "Processing Argument " + std::to_string(i + 1) + " of " + std::to_string(argCount));
            ASTNode *argNode = functionCallNode->args[i];
            CryoNodeType argType = argNode->metaData->type;
            CryoDataType argTypeData = argNode->data.varDecl->type;
            std::string argName = std::string(argNode->data.varDecl->name);
            CryoVariableNode *retreivedNode = compiler.getSymTable().getVariableNode(moduleName, argName);
            if (!retreivedNode)
            {
                debugger.logMessage("ERROR", __LINE__, "Functions", "Argument not found");
                CONDITION_FAILED;
            }

            std::cout << "===----------------------===" << std::endl;
            std::cout << "Argument #: " << i + 1 << std::endl;
            std::cout << "Function Name: " << functionName << std::endl;
            std::cout << "Argument Node Type: " << CryoNodeTypeToString(argType) << std::endl;
            std::cout << "Argument Data Type: " << CryoDataTypeToString(argTypeData) << std::endl;
            std::cout << "Argument Name: " << argNode->data.varDecl->name << std::endl;
            std::cout << "===----------------------===" << std::endl;

            std::string funcName = std::string(functionName);
            std::cout << "\n\nFunction Name: " << funcName << "\n"
                      << std::endl;

            // Callee's name:
            llvm::Function *calleeF = compiler.getContext().module->getFunction(funcName);
            if (!calleeF)
            {
                debugger.logMessage("ERROR", __LINE__, "Functions", "Function not found");
                CONDITION_FAILED;
            }

            // Get the argument type values
            llvm::FunctionType *calleeFT = calleeF->getFunctionType();
            llvm::Type *expectedType = calleeFT->getParamType(i);
            std::cout << "Argument Type: " << std::endl;

            debugger.logMessage("INFO", __LINE__, "Functions", "Function Found");

            // Get the current callee function return type
            llvm::Type *returnType = calleeF->getReturnType();

            std::cout << "Return Type: " << std::endl;

            switch (argTypeData)
            {
            case DATA_TYPE_INT:
            {
                debugger.logMessage("INFO", __LINE__, "Functions", "Creating Int Argument");
                if (retreivedNode->hasIndexExpr)
                {
                    debugger.logMessage("INFO", __LINE__, "Functions", "Argument has index expression");
                    std::string argumentName = std::string(argNode->data.varDecl->name);
                    std::cout << "Argument Name: " << argumentName << std::endl;

                    llvm::Value *argValue = variables.getVariable(argumentName);
                    if (!argValue)
                    {
                        debugger.logMessage("ERROR", __LINE__, "Functions", "Variable not found");
                        DEBUG_BREAKPOINT;
                    }
                    debugger.logLLVMValue(argValue);

                    if (argValue->getType()->isPointerTy())
                    {
                        debugger.logMessage("INFO", __LINE__, "Functions", "Argument is a pointer");
                        llvm::Type *literalInt = compiler.getTypes().getType(DATA_TYPE_INT, 0);
                        llvm::Value *loadValue = compiler.getContext().builder.CreateLoad(literalInt, argValue);
                        debugger.logLLVMValue(loadValue);
                        argValues.push_back(loadValue);
                        break;
                    }

                    // Set the function argument
                    argValues.push_back(argValue);
                    break;
                }
                debugger.logMessage("INFO", __LINE__, "Functions", "Argument named being passed to argValue : " + argName);
                llvm::Value *argValue = variables.getVariable(argName);
                // If the variable is not found, create a new one
                if (!argValue)
                {
                    debugger.logMessage("ERROR", __LINE__, "Functions", "Variable not found");
                    llvm::Value *newVar = variables.createLocalVariable(argNode);
                    // If the variable is not created, exit
                    if (!newVar)
                    {
                        debugger.logMessage("ERROR", __LINE__, "Functions", "Variable not created");
                        CONDITION_FAILED;
                    }
                    debugger.logMessage("INFO", __LINE__, "Functions", "Argument being pushed to argValues");
                    assert(newVar != nullptr);

                    debugger.logLLVMValue(newVar);

                    compiler.getContext().namedValues[argName] = newVar;
                    argValues.push_back(newVar);

                    std::cout << "Logging value at index " << i << std::endl;
                    debugger.logLLVMValue(argValues[i]);
                    break;
                }

                debugger.logMessage("INFO", __LINE__, "Functions", "Argument being pushed to argValues");
                llvm::Type *literalInt = compiler.getTypes().getType(DATA_TYPE_INT, 0);
                debugger.logMessage("INFO", __LINE__, "Functions", "Got Literal Int Type");
                argValues.push_back(argValue);
                break;
            }
            case DATA_TYPE_STRING:
            {
                debugger.logMessage("INFO", __LINE__, "Functions", "Creating String Argument");
                std::string argName = std::string(argNode->data.varDecl->name);
                // argName = types.trimStrQuotes(argName);
                argNode->data.varDecl->name = (char *)argName.c_str();
                std::cout << "Argument Name: " << argName << std::endl;

                llvm::Value *argValue = nullptr;
                llvm::Value *localVar = variables.getVariable(argName);
                if (!localVar)
                {
                    debugger.logMessage("ERROR", __LINE__, "Functions", "Local Variable not found, attempting to create");
                    llvm::Value *newVar = variables.createLocalVariable(argNode);
                    if (!newVar)
                    {
                        debugger.logMessage("ERROR", __LINE__, "Functions", "Local Variable not created");
                        exit(1);
                    }
                    debugger.logMessage("INFO", __LINE__, "Functions", "Local Variable created");
                    argValue = newVar;
                    compiler.getDebugger().logLLVMValue(argValue);
                }
                else
                {
                    debugger.logMessage("INFO", __LINE__, "Functions", "Local Variable found");
                    argValue = localVar;
                }

                debugger.logMessage("INFO", __LINE__, "Functions", "Argument being pushed to argValues");
                argValues.push_back(argValue);
                break;
            }
            default:
            {
                debugger.logMessage("ERROR", __LINE__, "Functions", "Unknown argument type");
                std::cout << "Received: " << CryoDataTypeToString(argTypeData) << std::endl;

                CONDITION_FAILED;
            }
            }

            std::vector<llvm::Type *> expectedTypes;
            // Loop through the function calls expected types from the callee function
            for (int i = 0; i < calleeFT->getNumParams(); ++i)
            {
                llvm::Type *expectedType = calleeFT->getParamType(i);
                std::string typeID = debugger.LLVMTypeIDToString(expectedType);
                std::cout << "Expected Type: " << typeID << std::endl;

                expectedTypes.push_back(expectedType);
            }

            // Loop through the arguments and assign the values to the expected types
            for (int i = 0; i < argCount; ++i)
            {
                std::cout << "\n\nArgument #: " << i + 1 << std::endl;
                llvm::Value *argValue = argValues[i];
                debugger.logLLVMValue(argValue);
                llvm::Type *expectedType = expectedTypes[i];
                std::string typeID = debugger.LLVMTypeIDToString(expectedType);
                std::cout << "Expected Type: " << typeID << std::endl;
                CryoDataType argDataType = argNode->data.varDecl->type;

                if (!argValue)
                {
                    debugger.logMessage("ERROR", __LINE__, "Functions", "Argument value not found");
                    std::cout << "Argument Name: " << argName << std::endl;
                    std::cout << "Argument Node Type: " << CryoNodeTypeToString(argType) << std::endl;
                    // Try to find the variable in the local scope
                    llvm::Value *localVar = variables.getVariable(argName);
                    if (!localVar)
                    {
                        debugger.logMessage("ERROR", __LINE__, "Functions", "Local Variable not found");
                        CONDITION_FAILED;
                    }
                    argValue = localVar;
                    argValues[i] = argValue;
                }

                // If the argument is not the expected type, cast it
                if (argValue->getType()->getTypeID() != expectedType->getTypeID())
                {
                    std::string valName = std::string(argNode->data.varDecl->name);
                    switch (argDataType)
                    {
                    case DATA_TYPE_INT:
                    {
                        llvm::Value *castedValue = types.castTyToVal(argValue, expectedType);
                        argValues[i] = castedValue;
                        break;
                    }
                    case DATA_TYPE_STRING:
                    {
                        break;
                    }
                    default:
                    {
                        debugger.logMessage("ERROR", __LINE__, "Functions", "Unknown argument type");
                        std::cout << "Received: " << CryoDataTypeToString(argDataType) << std::endl;
                        CONDITION_FAILED;
                    }
                    }
                }
                // If the argument is the expected type, continue
                else
                {
                    debugger.logMessage("INFO", __LINE__, "Functions", "Argument is the expected type");
                    debugger.logLLVMValue(argValue);
                    continue;
                }
            }

            // Get the function
            llvm::Function *function = compiler.getContext().module->getFunction(functionName);
            if (!function)
            {
                debugger.logMessage("ERROR", __LINE__, "Functions", "Function not found");
                CONDITION_FAILED;
            }
        }

        // If there are no arguments, just create the function call
        llvm::Function *function = compiler.getContext().module->getFunction(functionName);
        if (!function)
        {
            debugger.logMessage("ERROR", __LINE__, "Functions", "Function not found");
            CONDITION_FAILED;
        }

        llvm::Value *functionCall = compiler.getContext().builder.CreateCall(function, argValues);

        if (!functionCall)
        {
            debugger.logMessage("ERROR", __LINE__, "Functions", "Function call not created");
            CONDITION_FAILED;
        }

        debugger.logMessage("INFO", __LINE__, "Functions", "Function Call Created");

        return functionCall;
    }

    llvm::Value *Functions::createParameter(llvm::Argument *param, llvm::Type *argTypes)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Types &types = compiler.getTypes();
        Variables &variables = compiler.getVariables();
        debugger.logMessage("INFO", __LINE__, "Functions", "Creating Parameter");

        llvm::Value *resultParam = nullptr;
        std::string paramName = param->getName().str() + ".addr";
        std::cout << "Parameter Name: " << param->getName().str() << std::endl;
        std::cout << "Parameter Type: " << std::endl;
        std::cout << "Argument Type: " << std::endl;

        llvm::AllocaInst *alloca = compiler.getContext().builder.CreateAlloca(argTypes, param, paramName);
        compiler.getContext().namedValues[paramName] = alloca;
        // compiler.getContext().builder.CreateStore(param, alloca);

        resultParam = alloca;

        debugger.logLLVMValue(resultParam);

        debugger.logMessage("INFO", __LINE__, "Functions", "Parameter Created");

        return resultParam;
    }
} // namespace Cryo
