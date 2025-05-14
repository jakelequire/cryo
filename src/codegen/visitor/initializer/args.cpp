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
#include "codegen/codegen.hpp"

namespace Cryo
{
    /**
     * @brief Process function arguments and prepare them for a function call
     *
     * This method handles the common tasks when processing function arguments:
     * - Extracts argument values from AST nodes
     * - Handles type checking against expected parameter types
     * - Manages special types like strings and structs
     * - Performs necessary loads/stores for pointer types
     *
     * @param args Array of AST nodes representing the arguments
     * @param argCount Number of arguments
     * @param expectedParamTypes Optional array of expected parameter types (from function signature)
     * @param functionName Name of the function being called (for logging)
     * @return Vector of LLVM Values ready to be passed to CreateCall
     */
    std::vector<llvm::Value *> Initializer::processArguments(
        ASTNode **args,
        int argCount,
        DataType **expectedParamTypes,
        const std::string &functionName)
    {
        std::vector<llvm::Value *> processedArgs;

        logMessage(LMI, "INFO", "Initializer", "Processing %d arguments for %s",
                   argCount, functionName.c_str());

        for (int i = 0; i < argCount; i++)
        {
            // Get argument AST node and its type
            ASTNode *argNode = args[i];
            DataType *argType = DTM->astInterface->getTypeofASTNode(argNode);

            if (!argType)
            {
                logMessage(LMI, "ERROR", "Initializer", "Argument %d has no data type", i);
                continue;
            }

            // Get expected parameter type if available
            DataType *paramType = nullptr;
            if (expectedParamTypes && i < argCount)
            {
                paramType = expectedParamTypes[i];
            }

            // Check if the argument type is a struct or object type
            bool isStructOrObjectType = isStructOrObjectDataType(argType);

            // Check if the parameter type expects a struct or object pointer
            bool paramExpectsStructPtr = paramType && isStructOrObjectDataType(paramType);

            // Get the LLVM value for the argument
            llvm::Value *argValue = getInitializerValue(argNode);
            if (!argValue)
            {
                logMessage(LMI, "ERROR", "Initializer", "Argument %d value is null", i);
                continue;
            }

            // Special handling for string literals
            std::string argValName = argValue->getName().str();
            if (argValName.find("g_str") != std::string::npos)
            {
                // Allocate the string
                llvm::AllocaInst *strAlloc = context.getInstance().builder.CreateAlloca(
                    argValue->getType(), nullptr, argValName + ".alloc");
                context.getInstance().builder.CreateStore(argValue, strAlloc);
                argValue = context.getInstance().builder.CreateLoad(argValue->getType(), strAlloc, argValName + ".load");
                logMessage(LMI, "INFO", "Initializer", "Loaded string argument %d: %s", i, argValue->getName().str().c_str());
            }
            // Regular string handling
            else if (argType->container->primitive == PRIM_STR)
            {
                // Load the string
                argValue = context.getInstance().builder.CreateLoad(argValue->getType(), argValue, argValName + ".load");
                logMessage(LMI, "INFO", "Initializer", "Loaded string argument %d: %s", i, argValue->getName().str().c_str());
            }

            // Handle pointer types that need to be dereferenced
            if (argValue->getType()->isPointerTy() &&
                !(isStructOrObjectType && paramExpectsStructPtr) &&
                argType->container->primitive != PRIM_STR)
            {

                // Load the value if it's a pointer
                if (llvm::AllocaInst *allocaInst = llvm::dyn_cast<llvm::AllocaInst>(argValue))
                {
                    argValue = context.getInstance().builder.CreateLoad(
                        allocaInst->getAllocatedType(),
                        argValue,
                        argValName + ".load");
                    logMessage(LMI, "INFO", "Initializer", "Loaded pointer argument %d: %s",
                               i, argValue->getName().str().c_str());
                }
            }

            processedArgs.push_back(argValue);
        }

        return processedArgs;
    }

