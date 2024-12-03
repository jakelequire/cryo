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
    void Functions::handleFunction(ASTNode *node)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Handling Function");

        CryoNodeType nodeType = node->metaData->type;
        switch (nodeType)
        {
        case NODE_FUNCTION_DECLARATION:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Functions", "Handling Function Declaration");
            createFunctionDeclaration(node);
            break;
        }
        case NODE_FUNCTION_BLOCK:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Functions", "Handling Function Block");
            createFunctionBlock(node);
            break;
        }
        case NODE_FUNCTION_CALL:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Functions", "Handling Function Call");
            createFunctionCall(node);
            break;
        }
        case NODE_RETURN_STATEMENT:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Functions", "Handling Return Statement");
            createReturnStatement(node);
            break;
        }
        case NODE_EXTERN_FUNCTION:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Functions", "Handling Extern Function");
            createExternFunction(node);
            break;
        }
        case NODE_SCOPED_FUNCTION_CALL:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Functions", "Handling Scoped Function Call");
            createScopedFunctionCall(node);
            break;
        }
        default:
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Unknown node type");
            CONDITION_FAILED;
        }
    }

    llvm::Function *Functions::getFunction(std::string functionName)
    {
        CryoContext &context = compiler.getContext();
        llvm::Function *function = context.module->getFunction(functionName);
        if (!function)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Function not found");
            CONDITION_FAILED;
        }
        return function;
    }

    // -----------------------------------------------------------------------------------------------

    void Functions::createFunctionDeclaration(ASTNode *node)
    {
        CryoContext &cryoContext = compiler.getContext();
        Variables &variables = compiler.getVariables();
        Generator &generator = compiler.getGenerator();
        OldTypes &types = compiler.getTypes();
        IRSymTable &symTable = compiler.getSymTable();
        std::string currentNamespace = cryoContext.currentNamespace;
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Function Declaration");

        FunctionDeclNode *functionNode = node->data.functionDecl;
        assert(functionNode != nullptr);

        // Get the function name
        std::string functionName = std::string(functionNode->name);
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Function Name: " + std::string(functionName));

        // Check if the function is the main function
        if (functionName.compare("main") == 0)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Functions", "Main Function Found, creating entry point");
            this->createCryoMainFunction(node);
            return;
        }

        // Get the return type
        DataType *returnType = functionNode->type;
        llvm::Type *returnLLVMType = types.getReturnType(returnType);
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Return Type: " + std::string(DataTypeToString(returnType)));

        // Get the function arguments
        int argCount = functionNode->paramCount;
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Argument Count: " + std::to_string(argCount));

        std::vector<llvm::Type *> argTypes;
        for (int i = 0; i < argCount; ++i)
        {
            CryoParameterNode *argNode = functionNode->params[i]->data.param;
            assert(argNode != nullptr);

            DataType *_argType = argNode->type;
            if (_argType->container->baseType == PRIM_STRING)
            {
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Converting string to LLVM type");
                // int _len = types.getLiteralValLength(argNode->initializer);
                // Need to find a solution to get the length of the string, setting the type as a pointer might be a temp thing.
                llvm::Type *argType = types.getType(_argType, 32);
                argTypes.push_back(argType->getPointerTo());
                continue;
            }

            if (_argType->container->baseType == PRIM_INT)
            {
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Converting int to LLVM type");
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
            DevDebugger::logMessage("INFO", __LINE__, "Functions", "Parsing Statement " + std::to_string(i + 1) + " of " + std::to_string(functionBody->data.functionBlock->statementCount));
            ASTNode *statement = functionBody->data.functionBlock->statements[i];
            CryoNodeType nodeType = statement->metaData->type;
            std::cout << "Statement: " << CryoNodeTypeToString(statement->metaData->type) << std::endl;

            switch (nodeType)
            {
            case NODE_VAR_DECLARATION:
            {
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Variable Declaration");
                variables.createLocalVariable(statement);
                break;
            }
            case NODE_FUNCTION_CALL:
            {
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Function Call");
                createFunctionCall(statement);
                break;
            }
            case NODE_RETURN_STATEMENT:
            {
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Return Statement");
                llvm::Type *returnLLVMType = types.getReturnType(returnType);
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Return Type: " + std::string(DataTypeToString(returnType)));

                std::cout << "TypeOfDataType: " << std::string(TypeofDataTypeToString(returnType->container->baseType)) << std::endl;
                std::cout << "PrimitiveType: " << std::string(PrimitiveDataTypeToString(returnType->container->primitive)) << std::endl;

                switch (returnType->container->primitive)
                {
                case PRIM_VOID:
                {
                    DevDebugger::logMessage("INFO", __LINE__, "Functions", "Returning void");
                    compiler.getContext().builder.CreateRet(nullptr);
                    break;
                }
                case PRIM_INT:
                {
                    CryoNodeType returnTypeNode = statement->data.returnStatement->expression->metaData->type;
                    std::cout << "Return Type Node: " << CryoNodeTypeToString(returnTypeNode) << std::endl;
                    switch (returnTypeNode)
                    {
                    case NODE_VAR_NAME:
                    {
                        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Returning Var Name with type int");
                        std::string varName = statement->data.returnStatement->expression->data.varName->varName;
                        STVariable *stVarNode = symTable.getVariable(currentNamespace, varName);
                        if (!stVarNode)
                        {
                            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Failed to get variable node");
                            CONDITION_FAILED;
                        }
                        llvm::Value *varValue = stVarNode->LLVMValue;
                        if (!varValue)
                        {
                            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Failed to get variable value");
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
                        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Returning Literal Expression with type int");
                        llvm::Value *returnValue = generator.getInitilizerValue(statement);
                        compiler.getContext().builder.CreateRet(returnValue);
                        break;
                    }
                    case NODE_BINARY_EXPR:
                    {
                        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Returning Binary Expression with type int");
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
                case PRIM_STRING:
                {
                    DevDebugger::logMessage("INFO", __LINE__, "Functions", "Returning string");
                    int _len = types.getLiteralValLength(statement);
                    llvm::Type *returnType = types.getType(createPrimitiveStringType(_len), _len);
                    llvm::Value *returnValue = generator.getInitilizerValue(statement);
                    compiler.getContext().builder.CreateRet(returnValue);

                    break;
                }
                default:
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Unknown return type");
                    std::cout << "Received: " << DataTypeToString(returnType) << std::endl;
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
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Adding terminator to function");
            CONDITION_FAILED;
        }
        else
        {
            DevDebugger::logMessage("INFO", __LINE__, "Functions", "Function already has a terminator");
        }

        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Function Declaration Created");

        // Exit the scope of the function
        compiler.getContext().builder.ClearInsertionPoint();
        compiler.getContext().currentFunction = nullptr;
        compiler.getContext().inGlobalScope = true;

        return;
    }

    // -----------------------------------------------------------------------------------------------

    void Functions::createFunctionBlock(ASTNode *node)
    {
        OldTypes &types = compiler.getTypes();
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Function Block");

        CryoFunctionBlock *functionBlockNode = node->data.functionBlock;

        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Getting Function Block");

        ASTNode **statements = functionBlockNode->statements;
        int statementCount = functionBlockNode->statementCount;

        for (int i = 0; i < statementCount; ++i)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Functions", "Processing Statement " + std::to_string(i + 1) + " of " + std::to_string(statementCount));
            ASTNode *statement = statements[i];
            if (statement->metaData->type == NODE_RETURN_STATEMENT)
            {
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Return Statement");
                return;
            }
            compiler.getGenerator().parseTree(statement);
        }

        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Function Block Created");

        return;
    }

    // -----------------------------------------------------------------------------------------------

    void Functions::createReturnStatement(ASTNode *node)
    {
        Generator &generator = compiler.getGenerator();
        CryoContext &cryoContext = compiler.getContext();
        if (node->metaData->type != NODE_RETURN_STATEMENT)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Node is not a return statement");
            CONDITION_FAILED;
        }
        OldTypes &types = compiler.getTypes();
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Return Statement");

        CryoReturnNode *returnNode = node->data.returnStatement;
        assert(returnNode != nullptr);

        DataType *returnType = returnNode->type;
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Return Type: " + std::string(DataTypeToString(returnType)));

        if (!returnNode->expression && returnType->container->primitive != PRIM_VOID)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Return expression is null");
            CONDITION_FAILED;
        }
        if (!returnNode->expression && returnType->container->primitive == PRIM_VOID)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Functions", "Return expression is null");
            cryoContext.builder.CreateRetVoid();
            return;
        }

        CryoNodeType nodeType = returnNode->expression->metaData->type;
        std::cout << "Return Node Type: " << CryoNodeTypeToString(nodeType) << std::endl;

        switch (nodeType)
        {
        case NODE_LITERAL_EXPR:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Literal Expression");
            llvm::Value *returnValue = generator.getInitilizerValue(node);
            cryoContext.builder.CreateRet(returnValue);
            break;
        }
        case NODE_VAR_NAME:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Var Name");
            std::string varName = returnNode->expression->data.varName->varName;
            STVariable *stVarNode = compiler.getSymTable().getVariable(cryoContext.currentNamespace, varName);
            if (!stVarNode)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Failed to get variable node");
                CONDITION_FAILED;
            }

            llvm::Value *varValue = stVarNode->LLVMValue;
            if (!varValue)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Failed to get variable value");
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
            DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Binary Expression");
            llvm::Value *returnValue = generator.getInitilizerValue(node);
            cryoContext.builder.CreateRet(returnValue);
            break;
        }
        case NODE_FUNCTION_CALL:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Function Call");
            ASTNode *functionCall = returnNode->expression;
            llvm::Value *funcCallVal = createFunctionCall(functionCall);
            // Return the value of the function call
            llvm::Function *currentFunction = cryoContext.currentFunction;
            llvm::Type *returnType = currentFunction->getReturnType();
            if (returnType->isVoidTy())
            {
                cryoContext.builder.CreateRetVoid();
            }
            else
            {
                cryoContext.builder.CreateRet(funcCallVal);
            }
            break;
        }
        default:
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Unknown node type");
            std::cout << "Received: " << CryoNodeTypeToString(nodeType) << std::endl;
            CONDITION_FAILED;
        }
        }
        return;
    }

    llvm::Value *Functions::createReturnNode(ASTNode *node)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Return Statement");

        if (node->metaData->type != NODE_RETURN_STATEMENT)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Node is not a return statement");
            CONDITION_FAILED;
        }

        CryoReturnNode *returnNode = node->data.returnStatement;
        assert(returnNode != nullptr);

        DataType *returnType = returnNode->type;
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Return Type: " + std::string(DataTypeToString(returnType)));

        ASTNode *returnExpression = returnNode->expression;
        if (!returnExpression)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Return expression is null");
            CONDITION_FAILED;
        }

        llvm::Value *returnValue = compiler.getGenerator().getInitilizerValue(returnExpression);
        if (!returnValue)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Failed to get return value");
            CONDITION_FAILED;
        }

        return returnValue;
    }

    // -----------------------------------------------------------------------------------------------

    /**
     * @brief Traverse the function block to get the return type for the function declaration.
     * (Not the return statement terminator)
     */
    llvm::Type *Functions::traverseBlockReturnType(CryoFunctionBlock *blockNode)
    {
        OldTypes &types = compiler.getTypes();
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Traversing Function Block");

        ASTNode **statements = blockNode->statements;
        llvm::Type *returnType = nullptr;

        for (int i = 0; i < blockNode->statementCount; ++i)
        {
            ASTNode *statement = statements[i];
            if (statement->metaData->type == NODE_RETURN_STATEMENT)
            {
                DataType *nodeDataType = statement->data.returnStatement->type;

                switch (nodeDataType->container->baseType)
                {
                case PRIM_INT:
                {

                    DevDebugger::logMessage("INFO", __LINE__, "Functions", "Returning int");
                    llvm::Type *retType = types.getType(createPrimitiveIntType(), 0);
                    return retType;
                }
                case PRIM_STRING:
                {
                    DevDebugger::logMessage("INFO", __LINE__, "Functions", "Returning string");
                    int _len = types.getLiteralValLength(statement->data.returnStatement->expression);
                    // +1 for the null terminator
                    returnType = types.getType(createPrimitiveStringType(_len), _len + 1);
                    break;
                }
                case PRIM_VOID:
                {
                    DevDebugger::logMessage("INFO", __LINE__, "Functions", "Returning void");
                    returnType = types.getReturnType(createPrimitiveVoidType());
                    break;
                }
                default:
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Unknown return type");
                    std::cout << "Received: " << DataTypeToString(nodeDataType) << std::endl;
                    CONDITION_FAILED;
                }
                }
            }
            if (statement->metaData->type == NODE_BINARY_EXPR)
            {
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "");
                returnType = types.getReturnType(createPrimitiveIntType());
            }
        }
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Function Block Traversed");
        return returnType;
    }

    // -----------------------------------------------------------------------------------------------

    /**
     * @brief Create an extern function.
     * This should only declare the function in the IR, it will not create the function body.
     */
    void Functions::createExternFunction(ASTNode *node)
    {
        IRSymTable &symTable = compiler.getSymTable();
        std::string namespaceName = compiler.getContext().currentNamespace;
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Extern Function");

        ExternFunctionNode *functionNode = node->data.externFunction;

        // Get the function name
        char *functionName = functionNode->name;
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Function Name: " + std::string(functionName));

        // Check if the function already exists
        if (doesExternFunctionExist(functionName))
        {
            // If the function is already found, we will throw a warning but continue
            DevDebugger::logMessage("WARN", __LINE__, "Functions", "Function already exists");
            return;
        }

        // Get the return type
        DataType *returnType = functionNode->type;
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Return Type: " + std::string(DataTypeToString(returnType)));

        // Get the function arguments
        int argCount = functionNode->paramCount;
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Argument Count: " + std::to_string(argCount));

        std::vector<llvm::Type *> argTypes;
        for (int i = 0; i < argCount; ++i)
        {
            CryoParameterNode *argNode = functionNode->params[i]->data.param;
            assert(argNode != nullptr);
            DataType *_argType = argNode->type;

            if (_argType->container->baseType == PRIM_STRING)
            {
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Converting string to LLVM type");
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

        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Extern Function Created");

        return;
    }

    bool Functions::doesExternFunctionExist(std::string functionName)
    {
        llvm::Function *function = compiler.getContext().module->getFunction(functionName);
        if (!function)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Function not found");
            return false;
        }
        else
        {
            DevDebugger::logMessage("INFO", __LINE__, "Functions", "Function found");
            return true;
        }
    }

    // -----------------------------------------------------------------------------------------------

    llvm::Value *Functions::createParameter(llvm::Argument *param, llvm::Type *argTypes, ASTNode *paramNode)
    {
        OldTypes &types = compiler.getTypes();
        Variables &variables = compiler.getVariables();
        IRSymTable &symTable = compiler.getSymTable();
        std::string namespaceName = compiler.getContext().currentNamespace;
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Parameter");

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
        llvm::Value *loadInst = compiler.getContext().builder.CreateLoad(argTypes, alloca, paramName + ".load");

        std::string _paramName = param->getName().str();
        compiler.getContext().namedValues[param->getName().str()] = loadInst;

        symTable.addParamAsVariable(namespaceName, _paramName, loadInst, argTypes, storeInst);
        DataType *paramType = paramNode->data.param->type;
        if (!paramType)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Parameter type not found");
            CONDITION_FAILED;
        }
        symTable.addDataTypeToVar(namespaceName, _paramName, paramType);

        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Parameter Created");

        return loadInst;
    }

    llvm::Value *Functions::createParamFromParamNode(ASTNode *paramNode)
    {
        OldTypes &types = compiler.getTypes();
        Variables &variables = compiler.getVariables();
        IRSymTable &symTable = compiler.getSymTable();
        CryoContext &context = compiler.getContext();
        std::string namespaceName = compiler.getContext().currentNamespace;
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Parameter");

        DevDebugger::logNode(paramNode);

        std::string paramName = paramNode->data.param->name;
        std::cout << "Parameter Name: " << paramName << std::endl;

        DataType *paramType = paramNode->data.param->type;
        llvm::Type *paramLLVMType = types.getType(paramType, 0);
        std::cout << "Parameter Type: " << std::endl;
        DevDebugger::logLLVMType(paramLLVMType);

        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Alloca Instruction");

        // Get the insert block
        llvm::BasicBlock *insertBlock = context.builder.GetInsertBlock();
        if (!insertBlock)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Insert block not found");
            CONDITION_FAILED;
        }

        llvm::AllocaInst *alloca = context.builder.CreateAlloca(paramLLVMType, nullptr, paramName);
        alloca->setAlignment(llvm::Align(8));

        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Store Instruction");

        llvm::StoreInst *storeInst = context.builder.CreateStore(alloca, alloca);
        storeInst->setAlignment(llvm::Align(8));

        std::string _paramName = paramName;
        compiler.getContext().namedValues[paramName] = alloca;

        symTable.addParamAsVariable(namespaceName, _paramName, alloca, paramLLVMType, storeInst);
        DataType *paramDataType = paramNode->data.param->type;

        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Parameter Created");

        return alloca;
    }

    llvm::Value *Functions::anyTypeParam(std::string functionName, llvm::Value *argValue)
    {
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
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Verifying Callee Arguments");

        std::vector<llvm::Value *> verifiedArgs;
        llvm::FunctionType *calleeFT = callee->getFunctionType();
        int argCount = calleeFT->getNumParams();
        if (argCount != argValues.size())
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Argument count mismatch");
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
                DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Parameter not found");
                CONDITION_FAILED;
            }

            llvm::Value *argValue = argValues[i];
            llvm::Type *expectedType = expectedTypes[i];

            if (argValue->getType()->getTypeID() != expectedType->getTypeID())
            {
                DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Argument type mismatch");

                // Cast the argument to the expected type
                // llvm::Value *castValue = createArgCast(argValue, expectedType);
                verifiedArgs.push_back(argValue);
                continue;
            }
            else
            {
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Argument type matches expected type");
                verifiedArgs.push_back(argValue);
            }
        }

        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Callee Arguments Verified");

        return verifiedArgs;
    }

    llvm::Value *Functions::createArgCast(llvm::Value *argValue, llvm::Type *expectedType)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Argument Cast");

        llvm::Value *castValue = nullptr;
        llvm::Type *sourceType = argValue->getType();

        if (sourceType == expectedType)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Functions", "Argument type matches expected type");
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
            DevDebugger::logMessage("WARNING", __LINE__, "Functions", "Using bitcast as fallback");
            castValue = argValue;
        }
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Argument Cast Created");
        return castValue;
    }

    llvm::Value *Functions::createArgumentVar(ASTNode *node)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Argument Variable");

        llvm::Value *argVar = compiler.getVariables().createLocalVariable(node);
        if (!argVar)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Argument variable not created");
            CONDITION_FAILED;
        }

        return argVar;
    }

    llvm::Function *Functions::findClassMethod(std::string className, std::string methodName)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Finding Class Method");
        std::cout << "Class Name: " << className << std::endl;
        std::cout << "Method Name: " << methodName << std::endl;

        // All methods have the syntax of `class.{className}.{methodName}`
        std::string classMethodName = "class." + className + "." + methodName;
        llvm::Function *method = compiler.getContext().module->getFunction(classMethodName);
        if (!method)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Method not found");
            CONDITION_FAILED;
        }

        return method;
    }

} // namespace Cryo
