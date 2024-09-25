#include "cpp/codegen.h"

namespace Cryo
{

    llvm::Value *Arrays::createArrayLiteral(ASTNode *node, std::string varName)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Arrays", "Creating Array Literal");

        CryoArrayNode *arrayNode = node->data.array;
        assert(arrayNode != nullptr);

        llvm::Type *llvmType = nullptr;
        llvm::Value *llvmValue = nullptr;
        llvm::ArrayType *llvmArrayType = nullptr;

        // Get the type of the array
        std::vector<llvm::Constant *> elements;
        for (int i = 0; i < arrayNode->elementCount; ++i)
        {
            ASTNode *element = arrayNode->elements[i];
            if (element->metaData->type == NODE_LITERAL_EXPR)
            {
                // Note to self, might need to get the length of each element
                CryoDataType dataType = element->data.literal->dataType;
                switch (dataType)
                {
                case DATA_TYPE_INT:
                {
                    llvmType = compiler.getTypes().getType(element->data.literal->dataType, 0);
                    llvm::Constant *llvmElement = llvm::ConstantInt::get(llvmType, element->data.literal->value.intValue);
                    elements.push_back(llvmElement);
                    break;
                }
                case DATA_TYPE_STRING:
                {
                    int _len = compiler.getTypes().getLiteralValLength(element);
                    llvmType = compiler.getTypes().getType(element->data.literal->dataType, _len + 1);
                    llvm::Constant *llvmElement = llvm::ConstantDataArray::getString(compiler.getContext().context, element->data.literal->value.stringValue);
                    elements.push_back(llvmElement);
                    break;
                }
                }
            }
            else
            {
                debugger.logMessage("ERROR", __LINE__, "Arrays", "Unknown element type");
                DEBUG_BREAKPOINT;
            }
        }
        debugger.logMessage("INFO", __LINE__, "Arrays", "Finished processing elements");

        // Create the array
        llvmArrayType = llvm::ArrayType::get(llvmType, arrayNode->elementCount);
        llvmValue = llvm::ConstantArray::get(llvmArrayType, elements);

        // Get the current block that we are in
        llvm::BasicBlock *block = compiler.getContext().builder.GetInsertBlock();
        if (!block)
        {
            debugger.logMessage("ERROR", __LINE__, "Arrays", "Block not found");
            exit(1);
        }

        // Alloc the array without initializing it
        llvm::Value *llvmAlloc = compiler.getContext().builder.CreateAlloca(llvmArrayType, nullptr, varName);
        // Store the array in the variable
        llvm::Value *llvmStore = compiler.getContext().builder.CreateStore(llvmValue, llvmAlloc);
        // Add the variable to the named values
        compiler.getContext().namedValues[varName] = llvmAlloc;

        debugger.logMessage("INFO", __LINE__, "Arrays", "Array Literal Created");

