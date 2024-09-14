#include "cpp/codegen.h"

namespace Cryo
{

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

    void Arrays::handleIndexExpression(ASTNode *node, std::string varName)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Arrays", "Handling Index Expression");
        IndexExprNode *indexNode = node->data.indexExpr;
        assert(indexNode != nullptr);
        debugger.logMessage("INFO", __LINE__, "Arrays", "Index Expression Node Found");

        // Get the array name
        char *arrayName = indexNode->name;
        std::cout << "Array Name: " << arrayName << std::endl;

        // Look up the array in the symbol table
        ASTNode *arrayNode = compiler.getSymTable().getASTNode(compiler.getContext().currentNamespace, NODE_VAR_DECLARATION, arrayName);

        int elementCount = getArrayLength(arrayNode);

        // Get the index of the literal expression
        // llvm::Value *index = compiler.getGenerator().handleLiteralExpression(indexNode->index);
        // debugger.logMessage("INFO", __LINE__, "Arrays", "Index Processed");

        int indexValue = indexNode->index->data.literal->value.intValue;
        std::cout << "Index Value: " << indexValue << std::endl;

        ASTNode *indexedArrayEl = nullptr;
        CryoDataType indexedArrayElType;
        LiteralNode *indexedArrayElLit = nullptr;
        llvm::Value *indexedArrayElValue = nullptr;
        llvm::Type *llvmType = nullptr;
        llvm::Constant *llvmConstant = nullptr;
        for (int i = 0; i < elementCount; ++i)
        {
            ASTNode *arrNode = arrayNode->data.array->elements[i];
            if (i == indexValue)
            {
                std::cout << "Element Found" << std::endl;
                indexedArrayEl = arrNode;
                indexedArrayElType = arrNode->data.literal->dataType;
                std::cout << "Element Type: " << CryoDataTypeToString(indexedArrayElType) << std::endl;
                std::cout << "Element Value: " << arrNode->data.literal->value.intValue << std::endl;
                indexedArrayElLit = arrNode->data.literal;
                debugger.logNode(indexedArrayEl);

                // Create the variable from the literal expression
                llvmType = compiler.getTypes().getType(indexedArrayElType, 0);
                switch (indexedArrayElType)
                {
                case DATA_TYPE_INT:
                {
                    llvmConstant = llvm::ConstantInt::get(llvmType, indexedArrayElLit->value.intValue);
                    break;
                }
                }
                break;
            }
        }

        // llvmConstant = llvm::dyn_cast<llvm::Constant>(indexedArrayElValue);

        // Create the variable
        llvm::GlobalVariable *var = new llvm::GlobalVariable(
            *compiler.getContext().module,
            llvmType,
            false,
            llvm::GlobalValue::ExternalLinkage,
            llvmConstant,
            llvm::Twine(varName));

        debugger.logMessage("INFO", __LINE__, "Arrays", "Index Expression Processed");
        return;
    }

} // namespace Cryo
