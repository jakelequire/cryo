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

    // This is the cryo main function generation, it will be the entry point for the program.
    // As of now, it is the same code as the `createFunctionDeclaration` function.
    // I have separated it to have specific control over the main function for the future.
    void Functions::createCryoMainFunction(ASTNode *node)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Creating Cryo Main Function");

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

        // Get the return type
        DataType *returnType = functionNode->type->container->type.functionType->returnType;
        llvm::Type *returnLLVMType = types.getReturnType(returnType);
        std::string returnTypeName = DTM->debug->dataTypeToString(returnType);
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Return Type: " + returnTypeName);

        // Get the function arguments
        int argCount = functionNode->paramCount;
        DevDebugger::logMessage("INFO", __LINE__, "Functions", "Argument Count: " + std::to_string(argCount));

        std::vector<llvm::Type *> argTypes;
        for (int i = 0; i < argCount; ++i)
        {
            CryoParameterNode *argNode = functionNode->params[i]->data.param;
            assert(argNode != nullptr);

            DataType *_argType = argNode->type;
            if (_argType->container->primitive == PRIM_STRING)
            {
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Converting string to LLVM type");
                // int _len = types.getLiteralValLength(argNode->initializer);
                // Need to find a solution to get the length of the string, setting the type as a pointer might be a temp thing.
                llvm::Type *argType = types.getType(_argType, 32);
                argTypes.push_back(argType->getPointerTo());
                continue;
            }

            if (_argType->container->primitive == PRIM_INT)
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

        // Create the function
        llvm::Function *function = llvm::Function::Create(
            functionType,
            llvm::Function::ExternalLinkage,
            llvm::Twine(functionName),
            *cryoContext.module);

        // Add the function to the symbol table
        cryoContext.namedValues[functionName] = function;

        cryoContext.setCurrentFunction(function);

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
                llvm::Type *returnLLVMType = types.getType(returnType, 0);
                std::string returnTypeName = DTM->debug->dataTypeToString(returnType);
                DevDebugger::logMessage("INFO", __LINE__, "Functions", "Return Type: " + returnTypeName);

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
                        DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Unknown return type");
                        CONDITION_FAILED;
                    }
                    }
                    break;
                }
                case PRIM_STRING:
                {
                    DevDebugger::logMessage("INFO", __LINE__, "Functions", "Returning string");
                    int _len = types.getLiteralValLength(statement);
                    llvm::Type *returnType = types.getType(DTM->primitives->createString(), 0);
                    llvm::Value *returnValue = generator.getInitilizerValue(statement);
                    compiler.getContext().builder.CreateRet(returnValue);

                    break;
                }
                default:
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Functions", "Unknown return type");
                    std::cout << "Received: " << DTM->debug->dataTypeToString(returnType) << std::endl;
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
} // namespace Cryo