        return llvmAlloc;
    }

    /**
     * @brief Handles the array literal node. Processes the array elements and returns the LLVM value.
     * @param node The array literal node.
     * @return The LLVM value of the array literal.
     */
    void Arrays::handleArrayLiteral(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Arrays", "Handling Array Literal");

        CryoArrayNode *arrayNode = node->data.array;
        assert(arrayNode != nullptr);

        llvm::Type *llvmType = nullptr;
        llvm::Value *llvmValue = nullptr;
        llvm::Constant *llvmConstant = nullptr;
        llvm::ArrayType *llvmArrayType = nullptr;

        // Get the type of the array
        std::vector<llvm::Constant *> elements;
        for (int i = 0; i < arrayNode->elementCount; ++i)
        {
            ASTNode *element = arrayNode->elements[i];
            if (element->metaData->type == NODE_LITERAL_EXPR)
            {
                // Note to self, might need to get the length of each element
                CryoDataType dataType = element->data.literal->dataType;
                switch (dataType)
                {
                case DATA_TYPE_INT:
                {
                    llvmType = compiler.getTypes().getType(element->data.literal->dataType, 0);
                    llvm::Constant *llvmElement = llvm::ConstantInt::get(llvmType, element->data.literal->value.intValue);
                    elements.push_back(llvmElement);
                    break;
                }
                case DATA_TYPE_STRING:
                {
                    int _len = compiler.getTypes().getLiteralValLength(element);
                    llvmType = compiler.getTypes().getType(element->data.literal->dataType, _len + 1);
                    llvm::Constant *llvmElement = llvm::ConstantDataArray::getString(compiler.getContext().context, element->data.literal->value.stringValue);
                    elements.push_back(llvmElement);
                    break;
                }
                }
            }
            else
            {
                debugger.logMessage("ERROR", __LINE__, "Arrays", "Unknown element type");
                exit(1);
            }
        }

        // Create the array
        llvmArrayType = llvm::ArrayType::get(llvmType, arrayNode->elementCount);
        llvmValue = llvm::ConstantArray::get(llvmArrayType, elements);

        // Create the constant
        llvmConstant = llvm::dyn_cast<llvm::Constant>(llvmValue);

        // Create the global variable
        llvm::GlobalVariable *var = new llvm::GlobalVariable(
            *compiler.getContext().module,
            llvmArrayType,
            false,
            llvm::GlobalValue::ExternalLinkage,
            llvmConstant,
            llvm::Twine("arr"));

        debugger.logMessage("INFO", __LINE__, "Arrays", "Array Literal Processed");

        return;
    }

    llvm::ArrayType *Arrays::getArrayType(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Arrays", "Converting array to LLVM type");

        llvm::Type *llvmType = nullptr;
        llvm::ArrayType *llvmArrayType = nullptr;

        switch (node->metaData->type)
        {
        case NODE_ARRAY_LITERAL:
        {
            debugger.logMessage("INFO", __LINE__, "Arrays", "Converting array literal to LLVM type");

            llvmType = compiler.getTypes().getType(DATA_TYPE_INT, 0);
            llvmArrayType = llvm::ArrayType::get(llvmType, getArrayLength(node));
            break;
        }

        default:
            debugger.logMessage("ERROR", __LINE__, "Arrays", "Unknown node type");
            exit(1);
        }

        return llvmArrayType;
    }

    /**
     * @brief Get the length of an array.
     * Of type `node->data.array`.
     * (ASSERTS)
     * @param node The array node.
     * @return The length of the array.
     */
    int Arrays::getArrayLength(ASTNode *node)
    {
        CryoArrayNode *arrayNode = node->data.array;
        assert(arrayNode != nullptr);

        return arrayNode->elementCount;
    }

    // This will be creating the global variable.
    void Arrays::handleIndexExpression(ASTNode *node, std::string varName)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        BackendSymTable &symTable = compiler.getSymTable();
        CryoContext &context = compiler.getContext();
        Variables &variables = compiler.getVariables();
        Types &types = compiler.getTypes();
        debugger.logMessage("INFO", __LINE__, "Arrays", "Handling Index Expression");

        IndexExprNode *indexNode = node->data.indexExpr;
        assert(indexNode != nullptr);

        std::string arrayName = std::string(indexNode->name);

        if (node->metaData->type == NODE_INDEX_EXPR)
        {
            std::cout << "Node is a index expression" << std::endl;
            CryoNodeType indexNodeType = node->data.indexExpr->index->metaData->type;

            // Check if it's a literal `foo[5]`
            if (indexNodeType == NODE_LITERAL_EXPR)
            {
                std::cout << "Array Name: " << indexNode->name << std::endl;
                int indexValue = indexNode->index->data.literal->value.intValue;
                std::cout << "Index Value: " << indexValue << std::endl;
                ASTNode *arrayNode = symTable.getASTNode(context.currentNamespace, NODE_VAR_DECLARATION, arrayName);
                ASTNode *indexArrayNode = arrayNode->data.array->elements[indexValue];
                CryoNodeType indexArrayNodeType = indexArrayNode->metaData->type;
                if (indexArrayNodeType == NODE_LITERAL_EXPR)
                {
                    std::cout << "Index Array Node Type: " << CryoNodeTypeToString(indexArrayNodeType) << std::endl;
                    std::cout << "Element Type: " << CryoDataTypeToString(indexArrayNode->data.literal->dataType) << std::endl;
                    std::cout << "Element Value: " << indexArrayNode->data.literal->value.intValue << std::endl;
                    // Creat the variable from the literal expression
                    llvm::Type *llvmType = compiler.getTypes().getType(indexArrayNode->data.literal->dataType, 0);
                    llvm::Constant *llvmConstant = llvm::ConstantInt::get(llvmType, indexArrayNode->data.literal->value.intValue);
                    // Create the global variable
                    llvm::GlobalVariable *var = new llvm::GlobalVariable(
                        *context.module,
                        llvmType,
                        false,
                        llvm::GlobalValue::ExternalLinkage,
                        llvmConstant,
                        llvm::Twine(varName));
                }
                else
                {
                    std::cout << "Index Array Node Type: Unknown" << std::endl;
                    std::cout << "Received: " << CryoNodeTypeToString(indexArrayNodeType) << std::endl;
                    exit(0);
                }
                return;
            }
            // Check if it's a variable `foo[bar]`
            if (indexNodeType == NODE_VAR_NAME)
            {
                // The variable name of the index `[bar]`
                std::string indexVarName = indexNode->index->data.varName->varName;
                // The array it's indexing `foo: int[] = [1, 2, 3]`
                std::string arrayName = indexNode->name;
                // Get the variable value
                llvm::Value *varValuePtr = variables.getVariable(indexVarName);
                if (!varValuePtr)
                {
                    debugger.logMessage("ERROR", __LINE__, "Arrays", "Variable value not found");
                    // Create a new local variable
                    llvm::Value *localVar = variables.createLocalVariable(indexNode->index);
                    varValuePtr = localVar;
                }
                std::cout << "Variable Value: " << varValuePtr << std::endl;

                // Look up the array in the symbol table
                ASTNode *arrayNode = symTable.getASTNode(context.currentNamespace, NODE_VAR_DECLARATION, arrayName);
                if (!arrayNode)
                {
                    debugger.logMessage("ERROR", __LINE__, "Arrays", "Array not found");
                    exit(1);
                }
                int elementCount = getArrayLength(arrayNode);
                std::cout << "Element Count: " << elementCount << std::endl;
                // Get the literal value from the index variable
                // int indexValue = types.getLiteralIntValue(indexNode->index);
                std::cout << "Node Type: " << CryoNodeTypeToString(indexNode->index->metaData->type) << std::endl;
                std::cout << "Var Name: " << indexNode->index->data.varName->varName << std::endl;
                ASTNode *indexNodeVar = symTable.getASTNode(context.currentNamespace, NODE_VAR_DECLARATION, indexNode->index->data.varName->varName);
                std::cout << "Index Node Var: " << indexNodeVar << std::endl;
                if (!indexNodeVar)
                {
                    debugger.logMessage("ERROR", __LINE__, "Arrays", "Index variable not found");
                    exit(1);
                }
                ASTNode *indexedArrayValue = nullptr;
                llvm::Value *indexedValue = nullptr;
                if (indexNodeVar->metaData->type == NODE_LITERAL_EXPR)
                {
                    debugger.logMessage("INFO", __LINE__, "Arrays", "Index variable is a literal");
                    int _indexValue = types.getLiteralIntValue(indexNodeVar->data.literal);
                    indexedArrayValue = arrayNode->data.array->elements[_indexValue];
                    indexedValue = compiler.getGenerator().getInitilizerValue(indexedArrayValue);
                }

                llvm::Constant *constValue = llvm::dyn_cast<llvm::Constant>(indexedValue);

                // Create the global variable
                llvm::GlobalVariable *var = new llvm::GlobalVariable(
                    *context.module,
                    llvm::Type::getInt32Ty(context.context),
                    false,
                    llvm::GlobalValue::ExternalLinkage,
                    constValue,
                    llvm::Twine(varName));

                return;
            }
            debugger.logMessage("ERROR", __LINE__, "Arrays", "Unknown index type");
            return;
        }
        debugger.logMessage("ERROR", __LINE__, "Arrays", "Unknown node type");
        return;
    }

    llvm::Value *Arrays::indexArrayForValue(ASTNode *array, int index)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Array", "Indexing Array for Value");

        CryoArrayNode *arrayNode = array->data.array;
        assert(arrayNode != nullptr);

        for (int i = 0; i < arrayNode->elementCount; ++i)
        {
            ASTNode *element = arrayNode->elements[i];
            if (i == index)
            {
                debugger.logMessage("INFO", __LINE__, "Array", "Index Found");
                return compiler.getGenerator().getInitilizerValue(element);
            }
            else
            {
                debugger.logMessage("INFO", __LINE__, "Array", "Index Not Found");
            }
        }
        debugger.logMessage("ERROR", __LINE__, "Array", "Failed to index array for value");
    }

    llvm::Value *Arrays::getIndexExpressionValue(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        BackendSymTable &symTable = compiler.getSymTable();
        CryoContext &context = compiler.getContext();
        debugger.logMessage("INFO", __LINE__, "Arrays", "Getting Index Expression Value");

        IndexExprNode *indexNode = node->data.indexExpr;
        assert(indexNode != nullptr);

        debugger.logMessage("INFO", __LINE__, "Arrays", "Index Node Found");

        std::string arrayName = std::string(indexNode->name);
        debugger.logMessage("INFO", __LINE__, "Arrays", "Array Name: " + arrayName);

        // This should be returning the value of the indexed array
        llvm::Value *indexedValue = nullptr;
        ASTNode *array = symTable.getASTNode(context.currentNamespace, NODE_VAR_DECLARATION, arrayName);
        if (!array)
        {
            debugger.logMessage("ERROR", __LINE__, "Arrays", "Array not found");
            exit(1);
        }
        int elementCount = getArrayLength(array);

        // Get the literal value from the index variable
        ASTNode *literalNode = indexNode->index;
        if (literalNode->metaData->type == NODE_VAR_NAME)
        {
            ASTNode *indexNodeVar = symTable.getASTNode(context.currentNamespace, NODE_VAR_DECLARATION, literalNode->data.varName->varName);
            if (!indexNodeVar)
            {
                debugger.logMessage("ERROR", __LINE__, "Arrays", "Index variable not found");
                exit(1);
            }
            if (indexNodeVar->metaData->type == NODE_LITERAL_EXPR)
            {
                debugger.logMessage("INFO", __LINE__, "Arrays", "Index variable is a literal");
                int _indexValue = compiler.getTypes().getLiteralIntValue(indexNodeVar->data.literal);
                indexedValue = indexArrayForValue(array, _indexValue);

                return indexedValue;
            }
        }
        if (literalNode->metaData->type == NODE_LITERAL_EXPR)
        {
            if (literalNode->data.literal->dataType == DATA_TYPE_INT)
            {
                int _indexValue = literalNode->data.literal->value.intValue;
                indexedValue = indexArrayForValue(array, _indexValue);

                return indexedValue;
            }
            else
            {
                debugger.logMessage("ERROR", __LINE__, "Arrays", "Unknown literal type");
                exit(1);
            }
        }

        debugger.logMessage("INFO", __LINE__, "Arrays", "Index Expression Value Found");

        return indexedValue;
    }

} // namespace Cryo
