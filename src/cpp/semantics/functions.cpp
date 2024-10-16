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
#include "cpp/codegen.hpp"

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
        case NODE_SCOPED_FUNCTION_CALL:
        {
            debugger.logMessage("INFO", __LINE__, "Functions", "Handling Scoped Function Call");
            createScopedFunctionCall(node);
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
        BackendSymTable &symTable = compiler.getSymTable();
        std::string currentNamespace = cryoContext.currentNamespace;
        debugger.logMessage("INFO", __LINE__, "Functions", "Creating Function Declaration");

        FunctionDeclNode *functionNode = node->data.functionDecl;
        assert(functionNode != nullptr);

        // Get the function name
        char *functionName = functionNode->name;
        debugger.logMessage("INFO", __LINE__, "Functions", "Function Name: " + std::string(functionName));

        // Get the return type
        CryoDataType returnType = functionNode->returnType;
        llvm::Type *returnLLVMType = types.getReturnType(returnType);
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
        llvm::FunctionType *functionType = llvm::FunctionType::get(returnLLVMType, argTypes, false);

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
            std::string paramName = std::string(functionNode->params[i]->data.param->name);
            std::cout << "Function Param Name: " << paramName << std::endl;
            ASTNode *paramNode = functionNode->params[i];
            arg.setName(paramName);
            llvm::Value *param = createParameter(&arg, argTypes[i], paramNode);
            compiler.getContext().namedValues[paramName] = &arg;

            symTable.addParameter(currentNamespace, paramName, paramNode);
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

            switch (nodeType)
            {
            case NODE_VAR_DECLARATION:
            {
                debugger.logMessage("INFO", __LINE__, "Functions", "Creating Variable Declaration");
                variables.createLocalVariable(statement);
                break;
            }
            case NODE_FUNCTION_CALL:
            {
                debugger.logMessage("INFO", __LINE__, "Functions", "Creating Function Call");
                createFunctionCall(statement);
                break;
            }
            case NODE_RETURN_STATEMENT:
            {
                debugger.logMessage("INFO", __LINE__, "Functions", "Creating Return Statement");
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
                    CryoNodeType returnTypeNode = statement->data.returnStatement->expression->metaData->type;
                    std::cout << "Return Type Node: " << CryoNodeTypeToString(returnTypeNode) << std::endl;
                    switch (returnTypeNode)
                    {
                    case NODE_VAR_NAME:
                    {
                        debugger.logMessage("INFO", __LINE__, "Functions", "Returning Var Name with type int");
                        std::string varName = statement->data.returnStatement->expression->data.varName->varName;
                        STVariable *stVarNode = symTable.getVariable(currentNamespace, varName);
                        if (!stVarNode)
                        {
                            debugger.logMessage("ERROR", __LINE__, "Functions", "Failed to get variable node");
                            CONDITION_FAILED;
                        }
                        llvm::Value *varValue = stVarNode->LLVMValue;
                        if (!varValue)
                        {
                            debugger.logMessage("ERROR", __LINE__, "Functions", "Failed to get variable value");
                            CONDITION_FAILED;
                        }
                        llvm::Instruction *inst = llvm::dyn_cast<llvm::Instruction>(varValue);
                        llvm::Type *instTy = types.parseInstForType(inst);

                        llvm::Value *returnValue = compiler.getContext().builder.CreateLoad(instTy, varValue, varName + ".retload");
                        compiler.getContext().builder.CreateRet(returnValue);
                        break;
                    }
                    case NODE_LITERAL_EXPR:
                    {
                        debugger.logMessage("INFO", __LINE__, "Functions", "Returning Literal Expression with type int");
                        llvm::Value *returnValue = generator.getInitilizerValue(statement);
                        compiler.getContext().builder.CreateRet(returnValue);
                        break;
                    }
                    case NODE_BINARY_EXPR:
                    {
                        debugger.logMessage("INFO", __LINE__, "Functions", "Returning Binary Expression with type int");
                        llvm::Value *returnValue = generator.getInitilizerValue(statement);
                        compiler.getContext().builder.CreateRet(returnValue);
                        break;
                    }
                    default:
                    {
                        std::cout << "Unknown return type node: " << CryoNodeTypeToString(returnTypeNode) << std::endl;
                        CONDITION_FAILED;
                    }
                    }
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

                break;
            }
            default:
            {
                compiler.getGenerator().parseTree(statement);
            }
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

        CryoNodeType nodeType = returnNode->expression->metaData->type;
        std::cout << "Return Node Type: " << CryoNodeTypeToString(nodeType) << std::endl;

        switch (nodeType)
        {
        case NODE_LITERAL_EXPR:
        {
            debugger.logMessage("INFO", __LINE__, "Functions", "Creating Literal Expression");
            llvm::Value *returnValue = generator.getInitilizerValue(node);
            cryoContext.builder.CreateRet(returnValue);
            break;
        }
        case NODE_VAR_NAME:
        {
            debugger.logMessage("INFO", __LINE__, "Functions", "Creating Var Name");
            std::string varName = returnNode->expression->data.varName->varName;
            STVariable *stVarNode = compiler.getSymTable().getVariable(cryoContext.currentNamespace, varName);
            if (!stVarNode)
            {
                debugger.logMessage("ERROR", __LINE__, "Functions", "Failed to get variable node");
                CONDITION_FAILED;
            }

            llvm::Value *varValue = stVarNode->LLVMValue;
            if (!varValue)
            {
                debugger.logMessage("ERROR", __LINE__, "Functions", "Failed to get variable value");
                CONDITION_FAILED;
            }

            llvm::Instruction *inst = llvm::dyn_cast<llvm::Instruction>(varValue);
            llvm::Type *instTy = types.parseInstForType(inst);

            llvm::Value *returnValue = cryoContext.builder.CreateLoad(instTy, varValue, varName + ".retload");
            cryoContext.builder.CreateRet(returnValue);

            break;
        }
        case NODE_BINARY_EXPR:
        {
            debugger.logMessage("INFO", __LINE__, "Functions", "Creating Binary Expression");
            llvm::Value *returnValue = generator.getInitilizerValue(node);
            cryoContext.builder.CreateRet(returnValue);
            break;
        }
        default:
        {
            debugger.logMessage("ERROR", __LINE__, "Functions", "Unknown node type");
            std::cout << "Received: " << CryoNodeTypeToString(nodeType) << std::endl;
            CONDITION_FAILED;
        }
        }
        return;
    }

    llvm::Value *Functions::createReturnNode(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Functions", "Creating Return Statement");

        if (node->metaData->type != NODE_RETURN_STATEMENT)
        {
            debugger.logMessage("ERROR", __LINE__, "Functions", "Node is not a return statement");
            CONDITION_FAILED;
        }

        CryoReturnNode *returnNode = node->data.returnStatement;
        assert(returnNode != nullptr);

        CryoDataType returnType = returnNode->returnType;
        debugger.logMessage("INFO", __LINE__, "Functions", "Return Type: " + std::string(CryoDataTypeToString(returnType)));

        ASTNode *returnExpression = returnNode->expression;
        if (!returnExpression)
        {
            debugger.logMessage("ERROR", __LINE__, "Functions", "Return expression is null");
            CONDITION_FAILED;
        }

        std::cout << "\n\nReturn Expression: " << std::endl;
        std::cout << "Return Type: " << CryoDataTypeToString(returnType) << std::endl;
        std::cout << "Return Expression Type: " << CryoNodeTypeToString(returnExpression->metaData->type) << std::endl;
        debugger.logNode(returnExpression);
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
                    llvm::Type *retType = types.getType(DATA_TYPE_INT, 0);
                    return retType;
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
        BackendSymTable &symTable = compiler.getSymTable();
        std::string namespaceName = compiler.getContext().currentNamespace;
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
            std::string paramName = functionNode->params[i]->data.param->name;
            ASTNode *paramNode = functionNode->params[i];
            arg.setName(paramName);
            symTable.addParameter(namespaceName, paramName, paramNode);
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

        // STFunction *stFunction = compiler.getSymTable().getFunction(compiler.getContext().currentNamespace, functionName);

        // Get the argument values
        std::vector<llvm::Value *> argValues;
        for (int i = 0; i < argCount; ++i)
        {
            debugger.logMessage("INFO", __LINE__, "Functions", "Processing Argument " + std::to_string(i + 1) + " of " + std::to_string(argCount));
            ASTNode *argNode = functionCallNode->args[i];
            CryoNodeType argNodeType = argNode->metaData->type;

            std::cout << "===----------------------===" << std::endl;
            std::cout << "Argument #: " << i + 1 << std::endl;
            std::cout << "Function Name: " << functionName << std::endl;
            std::cout << "Argument Node Type: " << CryoNodeTypeToString(argNodeType) << std::endl;
            std::cout << "===----------------------===" << std::endl;

            std::string funcName = std::string(functionName);
            std::cout << "\n\nFunction Name: " << funcName << "\n";

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

            // Get the current callee function return type
            llvm::Type *returnType = calleeF->getReturnType();

            // Get the argument value
            switch (argNodeType)
            {
            case NODE_VAR_NAME:
            {
                debugger.logMessage("INFO", __LINE__, "Functions", "Argument is a variable name");
                VariableNameNode *varNameNode = argNode->data.varName;
                assert(varNameNode != nullptr);

                debugger.logMessage("INFO", __LINE__, "Functions", "Argument Variable Name: " + std::string(varNameNode->varName));

                llvm::Value *argNode = createVarNameCall(varNameNode);
                if (!argNode)
                {
                    debugger.logMessage("ERROR", __LINE__, "Functions", "Argument value not found");
                    CONDITION_FAILED;
                }

                argValues.push_back(argNode);
                break;
            }
            case NODE_LITERAL_EXPR:
            {
                debugger.logMessage("INFO", __LINE__, "Functions", "Argument is a literal expression");
                LiteralNode *literalNode = argNode->data.literal;
                assert(literalNode != nullptr);

                llvm::Value *argNode = createLiteralCall(literalNode);
                if (!argNode)
                {
                    debugger.logMessage("ERROR", __LINE__, "Functions", "Argument value not found");
                    CONDITION_FAILED;
                }

                argValues.push_back(argNode);
                break;
            }
            case NODE_VAR_DECLARATION:
            {
                debugger.logMessage("INFO", __LINE__, "Functions", "Argument is a variable declaration");
                CryoVariableNode *varNode = argNode->data.varDecl;
                assert(varNode != nullptr);

                llvm::Value *argNode = createVarDeclCall(varNode);
                if (!argNode)
                {
                    debugger.logMessage("ERROR", __LINE__, "Functions", "Argument value not found");
                    CONDITION_FAILED;
                }

                argValues.push_back(argNode);
                break;
            }
            case NODE_ARRAY_LITERAL:
            {
                debugger.logMessage("INFO", __LINE__, "Functions", "Argument is an array literal");
                CryoArrayNode *arrayNode = argNode->data.array;
                assert(arrayNode != nullptr);

                llvm::Value *argNode = createArrayCall(arrayNode);
                if (!argNode)
                {
                    debugger.logMessage("ERROR", __LINE__, "Functions", "Argument value not found");
                    CONDITION_FAILED;
                }

                argValues.push_back(argNode);
                break;
            }
            case NODE_INDEX_EXPR:
            {
                debugger.logMessage("INFO", __LINE__, "Functions", "Argument is an index expression");
                IndexExprNode *indexNode = argNode->data.indexExpr;
                assert(indexNode != nullptr);

                llvm::Value *argNode = createIndexExprCall(indexNode);
                if (!argNode)
                {
                    debugger.logMessage("ERROR", __LINE__, "Functions", "Argument value not found");
                    CONDITION_FAILED;
                }

                argValues.push_back(argNode);
                break;
            }
            default:
            {
                debugger.logMessage("ERROR", __LINE__, "Functions", "Unknown argument type");
                std::cout << "Received: " << CryoNodeTypeToString(argNodeType) << std::endl;
                CONDITION_FAILED;
            }
            }
        }

        // If there are no arguments, just create the function call
        if (argCount == 0)
        {
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

        // If there are arguments, create the function call with the arguments
        // We will verify the arguments in the function call
        llvm::Function *function = compiler.getContext().module->getFunction(functionName);
        std::vector<llvm::Value *> verifiedArgs = verifyCalleeArguments(function, argValues);

        llvm::Value *functionCall = compiler.getContext().builder.CreateCall(function, verifiedArgs);
        if (!functionCall)
        {
            debugger.logMessage("ERROR", __LINE__, "Functions", "Function call not created");
            CONDITION_FAILED;
        }

        debugger.logMessage("INFO", __LINE__, "Functions", "Function Call Created");

        return functionCall;
    }

    llvm::Value *Functions::createParameter(llvm::Argument *param, llvm::Type *argTypes, ASTNode *paramNode)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Types &types = compiler.getTypes();
        Variables &variables = compiler.getVariables();
        BackendSymTable &symTable = compiler.getSymTable();
        std::string namespaceName = compiler.getContext().currentNamespace;
        debugger.logMessage("INFO", __LINE__, "Functions", "Creating Parameter");

        std::string paramName = param->getName().str() + ".ptr";
        std::cout << "Parameter Name: " << param->getName().str() << std::endl;
        std::cout << "Parameter Type: " << std::endl;
        std::cout << "Argument Type: " << std::endl;

        // llvm::LoadInst *loadInst = compiler.getContext().builder.CreateLoad(argTypes, param, paramName);
        llvm::AllocaInst *alloca = compiler.getContext().builder.CreateAlloca(argTypes, nullptr, paramName);
        alloca->setAlignment(llvm::Align(8));

        llvm::StoreInst *storeInst = compiler.getContext().builder.CreateStore(param, alloca);
        storeInst->setAlignment(llvm::Align(8));
        // Load the value of the parameter
        // llvm::Value *loadInst = compiler.getContext().builder.CreateLoad(argTypes, alloca, paramName + ".load");

        std::string _paramName = param->getName().str();
        compiler.getContext().namedValues[param->getName().str()] = alloca;

        symTable.addParamAsVariable(namespaceName, _paramName, alloca, argTypes, storeInst);

        debugger.logMessage("INFO", __LINE__, "Functions", "Parameter Created");

        return alloca;
    }

    llvm::Value *Functions::anyTypeParam(std::string functionName, llvm::Value *argValue)
    {
        CryoDebugger &debugger = compiler.getDebugger();
    }

    void Functions::createScopedFunctionCall(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Functions", "Creating Scoped Function Call");

        ScopedFunctionCallNode *functionCallNode = node->data.scopedFunctionCall;
        assert(functionCallNode != nullptr);

        // Get the function name
        char *functionName = functionCallNode->functionName;
        debugger.logMessage("INFO", __LINE__, "Functions", "Function Name: " + std::string(functionName));

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
                debugger.logMessage("INFO", __LINE__, "Functions", "Creating Variable Declaration");
                llvm::Value *argValue = compiler.getGenerator().getInitilizerValue(argNode);
                if (!argValue)
                {
                    debugger.logMessage("ERROR", __LINE__, "Functions", "Argument value not found");
                    CONDITION_FAILED;
                }
                argValues.push_back(argValue);
                break;
            }
            case NODE_LITERAL_EXPR:
            {
                debugger.logMessage("INFO", __LINE__, "Functions", "Creating Literal Expression");
                llvm::Value *argValue = compiler.getGenerator().getInitilizerValue(argNode);
                if (!argValue)
                {
                    debugger.logMessage("ERROR", __LINE__, "Functions", "Argument value not found");
                    CONDITION_FAILED;
                }
                // Create a new variable for the literal
                llvm::Value *literalVar = compiler.getVariables().createLocalVariable(argNode);
                if (!literalVar)
                {
                    debugger.logMessage("ERROR", __LINE__, "Functions", "Literal variable not created");
                    CONDITION_FAILED;
                }

                // Store the literal value in the variable
                compiler.getContext().builder.CreateStore(argValue, literalVar);
                argValues.push_back(literalVar);
                break;
            }
            default:
            {
                debugger.logMessage("ERROR", __LINE__, "Functions", "Unknown argument type");
                CONDITION_FAILED;
            }
            }
        }

        // Get the function
        llvm::Function *function = compiler.getContext().module->getFunction(functionName);
        if (!function)
        {
            debugger.logMessage("ERROR", __LINE__, "Functions", "Function not found");
            CONDITION_FAILED;
        }

        // If there are no arguments, just create the function call
        llvm::Value *functionCall = compiler.getContext().builder.CreateCall(function, argValues);
        if (!functionCall)
        {
            debugger.logMessage("ERROR", __LINE__, "Functions", "Function call not created");
            CONDITION_FAILED;
        }

        debugger.logMessage("INFO", __LINE__, "Functions", "Scoped Function Call Created");

        return;
    }

    /// ### ============================================================================= ###
    /// ###
    /// ### Specialized Functions (For Function Calls Specifically)
    /// ### These functions are used to handle specific types of function calls
    /// ###
    /// ### ============================================================================= ###

    llvm::Value *Functions::createVarNameCall(VariableNameNode *varNameNode)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Variables &variables = compiler.getVariables();
        Types &types = compiler.getTypes();
        debugger.logMessage("INFO", __LINE__, "Functions", "Creating Variable Name Call");

        std::string varName = std::string(varNameNode->varName);
        std::string namespaceName = compiler.getContext().currentNamespace;

        STVariable *var = compiler.getSymTable().getVariable(namespaceName, varName);
        if (!var)
        {
            debugger.logMessage("ERROR", __LINE__, "Functions", "Variable not found");
            CONDITION_FAILED;
        }

        std::cout << "@createVarNameCall Variable Name: " << varName << std::endl;
        llvm::Value *varValue = var->LLVMValue;
        if (!varValue)
        {
            debugger.logMessage("ERROR", __LINE__, "Functions", "Variable value not found");
            CONDITION_FAILED;
        }

        llvm::StoreInst *storeInst = var->LLVMStoreInst;
        if (!storeInst)
        {
            debugger.logMessage("ERROR", __LINE__, "Functions", "Store instruction not found");
            CONDITION_FAILED;
        }

        llvm::Instruction *inst = llvm::dyn_cast<llvm::Instruction>(storeInst);
        llvm::Type *varInstType = types.parseInstForType(inst);

        // This is to dereference an integer type
        bool isIntType = varInstType->isIntegerTy();
        if (varValue->getType()->isPointerTy() && isIntType)
        {
            llvm::Instruction *inst = llvm::dyn_cast<llvm::Instruction>(storeInst);
            std::cout << "Instruction: " << std::endl;
            debugger.logLLVMInst(inst);
            llvm::Type *varInstType = types.parseInstForType(inst);
            llvm::LoadInst *varLoadValue = compiler.getContext().builder.CreateLoad(varInstType, varValue, varName + ".load.funcCall");
            if (!varLoadValue)
            {
                debugger.logMessage("ERROR", __LINE__, "Functions", "Variable value not loaded");
                CONDITION_FAILED;
            }
            varLoadValue->setAlignment(llvm::Align(8));

            debugger.logMessage("INFO", __LINE__, "Functions", "Var Name Call Created with dereference. For Variable: " + varName);
            return varLoadValue;
        }

        debugger.logMessage("INFO", __LINE__, "Functions", "Var Name Call Created without dereference. For Variable: " + varName);
        return varValue;
    }

    llvm::Value *Functions::createVarDeclCall(CryoVariableNode *varDeclNode)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        BackendSymTable &symTable = compiler.getSymTable();
        debugger.logMessage("INFO", __LINE__, "Functions", "Creating Variable Declaration Call");

        std::string varName = std::string(varDeclNode->name);

        std::string namespaceName = compiler.getContext().currentNamespace;

        STVariable *var = compiler.getSymTable().getVariable(namespaceName, varName);
        if (!var)
        {
            debugger.logMessage("ERROR", __LINE__, "Functions", "Variable not found");
            CONDITION_FAILED;
        }

        std::cout << "@createVarDeclCall Variable Name: " << varName << std::endl;
        STVariable *varValueNode = symTable.getVariable(namespaceName, varName);
        if (!varValueNode)
        {
            debugger.logMessage("ERROR", __LINE__, "Functions", "Variable not found");
            CONDITION_FAILED;
        }
        llvm::Value *varValue = varValueNode->LLVMValue;
        if (!varValue)
        {
            debugger.logMessage("ERROR", __LINE__, "Functions", "Variable value not found, creating variable");
            CryoNodeType nodeType = varDeclNode->initializer->metaData->type;
            std::cout << "Node Type of VarDecl: " << CryoNodeTypeToString(nodeType) << std::endl;
            CryoDataType dataType = varDeclNode->type;
            std::cout << "Data Type of VarDecl: " << CryoDataTypeToString(dataType) << std::endl;

            if (nodeType == NODE_LITERAL_EXPR)
            {
                llvm::Value *varValue = compiler.getGenerator().getLiteralValue(varDeclNode->initializer->data.literal);
                if (!varValue)
                {
                    debugger.logMessage("ERROR", __LINE__, "Functions", "Variable value not created");
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
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Functions", "Creating Literal Call");

        llvm::Value *literalValue = compiler.getGenerator().getLiteralValue(literalNode);
        if (!literalValue)
        {
            debugger.logMessage("ERROR", __LINE__, "Functions", "Literal value not found");
            CONDITION_FAILED;
        }

        CryoDataType dataType = literalNode->dataType;
        switch (dataType)
        {
        case DATA_TYPE_INT:
        {
            // Create the integer literal
            debugger.logMessage("INFO", __LINE__, "Functions", "Creating Integer Literal");
            llvm::Type *literalType = compiler.getTypes().getType(DATA_TYPE_INT, 0);
            int literalValue = literalNode->value.intValue;
            llvm::Value *literalInt = llvm::ConstantInt::get(literalType, literalValue, true);
            llvm::Value *literalVarPtr = compiler.getContext().builder.CreateAlloca(literalType, nullptr, "literal.int.ptr");
            if (!literalVarPtr)
            {
                debugger.logMessage("ERROR", __LINE__, "Functions", "Literal variable not created");
                CONDITION_FAILED;
            }

            llvm::Value *literalVarStore = compiler.getContext().builder.CreateStore(literalInt, literalVarPtr);

            llvm::LoadInst *literalVar = compiler.getContext().builder.CreateLoad(literalType, literalVarPtr, "lit.int.load.funcCall");
            if (!literalVar)
            {
                debugger.logMessage("ERROR", __LINE__, "Functions", "Literal variable not loaded");
                CONDITION_FAILED;
            }
            literalVar->setAlignment(llvm::Align(8));

            // Add the literal to the named values
            std::string literalName = literalVarPtr->getName().str();
            compiler.getContext().namedValues[literalName] = literalVarPtr;

            return literalVar;
        }
        case DATA_TYPE_STRING:
        {
            debugger.logMessage("INFO", __LINE__, "Functions", "Creating String Literal");
            llvm::Value *literalVarPtr = compiler.getContext().builder.CreateAlloca(literalValue->getType(), nullptr, "literal.str.ptr");
            if (!literalVarPtr)
            {
                debugger.logMessage("ERROR", __LINE__, "Functions", "Literal variable not created");
                CONDITION_FAILED;
            }
            llvm::Value *literalVar = compiler.getContext().builder.CreateStore(literalValue, literalVarPtr);
            if (!literalVar)
            {
                debugger.logMessage("ERROR", __LINE__, "Functions", "Literal variable not stored");
                CONDITION_FAILED;
            }

            return literalVarPtr;
        }
        case DATA_TYPE_BOOLEAN:
        {
            debugger.logMessage("INFO", __LINE__, "Functions", "Creating Boolean Literal");
            llvm::Type *literalType = compiler.getTypes().getType(DATA_TYPE_BOOLEAN, 0);
            bool literalValue = literalNode->value.booleanValue;
            llvm::Value *literalBool = llvm::ConstantInt::get(literalType, literalValue, true);
            llvm::Value *literalVarPtr = compiler.getContext().builder.CreateAlloca(literalType, nullptr, "literal.bool.ptr");
            if (!literalVarPtr)
            {
                debugger.logMessage("ERROR", __LINE__, "Functions", "Literal variable not created");
                CONDITION_FAILED;
            }

            llvm::Value *literalVarStore = compiler.getContext().builder.CreateStore(literalBool, literalVarPtr);

            llvm::LoadInst *literalVar = compiler.getContext().builder.CreateLoad(literalType, literalVarPtr, "lit.bool.load.funcCall");
            if (!literalVar)
            {
                debugger.logMessage("ERROR", __LINE__, "Functions", "Literal variable not loaded");
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
            debugger.logMessage("ERROR", __LINE__, "Functions", "Unknown literal type");
            CONDITION_FAILED;
        }
        }

        return nullptr;
    }

    llvm::Value *Functions::createIndexExprCall(IndexExprNode *indexNode)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Functions", "Creating Index Expression Call");

        DEBUG_BREAKPOINT;
    }

    llvm::Value *Functions::createFunctionCallCall(FunctionCallNode *functionCallNode)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Functions", "Creating Function Call Call");

        DEBUG_BREAKPOINT;
    }

    llvm::Value *Functions::createArrayCall(CryoArrayNode *arrayNode)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Functions", "Creating Array Call");

        DEBUG_BREAKPOINT;
    }

    /// ### ============================================================================= ###
    /// ###
    /// ### General Utility Functions
    /// ### Some of these are used to access and manipulate functions
    /// ###
    /// ### ============================================================================= ###

    std::vector<llvm::Value *> Functions::verifyCalleeArguments(llvm::Function *callee, const std::vector<llvm::Value *> &argValues)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Functions", "Verifying Callee Arguments");

        std::vector<llvm::Value *> verifiedArgs;
        llvm::FunctionType *calleeFT = callee->getFunctionType();
        int argCount = calleeFT->getNumParams();
        if (argCount != argValues.size())
        {
            debugger.logMessage("ERROR", __LINE__, "Functions", "Argument count mismatch");
            CONDITION_FAILED;
        }

        std::vector<llvm::Type *> expectedTypes;
        for (int i = 0; i < argCount; ++i)
        {
            llvm::Type *expectedType = calleeFT->getParamType(i);
            expectedTypes.push_back(expectedType);
        }

        // Find the orginal parameter types
        std::string namespaceName = compiler.getContext().currentNamespace;

        for (int i = 0; i < argCount; ++i)
        {
            std::string paramName = callee->getArg(i)->getName().str();
            STParameter *paramNode = compiler.getSymTable().getParameter(namespaceName, paramName);
            if (!paramNode)
            {
                debugger.logMessage("ERROR", __LINE__, "Functions", "Parameter not found");
                CONDITION_FAILED;
            }

            llvm::Value *argValue = argValues[i];
            llvm::Type *expectedType = expectedTypes[i];

            if (paramNode->dataType == DATA_TYPE_ANY)
            {
                // With a function parameter being `any` type, it only accepts pointers to any type
                llvm::Value *argValue = argValues[i];
                llvm::Type *argType = argValue->getType();
                if (!argType->isPointerTy())
                {
                    // If the argument isn't a pointer, we need to get the pointer of the variable
                    // Not the load instruction
                    // Check if it alreadyd exists in the variable symbol table:
                    std::string argName = argValue->getName().str();
                    std::cout << "Argument Name: " << argName << std::endl;
                    llvm::LoadInst *loadInst = llvm::dyn_cast<llvm::LoadInst>(argValue);
                    std::string argOperandName = loadInst->getOperand(0)->getName().str();
                    std::cout << "Argument Operand Name: " << argOperandName << std::endl;
                    llvm::Value *argPtr = compiler.getContext().namedValues[argOperandName];
                    if (!argPtr)
                    {
                        debugger.logMessage("ERROR", __LINE__, "Functions", "Argument pointer not found");
                        CONDITION_FAILED;
                    }

                    verifiedArgs.push_back(argPtr);

                    continue;
                }
                else
                {
                    verifiedArgs.push_back(argValue);
                }

                continue;
            }

            if (argValue->getType()->getTypeID() != expectedType->getTypeID())
            {
                debugger.logMessage("ERROR", __LINE__, "Functions", "Argument type mismatch");

                // Cast the argument to the expected type
                // llvm::Value *castValue = createArgCast(argValue, expectedType);
                verifiedArgs.push_back(argValue);
                continue;
            }
            else
            {
                debugger.logMessage("INFO", __LINE__, "Functions", "Argument type matches expected type");
                verifiedArgs.push_back(argValue);
            }
        }

        debugger.logMessage("INFO", __LINE__, "Functions", "Callee Arguments Verified");

        return verifiedArgs;
    }

    llvm::Value *Functions::createArgCast(llvm::Value *argValue, llvm::Type *expectedType)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Functions", "Creating Argument Cast");

        llvm::Value *castValue = nullptr;
        llvm::Type *sourceType = argValue->getType();

        if (sourceType == expectedType)
        {
            debugger.logMessage("INFO", __LINE__, "Functions", "Argument type matches expected type");
            return argValue;
        }

        llvm::IRBuilder<> &builder = compiler.getContext().builder;

        if (sourceType->isIntegerTy() && expectedType->isIntegerTy())
        {
            // Integer to integer cast
            if (sourceType->getIntegerBitWidth() < expectedType->getIntegerBitWidth())
            {
                castValue = builder.CreateZExt(argValue, expectedType, "zext_cast");
            }
            else
            {
                castValue = builder.CreateTrunc(argValue, expectedType, "trunc_cast");
            }
        }
        else if (sourceType->isFloatingPointTy() && expectedType->isFloatingPointTy())
        {
            // Float to float cast
            castValue = builder.CreateFPCast(argValue, expectedType, "fp_cast");
        }
        else if (sourceType->isIntegerTy() && expectedType->isFloatingPointTy())
        {
            // Integer to float cast
            castValue = builder.CreateSIToFP(argValue, expectedType, "int_to_fp_cast");
        }
        else if (sourceType->isFloatingPointTy() && expectedType->isIntegerTy())
        {
            // Float to integer cast
            castValue = builder.CreateFPToSI(argValue, expectedType, "fp_to_int_cast");
        }
        else if (sourceType->isPointerTy() && expectedType->isPointerTy())
        {
            // Pointer to pointer cast
            castValue = builder.CreatePointerCast(argValue, expectedType, "ptr_cast");
        }
        else if (sourceType->isArrayTy() && expectedType->isPointerTy())
        {
            // Array to pointer cast
            castValue = builder.CreatePointerCast(argValue, expectedType, "array_to_ptr_cast");
        }
        else
        {
            // Fallback to bitcast for other cases
            debugger.logMessage("WARNING", __LINE__, "Functions", "Using bitcast as fallback");
            castValue = argValue;
        }

        debugger.logMessage("INFO", __LINE__, "Functions", "Argument Cast Created");
        return castValue;
    }

} // namespace Cryo