    /**
     * @brief Process function declaration parameters and register them in the symbol table
     *
     * This method handles the common tasks when processing function declaration parameters:
     * - Creates symbol table entries for each parameter
     * - Handles special types like strings and structs appropriately
     * - Performs necessary allocation for pointer types
     *
     * @param function The LLVM function being created
     * @param params Array of AST nodes representing the parameters
     * @param paramCount Number of parameters
     * @param functionName Name of the function being declared (for logging)
     */
    void Initializer::processDeclarationParameters(
        llvm::Function *function,
        ASTNode **params,
        int paramCount,
        const std::string &functionName)
    {
        logMessage(LMI, "INFO", "Initializer", "Processing %d parameters for function declaration %s",
                   paramCount, functionName.c_str());
        IRSymbolTable *symbolTable = context.getInstance().symbolTable.get();
        llvm::Function::arg_iterator argIt = function->arg_begin();

        for (int i = 0; i < paramCount; i++)
        {
            ASTNode *param = params[i];
            DataType *paramDataType = param->data.param->type;

            if (!paramDataType)
            {
                logMessage(LMI, "ERROR", "Initializer", "Parameter %d has no data type", i);
                continue;
            }

            llvm::Type *paramType = symbolTable->getLLVMType(paramDataType);
            if (!paramType)
            {
                logMessage(LMI, "ERROR", "Initializer", "Parameter type is null");
                paramType = llvm::Type::getVoidTy(context.getInstance().context);
            }

            // Create a symbol for each parameter for the symbol table
            std::string paramName = param->data.param->name;
            logMessage(LMI, "INFO", "Initializer", "Parameter Name: %s", paramName.c_str());

            // Get the llvm argument
            llvm::Value *arg = &(*argIt++);
            arg->setName(paramName);

            // Check if this is a struct or object type that needs special handling
            bool isStructOrObjectType = isStructOrObjectDataType(paramDataType);

            // For pointer types and struct/object types, we may need special handling
            if (arg->getType()->isPointerTy() || isStructOrObjectType)
            {
                // Store the pointer to the parameter
                llvm::AllocaInst *allocaInst = context.getInstance().builder.CreateAlloca(
                    paramType, nullptr, paramName + ".alloca");
                context.getInstance().builder.CreateStore(arg, allocaInst);
                arg = allocaInst;
                logMessage(LMI, "INFO", "Initializer", "Created alloca for parameter %s", paramName.c_str());
            }

            // Create the variable symbol and add it to the symbol table
            AllocaType allocaType = AllocaTypeInference::inferFromNode(param, false);
            IRVariableSymbol paramSymbol = IRSymbolManager::createVariableSymbol(
                function, nullptr, paramType, paramName, allocaType);
            paramSymbol.dataType = paramDataType;
            paramSymbol.value = arg;

            symbolTable->addVariable(paramSymbol);
            logMessage(LMI, "INFO", "Initializer", "Added parameter %s to symbol table", paramName.c_str());
        }
    }

    /**
     * @brief Check if a DataType represents a struct or object type
     *
     * @param dataType The DataType to check
     * @return true if it's a struct, object, or special string type
     */
    bool Initializer::isStructOrObjectDataType(DataType *dataType)
    {
        if (!dataType || !dataType->container)
        {
            return false;
        }

        // Check if it's an object type (struct, class, etc.)
        if (dataType->container->typeOf == OBJECT_TYPE)
        {
            return true;
        }

        // Check if it's the special String primitive
        if (dataType->container->primitive == PRIM_STRING)
        {
            return true;
        }

        return false;
    }

    /**
     * @brief Process the return value of a function call
     *
     * @param result The raw result from CreateCall
     * @param returnType The expected return type
     * @param functionName Name of the function (for naming)
     * @return The processed return value
     */
    llvm::Value *Initializer::processReturnValue(
        llvm::Value *result,
        DataType *returnType,
        const std::string &functionName)
    {
        if (!result || !returnType)
        {
            return result;
        }

        // Check if we need to do any type casting based on the return type
        if (returnType->container->primitive == PRIM_STR)
        {
            // Convert to string type if needed
            result = context.getInstance().builder.CreateBitCast(
                result,
                context.getInstance().builder.getInt8Ty(),
                functionName + ".str.result");
        }
        else if (returnType->container->primitive == PRIM_STRING)
        {
            // Convert to String type if needed
            result = context.getInstance().builder.CreateBitCast(
                result,
                context.getInstance().builder.getInt8Ty()->getPointerTo(),
                functionName + ".string.result");
        }
        else if (returnType->container->primitive == PRIM_INT)
        {
            // Convert to int type if needed
            if (result->getType() != context.getInstance().builder.getInt32Ty())
            {
                result = context.getInstance().builder.CreateIntCast(
                    result,
                    context.getInstance().builder.getInt32Ty(),
                    true,
                    functionName + ".int.result");
            }
        }

        // For non-void returns, allocate and store the result
        if (!result->getType()->isVoidTy())
        {
            llvm::AllocaInst *returnAlloca = context.getInstance().builder.CreateAlloca(
                result->getType(), nullptr, functionName + ".result");
            context.getInstance().builder.CreateStore(result, returnAlloca);
            return returnAlloca;
        }

        return result;
    }

} // namespace Cryo
