#include "cpp/codegen.h"

namespace Cryo
{

    void Variables::handleConstVariable(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Variables", "Handling Const Variable");

        CryoVariableNode *varDecl = node->data.varDecl;
        assert(varDecl != nullptr);

        processConstVariable(varDecl);

        return;
    }

    // TODO: Implement
    void Variables::handleRefVariable(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Variables", "Handling Ref Variable");

        DEBUG_BREAKPOINT;
    }

    void Variables::handleMutableVariable(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Variables", "Handling Mutable Variable");

        createMutableVariable(node);
    }

    // -----------------------------------------------------------------------------------------------

    void Variables::handleVariableReassignment(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Types &types = compiler.getTypes();
        debugger.logMessage("INFO", __LINE__, "Variables", "Handling Variable Reassignment");
        std::string currentModuleName = compiler.getContext().currentNamespace;

        // Find the variable in the symbol table
        std::string existingVarName = std::string(node->data.varReassignment->existingVarName);
        std::cout << "Variable Name: " << existingVarName << std::endl;
        ASTNode *varNode = compiler.getSymTable().getASTNode(currentModuleName, NODE_VAR_DECLARATION, existingVarName);
        if (!varNode)
        {
            debugger.logMessage("ERROR", __LINE__, "Variables", "Variable not found");
            exit(1);
        }

        // Get the new value
        ASTNode *newValue = node->data.varReassignment->newVarNode;
        llvm::Value *newVal = compiler.getGenerator().getInitilizerValue(newValue);
        if (!newVal)
        {
            debugger.logMessage("ERROR", __LINE__, "Variables", "New value not found");
            exit(1);
        }

        llvm::Value *varValue = compiler.getContext().namedValues[existingVarName];
        if (!varValue)
        {
            debugger.logMessage("ERROR", __LINE__, "Variables", "Variable value not found");
            exit(1);
        }
        debugger.logMessage("INFO", __LINE__, "Variables", "Variable Value Found");
        llvm::Instruction *inst = compiler.getContext().builder.CreateStore(newVal, varValue);
        // Set the insert point
        debugger.logMessage("INFO", __LINE__, "Variables", "Variable Reassignment Handled");
        llvm::GlobalVariable *key = inst->getModule()->getNamedGlobal(existingVarName);
        if (key)
        {
            debugger.logMessage("INFO", __LINE__, "Variables", "Variable Reassignment Handled");
            key->setInitializer(llvm::dyn_cast<llvm::Constant>(newVal));
        }
        else
        {
            DEBUG_BREAKPOINT;
        }

        debugger.logMessage("INFO", __LINE__, "Variables", "Variable Reassignment Handled");
        return;
    }

    // -----------------------------------------------------------------------------------------------

