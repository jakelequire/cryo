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

    llvm::Value *Variables::createLocalVariable(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Variables", "Creating Local Variable");

        CryoVariableNode *varDecl = node->data.varDecl;
        assert(varDecl != nullptr);

        CryoDataType varType = varDecl->type;
        std::string varName = std::string(varDecl->name);
        llvm::Value *llvmValue = nullptr;
        llvm::Type *llvmType = nullptr;

        debugger.logMessage("INFO", __LINE__, "Variables", "Type: " + std::string(CryoDataTypeToString(varType)));

        if (varType == DATA_TYPE_STRING)
        {
            int _len = compiler.getTypes().getLiteralValLength(varDecl->initializer);
            llvmType = compiler.getTypes().getType(varType, _len);
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
        }
        if (varType == DATA_TYPE_INT)
        {
            llvmType = compiler.getTypes().getType(varType, 0);
            llvmValue = compiler.getContext().builder.CreateAlloca(llvmType, nullptr, varName);
            // Store the value
            int intval = varDecl->initializer->data.literal->value.intValue;
            llvm::Value *val = llvm::ConstantInt::get(llvmType, intval);
            llvm::Instruction *inst = compiler.getContext().builder.CreateStore(val, llvmValue);
            compiler.getContext().namedValues[varName] = llvmValue;
            debugger.logMessage("INFO", __LINE__, "Variables", "Type: " + std::string(CryoDataTypeToString(varType)));
        }
        else
        {
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

    // unused
    /*
        llvm::Value *value;
        llvm::Type *type;
        std::string name;
        CryoDataType dataType;
    */
    VariableIR *Variables::createNewLocalVariable(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Variables", "Creating New Variable");

        CryoVariableNode *varDecl = node->data.varDecl;
        assert(varDecl != nullptr);

        VariableIR *var = new VariableIR();

        CryoDataType varType = node->data.varDecl->type;
        std::string varName = std::string(varDecl->name);
        llvm::Value *llvmValue = nullptr;
        llvm::Type *llvmType = nullptr;

        debugger.logMessage("INFO", __LINE__, "Variables", "Type: " + std::string(CryoDataTypeToString(varType)));

        if (varType == DATA_TYPE_STRING)
        {
            int _len = compiler.getTypes().getLiteralValLength(varDecl->initializer);
            llvmType = compiler.getTypes().getType(varType, _len);
            var->type = llvmType;
            debugger.logMessage("INFO", __LINE__, "Variables", "Type: " + std::string(CryoDataTypeToString(varType)));
        }
        if (varType == DATA_TYPE_INT)
        {
            llvmType = compiler.getTypes().getType(varType, 0);
            var->type = llvmType;
            int val = varDecl->initializer->data.literal->value.intValue;
            llvmValue = llvm::ConstantInt::get(llvmType, val);
            debugger.logMessage("INFO", __LINE__, "Variables", "Type: " + std::string(CryoDataTypeToString(varType)));
        }
        else
        {
            llvmType = compiler.getTypes().getType(varType, 0);
            var->type = llvmType;
            debugger.logMessage("INFO", __LINE__, "Variables", "Type: " + std::string(CryoDataTypeToString(varType)));
        }

        debugger.logMessage("INFO", __LINE__, "Variables", "Type: " + std::string(CryoDataTypeToString(varType)));

        var->name = varName;
        var->dataType = varType;

        debugger.logMessage("INFO", __LINE__, "Variables", "Variable Created");
        var->value = llvmValue;

        debugger.logMessage("INFO", __LINE__, "Variables", "Variable Set");

        return var;
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

        llvm::Value *llvmValue = nullptr;

        llvmValue = compiler.getContext().module->getNamedGlobal(name);

        if (!llvmValue)
        {
            debugger.logMessage("ERROR", __LINE__, "Variables", "Variable not found");
            llvmValue = nullptr;
        }

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

    llvm::Value *Variables::getLocalScopedVariable(std::string name)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Variables", "Getting Local Scoped Variable");

        llvm::Value *llvmValue = nullptr;

        llvmValue = compiler.getContext().namedValues[name];

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

} // namespace Cryo
