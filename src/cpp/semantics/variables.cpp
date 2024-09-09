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

        exit(0);
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
        CryoDataType type = varNode->type;
        ASTNode *initializer = varNode->initializer;

        llvm::Type *llvmType = nullptr;
        llvm::Value *llvmValue = nullptr;
        llvm::Constant *llvmConstant = nullptr;
        llvm::ArrayType *llvmArrayType = nullptr;

        if (initializer)
        {
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
                    llvm::GlobalValue::ExternalLinkage,
                    llvmConstant,
                    llvm::Twine(varName));
            }
        }
        debugger.logMessage("INFO", __LINE__, "Variables", "Variable Created");

        return;
    }

} // namespace Cryo
