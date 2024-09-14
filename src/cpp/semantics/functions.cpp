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
            exit(1);
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
            CryoVariableNode *argNode = functionNode->params[i]->data.varDecl;
            assert(argNode != nullptr);
            CryoDataType _argType = argNode->type;

            if (_argType == DATA_TYPE_STRING)
            {
                debugger.logMessage("INFO", __LINE__, "Functions", "Converting string to LLVM type");
                int _len = types.getLiteralValLength(argNode->initializer);
                llvm::Type *argType = types.getType(_argType, _len);
                argTypes.push_back(argType);
                continue;
            }

            llvm::Type *argType = compiler.getTypes().getType(argNode->type, 0);
            argTypes.push_back(argType);
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

        // Set the function arguments
        int i = 0;
        for (auto &arg : function->args())
        {
            arg.setName(functionNode->params[i]->data.varDecl->name);
            ++i;
        }

        // Create the entry block
        llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(compiler.getContext().context, "entry", function);
        compiler.getContext().builder.SetInsertPoint(entryBlock);

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
                    llvm::Value *returnValue = generator.getInitilizerValue(statement);
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
                    exit(1);
                }
                }

                // move past the return statement
                continue;
            }

            compiler.getGenerator().parseTree(statement);
        }

        // Check if the current block is already terminated
        llvm::BasicBlock *currentBlock = compiler.getContext().builder.GetInsertBlock();
        if (!currentBlock->getTerminator())
        {
            debugger.logMessage("ERROR", __LINE__, "Functions", "Adding terminator to function");
            exit(1);
        }
        else
        {
            debugger.logMessage("INFO", __LINE__, "Functions", "Function already has a terminator");
        }

        debugger.logMessage("INFO", __LINE__, "Functions", "Function Declaration Created");

        // compiler.getContext().currentFunction = function;

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

                return;
            }
            compiler.getGenerator().parseTree(statement);
        }

        debugger.logMessage("INFO", __LINE__, "Functions", "Function Block Created");

        return;
    }

    // -----------------------------------------------------------------------------------------------

    // TODO: Remove
    void Functions::createReturnStatement(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Generator &generator = compiler.getGenerator();
        CryoContext &cryoContext = compiler.getContext();
        if (node->metaData->type != NODE_RETURN_STATEMENT)
        {
            debugger.logMessage("ERROR", __LINE__, "Functions", "Node is not a return statement");
            return;
        }
        Types &types = compiler.getTypes();
        debugger.logMessage("INFO", __LINE__, "Functions", "Creating Return Statement");

        CryoReturnNode *returnNode = node->data.returnStatement;
        assert(returnNode != nullptr);

        CryoDataType returnType = returnNode->returnType;
        debugger.logMessage("INFO", __LINE__, "Functions", "Return Type: " + std::string(CryoDataTypeToString(returnType)));

        // This function should not trigger, exit if it does
        exit(1);

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
                    exit(1);
                    break;
                }
                }
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
            CryoVariableNode *argNode = functionNode->params[i]->data.varDecl;
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
            arg.setName(functionNode->params[i]->data.varDecl->name);
            ++i;
        }

        debugger.logMessage("INFO", __LINE__, "Functions", "Extern Function Created");

        return;
    }

    // -----------------------------------------------------------------------------------------------

    void Functions::createFunctionCall(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Generator &generator = compiler.getGenerator();
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
        debugger.logMessage("INFO", __LINE__, "Functions", "Argument Count: " + std::to_string(argCount));

        // Get the argument values
        std::vector<llvm::Value *> argValues;
        for (int i = 0; i < argCount; ++i)
        {
            ASTNode *argNode = functionCallNode->args[i];
            CryoNodeType argType = argNode->metaData->type;
            CryoDataType argTypeData = argNode->data.varDecl->type;

            std::cout << "===----------------------===" << std::endl;
            std::cout << "Argument Type: " << CryoNodeTypeToString(argType) << std::endl;
            std::cout << "Argument Data Type: " << CryoDataTypeToString(argTypeData) << std::endl;
            std::cout << "===----------------------===" << std::endl;

            // if (argType == NODE_)

            if (argTypeData == DATA_TYPE_STRING)
            {
                // We can use direct values for strings like we do for integers.
                // We can also use the symbol table to get the value of the string.
                // Functions that take in strings as arguments treat them as pointers.

                // Get the string value
                std::string argName = std::string(argNode->data.varDecl->name);
                std::cout << "!Argument Name: " << argName << std::endl;

                // Check if the variable is in the global named values
                llvm::GlobalValue *globalValue = compiler.getContext().module->getNamedValue(argName);
                if (globalValue)
                {
                    std::cout << "Global Value Found" << std::endl;
                    argValues.push_back(globalValue);
                    continue;
                }
                else
                {
                    debugger.logMessage("INFO", __LINE__, "Functions", "Creating String Argument");
                    // Trim the `\22` from the string
                    argName = argName.substr(1, argName.length() - 2);
                    // Reassign the name to the variable
                    argNode->data.varDecl->name = (char *)argName.c_str();
                    // Since the variable is not in the global named values, we can assume that it is a local variable.
                    int _len = compiler.getTypes().getLiteralValLength(argNode);
                    // Note: unsure why I have to subtract 1 but it works for now
                    llvm::Type *argType = compiler.getTypes().getType(DATA_TYPE_STRING, _len - 1);
                    llvm::Value *argValue = compiler.getGenerator().getInitilizerValue(argNode);
                    if (argValue == nullptr)
                    {
                        debugger.logMessage("INFO", __LINE__, "Functions", "Argument Value Not Found, creating one...");
                        llvm::Constant *initializer = llvm::ConstantDataArray::getString(compiler.getContext().context, argName, true);
                        llvm::GlobalVariable *globalVariable = new llvm::GlobalVariable(
                            *compiler.getContext().module,
                            argType,
                            false,
                            llvm::GlobalValue::ExternalLinkage,
                            initializer,
                            llvm::Twine(argName));

                        // Get the value of the string
                        llvm::Value *_argValue = compiler.getGenerator().getInitilizerValue(argNode);

                        argValues.push_back(_argValue);
                    }
                    else
                    {
                        debugger.logMessage("INFO", __LINE__, "Functions", "Argument Value Found");
                        argValues.push_back(argValue);
                    }
                    std::cout << "Arg Value: " << argValue << std::endl;
                    continue;
                }

                // Create a load instruction to get the value of the string
                llvm::Value *argValue = generator.getInitilizerValue(argNode);
                assert(argValue != nullptr);

                argValues.push_back(argValue);
                continue;
            }

            if (argTypeData == DATA_TYPE_INT)
            {
                // Find the variable in the symbol table
                std::string argName = std::string(argNode->data.varDecl->name);
                std::cout << "Argument Name: " << argName << std::endl;

                exit(0);
                continue;
            }

            std::string argName = std::string(argNode->data.varDecl->name);
            std::cout << "Argument Type: " << CryoNodeTypeToString(argType) << std::endl;

            ASTNode *retreivedNode = compiler.getSymTable().getASTNode(moduleName, argType, argName);
            if (!retreivedNode)
            {
                debugger.logMessage("ERROR", __LINE__, "Functions", "Argument not found");
                exit(1);
            }

            llvm::Value *argValue = compiler.getGenerator().getInitilizerValue(retreivedNode);
            argValues.push_back(argValue);
        }

        // Get the function
        llvm::Function *function = compiler.getContext().module->getFunction(functionName);
        if (!function)
        {
            debugger.logMessage("ERROR", __LINE__, "Functions", "Function not found");
            exit(1);
        }

        // Create the function call
        llvm::Value *functionCall = compiler.getContext().builder.CreateCall(function, argValues);

        debugger.logMessage("INFO", __LINE__, "Functions", "Function Call Created");

        return;
    }

}
