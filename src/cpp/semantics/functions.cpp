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
            break;
        }
        case NODE_RETURN_STATEMENT:
        {
            debugger.logMessage("INFO", __LINE__, "Functions", "Handling Return Statement");
            createReturnStatement(node);
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

        // Create the function type
        llvm::FunctionType *functionType = llvm::FunctionType::get(types.getType(returnType, 0), argTypes, false);

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

            // if(nodeType == NODE_VAR_DECLARATION)
            // {
            //     VariableIR* varIR = variables.createNewLocalVariable(statement);
            //     llvm::Value* varValue = varIR->value;
            //     llvm::Type* varType = varIR->type;
            //     std::string varName = varIR->name;
            // }

            if(nodeType == NODE_RETURN_STATEMENT) 
            {
                llvm::Type *returnLLVMType = types.getReturnType(returnType);
                if(returnType == DATA_TYPE_VOID)
                {
                    compiler.getContext().builder.CreateRetVoid();
                }
                else
                {
                    // For non-void functions, grab the return value from the return statement
                    llvm::Value *returnValue = generator.getInitilizerValue(statement);
                    compiler.getContext().builder.CreateRet(returnValue);
                }
            }

            compiler.getGenerator().parseTree(statement);
        }

        // Check if the current block is already terminated
        llvm::BasicBlock *currentBlock = compiler.getContext().builder.GetInsertBlock();
        if (!currentBlock->getTerminator())
        {
            debugger.logMessage("INFO", __LINE__, "Functions", "Adding terminator to function");
            if (returnType == DATA_TYPE_VOID)
            {
                compiler.getContext().builder.CreateRetVoid();
            }
            else
            {
                // For non-void functions, create a default return value
                llvm::Type *returnLLVMType = types.getReturnType(returnType);
                llvm::Value *defaultReturnValue = llvm::UndefValue::get(returnLLVMType);
                compiler.getContext().builder.CreateRet(defaultReturnValue);
            }
        } else 
        {
            debugger.logMessage("INFO", __LINE__, "Functions", "Function already has a terminator");
        }

        debugger.logMessage("INFO", __LINE__, "Functions", "Function Declaration Created");

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

    void Functions::createReturnStatement(ASTNode *node)
    {

        CryoDebugger &debugger = compiler.getDebugger();
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
        if (returnType == DATA_TYPE_VOID)
        {
            debugger.logMessage("INFO", __LINE__, "Functions", "Returning void");
            llvm::ReturnInst *ret = llvm::ReturnInst::Create(compiler.getContext().context, nullptr, compiler.getContext().builder.GetInsertBlock());

            // add the return instruction to the block
            compiler.getContext().builder.Insert(ret, "main");

            return;
        }
    }

}