    llvm::Value *Variables::createLocalVariable(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Types &types = compiler.getTypes();
        debugger.logMessage("INFO", __LINE__, "Variables", "Creating Local Variable");

        std::string _varName = std::string(node->data.varDecl->name);

        CryoVariableNode *varDecl = node->data.varDecl;
        assert(varDecl != nullptr);

        CryoDataType varType = varDecl->type;
        ASTNode *initializer = varDecl->initializer;
        assert(initializer != nullptr);
        CryoNodeType initializerNodeType = initializer->metaData->type;
        std::string varName = std::string(varDecl->name);
        llvm::Value *llvmValue = nullptr;
        llvm::Type *llvmType = nullptr;

        // Check if it already exists
        std::cout << "Variable Name: " << varName << std::endl;
        llvmValue = compiler.getContext().namedValues[varName];
        if (llvmValue)
        {
            debugger.logMessage("WARN", __LINE__, "Variables", "Variable already exists");
            return llvmValue;
        }

        if (initializerNodeType == NODE_FUNCTION_CALL)
        {
            debugger.logMessage("INFO", __LINE__, "Variables", "Variable is a function call");
            llvmValue = createVarWithFuncCallInitilizer(node);
            compiler.getContext().namedValues[varName] = llvmValue;
            return llvmValue;
        }

        if (initializerNodeType == NODE_BINARY_EXPR)
        {
            debugger.logMessage("INFO", __LINE__, "Variables", "Variable is a binary expression");
            // We will have to walk though the binary expression if it has multiple expressions within it
            std::cout << "Name being passed (very start): " << varName << std::endl;
            llvmValue = compiler.getBinaryExpressions().handleComplexBinOp(initializer);
            compiler.getContext().namedValues[varName] = llvmValue;
            // set the name of the `llvmValue` to the variable name
            llvmValue->setName(varName);

            return llvmValue;
        }

        debugger.logMessage("INFO", __LINE__, "Variables", "Type: " + std::string(CryoDataTypeToString(varType)));

        if (varType == DATA_TYPE_STRING)
        {
            debugger.logMessage("INFO", __LINE__, "Variables", "Creating String Variable");
            int _len = compiler.getTypes().getLiteralValLength(varDecl->initializer);
            llvmType = compiler.getTypes().getType(varType, _len + 3);
            debugger.logMessage("INFO", __LINE__, "Variables", "Type: " + std::string(CryoDataTypeToString(varType)));

            // Create a alloc/store instruction
            llvmValue = compiler.getContext().builder.CreateAlloca(llvmType, nullptr, varName);
            compiler.getContext().namedValues[varName] = llvmValue;
            // Get the value of the string
            llvm::Value *varValue = compiler.getGenerator().getInitilizerValue(varDecl->initializer);
            if (!varValue)
            {
                debugger.logMessage("ERROR", __LINE__, "Variables", "Variable value not found");
                exit(1);
            }
            debugger.logMessage("INFO", __LINE__, "Variables", "Variable Value Found");
            llvm::Instruction *inst = compiler.getContext().builder.CreateStore(varValue, llvmValue);

            return inst;
        }
        if (varType == DATA_TYPE_INT)
        {
            llvmType = compiler.getTypes().getType(varType, 0);
            llvm::Type *ptrType = llvmType;
            llvm::Value *varValue = compiler.getContext().builder.CreateAlloca(ptrType, nullptr, varName);
            llvmValue = varValue;
            // Store the value
            int intval = varDecl->initializer->data.literal->value.intValue;
            llvm::Value *val = llvm::ConstantInt::get(llvmType, intval);
            compiler.getContext().builder.CreateStore(val, llvmValue);
            compiler.getContext().namedValues[varName] = llvmValue;
            debugger.logMessage("INFO", __LINE__, "Variables", "Type: " + std::string(CryoDataTypeToString(varType)));

            return varValue;
        }
        else
        {
            debugger.logMessage("INFO", __LINE__, "Variables", "Local Variable has unknown type");
            DEBUG_BREAKPOINT;
            llvmType = compiler.getTypes().getType(varType, 0);
            debugger.logMessage("INFO", __LINE__, "Variables", "Type: " + std::string(CryoDataTypeToString(varType)));
        }

        debugger.logMessage("INFO", __LINE__, "Variables", "Type: " + std::string(CryoDataTypeToString(varType)));

        // llvmValue = compiler.getContext().builder.CreateAlloca(llvmType, nullptr, varName);
        compiler.getContext().namedValues[varName] = llvmValue;

        debugger.logMessage("INFO", __LINE__, "Variables", "Variable Created");
        debugger.logMessage("INFO", __LINE__, "Variables", "Variable Set");

        return llvmValue;
    }

    // -----------------------------------------------------------------------------------------------

    void Variables::processConstVariable(CryoVariableNode *varNode)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        CryoContext &cryoContext = compiler.getContext();
        Arrays &arrays = compiler.getArrays();

        Types &types = compiler.getTypes();
        debugger.logMessage("INFO", __LINE__, "Variables", "Processing Const Variable");

        char *varName = varNode->name;
        std::cout << "Variable Name: " << varName << std::endl;
        CryoDataType type = varNode->type;
        ASTNode *initializer = varNode->initializer;

        llvm::Type *llvmType = nullptr;
        llvm::Value *llvmValue = nullptr;
        llvm::Constant *llvmConstant = nullptr;
        llvm::ArrayType *llvmArrayType = nullptr;

        bool isGlobal = varNode->isGlobal;

        if (!isGlobal)
        {
            // Get the current block
            llvm::BasicBlock *currentBlock = cryoContext.builder.GetInsertBlock();
            if (!currentBlock)
            {
                debugger.logMessage("ERROR", __LINE__, "Variables", "Current block not found");
                exit(1);
            }
            debugger.logMessage("INFO", __LINE__, "Variables", "Current block found");
            // Set the insert point
            cryoContext.builder.SetInsertPoint(currentBlock);
        }
        else
        {
            debugger.logMessage("INFO", __LINE__, "Variables", "Global Variable");
        }

        if (initializer)
        {
            if (initializer->metaData->type == NODE_INDEX_EXPR)
            {
                debugger.logMessage("INFO", __LINE__, "Variables", "Processing Array Index");
                char *arrayName = initializer->data.indexExpr->name;
                std::cout << "Array Name: " << arrayName << std::endl;
                arrays.handleIndexExpression(initializer, varName);
                debugger.logMessage("INFO", __LINE__, "Variables", "Array Index Processed");
                return;
            }
            // Check if the initializer is an array
            if (initializer->metaData->type == NODE_ARRAY_LITERAL)
            {
                debugger.logMessage("INFO", __LINE__, "Variables", "Processing Array Literal");

                arrays.handleArrayLiteral(initializer);

                debugger.logMessage("INFO", __LINE__, "Variables", "Array Literal Processed");
            }
            else
            {
                int _len = types.getLiteralValLength(initializer);
                if (type == DATA_TYPE_STRING)
                    _len += 1; // Add one for the null terminator
                debugger.logMessage("INFO", __LINE__, "Variables", "Length: " + std::to_string(_len));
                llvmType = types.getType(type, _len);
                char *typeNode = CryoDataTypeToString(type);
                debugger.logMessage("INFO", __LINE__, "Variables", "Varname: " + std::string(varName));
                debugger.logMessage("INFO", __LINE__, "Variables", "Data Type: " + std::string(typeNode));

                switch (type)
                {
                case DATA_TYPE_INT:
                {
                    debugger.logMessage("INFO", __LINE__, "Variables", "Creating Int Constant");
                    llvmConstant = llvm::ConstantInt::get(llvmType, initializer->data.literal->value.intValue);
                    break;
                }
                case DATA_TYPE_FLOAT:
                {
                    debugger.logMessage("INFO", __LINE__, "Variables", "Creating Float Constant");
                    llvmConstant = llvm::ConstantFP::get(llvmType, initializer->data.literal->value.floatValue);
                    break;
                }
                case DATA_TYPE_BOOLEAN:
                {
                    debugger.logMessage("INFO", __LINE__, "Variables", "Creating Boolean Constant");
                    llvmConstant = llvm::ConstantInt::get(llvmType, initializer->data.literal->value.booleanValue);
                    break;
                }
                case DATA_TYPE_STRING:
                {
                    debugger.logMessage("INFO", __LINE__, "Variables", "Creating String Constant");
                    llvmConstant = llvm::ConstantDataArray::getString(cryoContext.context, initializer->data.literal->value.stringValue);
                    break;
                }
                default:
                    debugger.logMessage("ERROR", __LINE__, "Variables", "Unknown type");
                    exit(1);
                    break;
                }

                debugger.logMessage("INFO", __LINE__, "Variables", "Constant Created");
                llvmValue = llvm::dyn_cast<llvm::Value>(llvmConstant);

                if (!llvmValue)
                {
                    debugger.logMessage("ERROR", __LINE__, "Variables", "Failed to cast constant to value");
                    exit(1);
                }

                llvm::GlobalVariable *var = new llvm::GlobalVariable(
                    *cryoContext.module,
                    llvmType,
                    false,
                    llvm::GlobalValue::ExternalLinkage,
                    llvmConstant,
                    llvm::Twine(varName));
                compiler.getContext().namedValues[varName] = var;
                debugger.logMessage("INFO", __LINE__, "Variables", "Variable Created");
            }
        }
        debugger.logMessage("INFO", __LINE__, "Variables", "Variable Created");

        return;
    }

    // -----------------------------------------------------------------------------------------------

    void Variables::createMutableVariable(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        CryoContext &cryoContext = compiler.getContext();
        Arrays &arrays = compiler.getArrays();

        Types &types = compiler.getTypes();
        debugger.logMessage("INFO", __LINE__, "Variables", "Processing Const Variable");
        CryoVariableNode *varNode = node->data.varDecl;
        char *varName = varNode->name;
        std::cout << "Variable Name: " << varName << std::endl;
        CryoDataType type = varNode->type;
        ASTNode *initializer = varNode->initializer;

        llvm::Type *llvmType = nullptr;
        llvm::Value *llvmValue = nullptr;
        llvm::Constant *llvmConstant = nullptr;
        llvm::ArrayType *llvmArrayType = nullptr;

        if (initializer)
        {
            if (initializer->metaData->type == NODE_INDEX_EXPR)
            {
                debugger.logMessage("INFO", __LINE__, "Variables", "Processing Array Index");
                char *arrayName = initializer->data.indexExpr->name;
                std::cout << "Array Name: " << arrayName << std::endl;
                arrays.handleIndexExpression(initializer, varName);
                debugger.logMessage("INFO", __LINE__, "Variables", "Array Index Processed");
                return;
            }
            // Check if the initializer is an array
            if (initializer->metaData->type == NODE_ARRAY_LITERAL)
            {
                debugger.logMessage("INFO", __LINE__, "Variables", "Processing Array Literal");

                arrays.handleArrayLiteral(initializer);

                debugger.logMessage("INFO", __LINE__, "Variables", "Array Literal Processed");
            }
            else
            {
                int _len = types.getLiteralValLength(initializer);
                if (type == DATA_TYPE_STRING)
                    _len += 1; // Add one for the null terminator
                debugger.logMessage("INFO", __LINE__, "Variables", "Length: " + std::to_string(_len));
                llvmType = types.getType(type, _len);
                char *typeNode = CryoDataTypeToString(type);
                debugger.logMessage("INFO", __LINE__, "Variables", "Type: " + std::string(typeNode));

                switch (type)
                {
                case DATA_TYPE_INT:
                {
                    debugger.logMessage("INFO", __LINE__, "Variables", "Creating Int Constant");
                    llvmConstant = llvm::ConstantInt::get(llvmType, initializer->data.literal->value.intValue);
                    break;
                }
                case DATA_TYPE_FLOAT:
                {
                    debugger.logMessage("INFO", __LINE__, "Variables", "Creating Float Constant");
                    llvmConstant = llvm::ConstantFP::get(llvmType, initializer->data.literal->value.floatValue);
                    break;
                }
                case DATA_TYPE_BOOLEAN:
                {
                    debugger.logMessage("INFO", __LINE__, "Variables", "Creating Boolean Constant");
                    llvmConstant = llvm::ConstantInt::get(llvmType, initializer->data.literal->value.booleanValue);
                    break;
                }
                case DATA_TYPE_STRING:
                {
                    debugger.logMessage("INFO", __LINE__, "Variables", "Creating String Constant");
                    llvmConstant = llvm::ConstantDataArray::getString(cryoContext.context, initializer->data.literal->value.stringValue);
                    break;
                }
                default:
                    debugger.logMessage("ERROR", __LINE__, "Variables", "Unknown type");
                    exit(1);
                    break;
                }

                debugger.logMessage("INFO", __LINE__, "Variables", "Constant Created");
                llvmValue = llvm::dyn_cast<llvm::Value>(llvmConstant);

                if (!llvmValue)
                {
                    debugger.logMessage("ERROR", __LINE__, "Variables", "Failed to cast constant to value");
                    exit(1);
                }

                llvm::GlobalVariable *var = new llvm::GlobalVariable(
                    *cryoContext.module,
                    llvmType,
                    false,
                    llvm::GlobalValue::WeakAnyLinkage,
                    llvmConstant,
                    llvm::Twine(varName));

                compiler.getContext().namedValues[varName] = var;
            }
        }
        debugger.logMessage("INFO", __LINE__, "Variables", "Variable Created");

        return;
    }

    // -----------------------------------------------------------------------------------------------

    llvm::Value *Variables::getVariable(std::string name)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Variables", "Getting Variable");
        std::cout << "Name being passed: " << name << std::endl;

        llvm::Value *llvmValue = nullptr;

        llvmValue = compiler.getContext().module->getNamedGlobal(name);

        if (!llvmValue)
        {
            debugger.logMessage("ERROR", __LINE__, "Variables", "Variable not found");
            llvmValue = nullptr;
        }

        std::cout << "Name being passed: " << name << std::endl;
        llvmValue = getLocalScopedVariable(name);
        if (!llvmValue)
        {
            debugger.logMessage("ERROR", __LINE__, "Variables", "Variable not found");
            llvmValue = nullptr;
        }

        if (llvmValue != nullptr)
        {
            debugger.logMessage("INFO", __LINE__, "Variables", "Variable Found");
        }

        return llvmValue;
    }

    // -----------------------------------------------------------------------------------------------

    llvm::Value *Variables::getLocalScopedVariable(std::string name)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Variables", "Getting Local Scoped Variable");

        llvm::Value *llvmValue = nullptr;
        std::cout << "Name being passed (start): " << name << std::endl;

        llvmValue = compiler.getContext().namedValues[name];

        if (!llvmValue)
        {
            debugger.logMessage("ERROR", __LINE__, "Variables", "Variable not found");
            llvmValue = nullptr;
        }

        if (llvmValue != nullptr)
        {
            debugger.logMessage("INFO", __LINE__, "Variables", "Variable Found");
            // Print the variable
            llvm::Instruction *instruction = llvm::dyn_cast<llvm::Instruction>(llvmValue);
            if (instruction)
            {
                std::string instName = instruction->getName().str();
                std::cout << "Inst Name: " << instName << std::endl;

                llvm::Value *instValue = instruction->getOperand(0);
                std::string instValueName = instValue->getName().str();
                std::cout << "Inst Value Name: " << instValueName << std::endl;
            }
        }

        return llvmValue;
    }

    // -----------------------------------------------------------------------------------------------

    llvm::Value *Variables::createVarWithFuncCallInitilizer(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Variables", "Creating Variable with Function Call Initializer");

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
            debugger.logMessage("ERROR", __LINE__, "Variables", "Initializer is not a function call");
            DEBUG_BREAKPOINT;
        }

        std::string functionName = std::string(initializer->data.functionCall->name);
        std::cout << "Function Name: " << functionName << std::endl;

        // Get the arguments
        std::vector<llvm::Value *> argValues;
        for (int i = 0; i < initializer->data.functionCall->argCount; ++i)
        {
            ASTNode *argNode = initializer->data.functionCall->args[i];
            CryoNodeType argNodeType = argNode->metaData->type;
            llvm::Value *argValue = compiler.getGenerator().getInitilizerValue(argNode);
            if (!argValue)
            {
                debugger.logMessage("ERROR", __LINE__, "Variables", "Argument value not found");
                exit(1);
            }
            if (argNodeType == NODE_VAR_DECLARATION)
            {
                std::cout << "Argument Name: " << argNode->data.varDecl->name << std::endl;
                std::cout << "Argument Type: " << CryoDataTypeToString(argNode->data.varDecl->type) << std::endl;
                std::cout << "Argument Node Type: " << CryoNodeTypeToString(argNode->metaData->type) << std::endl;
                ASTNode *varNode = compiler.getSymTable().getASTNode(moduleName, NODE_VAR_DECLARATION, argNode->data.varDecl->name);
                if (!varNode)
                {
                    debugger.logMessage("ERROR", __LINE__, "Variables", "Variable not found");
                    exit(1);
                }
                CryoNodeType varNodeType = varNode->metaData->type;
                std::cout << "Variable Node Type: " << CryoNodeTypeToString(varNodeType) << std::endl;
                if (varNodeType == NODE_LITERAL_EXPR)
                {
                    CryoDataType varDataType = varNode->data.literal->dataType;
                    switch (varDataType)
                    {
                    case DATA_TYPE_INT:
                    {
                        int val = varNode->data.literal->value.intValue;
                        argValue = compiler.getTypes().getLiteralIntValue(val);
                        break;
                    }
                    default:
                    {
                        debugger.logMessage("ERROR", __LINE__, "Variables", "Unknown data type");
                        exit(1);
                    }
                    }
                }
            }

            debugger.logMessage("INFO", __LINE__, "Variables", "Argument Value Found");
            argValues.push_back(argValue);
        }
        debugger.logMessage("INFO", __LINE__, "Variables", "Argument list parsed.");

        // Get the function
        llvm::Function *function = compiler.getContext().module->getFunction(functionName);
        if (!function)
        {
            debugger.logMessage("ERROR", __LINE__, "Variables", "Function not found");
            exit(1);
        }
        debugger.logMessage("INFO", __LINE__, "Variables", "Function Found");

        // Create the function call
        llvm::Value *functionCall = compiler.getContext().builder.CreateCall(function, argValues);
        if (!functionCall)
        {
            debugger.logMessage("ERROR", __LINE__, "Variables", "Function call not created");
            exit(1);
        }

        // Store the call into the variable
        llvm::Instruction *inst = compiler.getContext().builder.CreateStore(functionCall, varValue);

        debugger.logMessage("INFO", __LINE__, "Variables", "Function Call Created");

        return functionCall;
    }

} // namespace Cryo
