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
#include "cpp/codegen.h"

namespace Cryo
{

    /**
     * @brief The entry point to the generation process (passed from the top-level compiler).
     * @param node
     */
    void CodeGen::executeCodeGeneration(ASTNode *root)
    {
        std::cout << "[CPP] Executing Code Generation" << std::endl;

        CryoCompiler &compiler = this->compiler;
        assert(root != nullptr);

        compiler.getGenerator().generateCode(root);
    }

    /// @private
    void Generator::preprocess(ASTNode *root)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Declarations &declarations = compiler.getDeclarations();
        debugger.logMessage("INFO", __LINE__, "CodeGen", "Preprocessing Code Generation");

        // Get / Set the symbol table for the module and its state
        std::string namespaceName = getNamespace(root);
        compiler.getSymTable().initModule(root, namespaceName);
        compiler.getContext().currentNamespace = namespaceName;

        debugger.logMessage("INFO", __LINE__, "CodeGen", "Linting Tree");
        bool validateTree = debugger.lintTree(root);
        if (validateTree == false)
        {
            std::cerr << "[CPP] Tree is invalid!" << std::endl;
            CONDITION_FAILED;
        }

        // Declare all functions in the AST tree
        // declarations.preprocessDeclare(root); <- TODO: Implement this function

        debugger.logMessage("INFO", __LINE__, "CodeGen", "Preprocessing Complete");
        return;
    }

    /**
     * Generates code from the given AST tree.
     *
     * @param root The root node of the AST tree.
     */
    void Generator::generateCode(ASTNode *root)
    {
        std::cout << "[CPP] Generating Code" << std::endl;
        CryoDebugger &debugger = compiler.getDebugger();
        CryoContext &cryoContext = compiler.getContext();
        assert(cryoContext.module != nullptr);

        // Preprocess the AST tree
        preprocess(root);

        debugger.logMessage("INFO", __LINE__, "CodeGen", "Parsing Tree");
        parseTree(root);

        if (llvm::verifyModule(*cryoContext.module, &llvm::errs()))
        {
            std::cout << "\n>===------- Error: LLVM module verification failed -------===<\n";
            cryoContext.module->print(llvm::errs(), nullptr);
            std::cout << "\n>===----------------- End Error -----------------===<\n";
            debugger.logMessage("ERROR", __LINE__, "CodeGen", "LLVM module verification failed");
            exit(1);
        }
        else
        {
            addWhitespaceAfterLoadStore(*cryoContext.module);
            std::error_code EC;
            std::string outputFileName = cryoContext.module->getSourceFileName();
            // Trim out the file extension
            outputFileName = outputFileName.substr(0, outputFileName.find_last_of("."));
            outputFileName += ".ll";
            // std::string outputFilename = cryoContext.module->getModuleIdentifier() + ".ll";
            llvm::raw_fd_ostream dest(outputFileName, EC, llvm::sys::fs::OF_None);

            if (EC)
            {
                debugger.logMessage("ERROR", __LINE__, "CodeGen", "Error opening file for writing");
            }
            else
            {
                std::cout << "\n>===------- LLVM IR Code -------===<\n"
                          << std::endl;
                // Create our custom annotator
                LoadStoreWhitespaceAnnotator LSWA;

                // Use the custom annotator when printing
                cryoContext.module->print(dest, &LSWA);
                cryoContext.module->print(llvm::outs(), &LSWA);
                std::cout << "\n>===------- End IR Code ------===<\n"
                          << std::endl;

                bool isActiveBuild = compiler.getCompilerState().isActiveBuild;
                if (isActiveBuild)
                {
                    debugger.logMessage("INFO", __LINE__, "CodeGen", "Active Build");
                    // Create the IR File
                    std::string _irFileName = cryoContext.state.fileName;
                    std::string irFileName = _irFileName.substr(0, _irFileName.find_last_of("."));
                    // Trim the directory path
                    irFileName = irFileName.substr(irFileName.find_last_of("/") + 1);
                    irFileName += ".ll";
                    std::cout << "IR File Name: " << irFileName << std::endl;
                    // Current working directory
                    std::string cwd = std::filesystem::current_path().string();
                    std::string irFilePath = cwd + "/" + irFileName;
                    std::cout << "IR File Path: " << irFilePath << std::endl;

                    std::error_code EC;
                    llvm::raw_fd_ostream irFile(irFilePath, EC, llvm::sys::fs::OF_None);
                    if (EC)
                    {
                        std::cerr << "Could not open file: " << EC.message() << "\n";
                        return;
                    }

                    // Write to irFile directly
                    llvm::raw_fd_ostream irFileOut(irFilePath, EC, llvm::sys::fs::OF_None);
                    cryoContext.module->print(irFileOut, nullptr);
                    irFileOut.flush();
                    irFileOut.close();
                }

                dest.flush();
                dest.close();

                debugger.logMessage("INFO", __LINE__, "CodeGen", "Code CodeGen Complete");
            }
        }

        debugger.logMessage("INFO", __LINE__, "CodeGen", "Code CodeGen Complete");
        return;
    }

    /**
     * Parses the given AST tree starting from the root node.
     *
     * @param root The root node of the AST tree.
     */
    void Generator::parseTree(ASTNode *root)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logNode(root);
        debugger.logMessage("INFO", __LINE__, "CodeGen", "Parsing Tree");

        if (!root)
        {
            debugger.logMessage("ERROR", __LINE__, "CodeGen", "Root is null!");
            CONDITION_FAILED;
        }

        Generator &generator = this->compiler.getGenerator();

        switch (root->metaData->type)
        {
        case NODE_PROGRAM:
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Program");
            generator.handleProgram(root);
            break;
        case NODE_IMPORT_STATEMENT:
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Import Statement");
            generator.handleImportStatement(root);
            break;
        case NODE_FUNCTION_DECLARATION:
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Function Declaration");
            generator.handleFunctionDeclaration(root);
            break;
        case NODE_FUNCTION_BLOCK:
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Function Block");
            generator.handleFunctionBlock(root);
            break;
        case NODE_RETURN_STATEMENT:
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Return Statement");
            generator.handleReturnStatement(root);
            break;
        case NODE_FUNCTION_CALL:
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Function Call");
            generator.handleFunctionCall(root);
            break;
        case NODE_VAR_DECLARATION:
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Variable Declaration");
            generator.handleVariableDeclaration(root);
            break;
        case NODE_BINARY_EXPR:
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Binary Expression");
            generator.handleBinaryExpression(root);
            break;
        case NODE_UNARY_EXPR:
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Unary Expression");
            generator.handleUnaryExpression(root);
            break;
        case NODE_LITERAL_EXPR:
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Literal Expression");
            generator.handleLiteralExpression(root);
            break;
        case NODE_IF_STATEMENT:
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling If Statement");
            generator.handleIfStatement(root);
            break;
        case NODE_WHILE_STATEMENT:
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling While Statement");
            generator.handleWhileStatement(root);
            break;
        case NODE_FOR_STATEMENT:
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling For Statement");
            generator.handleForStatement(root);
            break;
        case NODE_EXTERN_FUNCTION:
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Extern Function");
            generator.handleExternFunction(root);
            break;
        case NODE_NAMESPACE:
        {
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Namespace");
            // set the module name
            std::string moduleName = std::string(root->data.cryoNamespace->name);
            std::cout << "Module Name: " << moduleName << std::endl;
            compiler.getContext().module->setModuleIdentifier(moduleName);
            // Set the namespace name
            break;
        }
        case NODE_BLOCK:
        {
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Block");
            ASTNode **statements = root->data.block->statements;
            int statementCount = root->data.block->statementCount;
            for (int i = 0; i < statementCount; i++)
            {
                generator.parseTree(statements[i]);
            }
            break;
        }
        case NODE_INDEX_EXPR:
        {
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Index Expression");
            break;
        }
        case NODE_VAR_REASSIGN:
        {
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Variable Reassignment");
            generator.handleReassignment(root);
            break;
        }
        case NODE_VAR_NAME:
        {
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Variable Name");
            break;
        }
        case NODE_STRUCT_DECLARATION:
        {
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Struct Declaration");
            generator.handleStruct(root);
            break;
        }
        case NODE_SCOPED_FUNCTION_CALL:
        {
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Scoped Function Call");
            generator.handleScopedFunctionCall(root);
            break;
        }
        default:
            debugger.logMessage("ERROR", __LINE__, "CodeGen", "Unknown Node Type");
            std::cout << "Received: " << CryoNodeTypeToString(root->metaData->type) << std::endl;
            CONDITION_FAILED;
            break;
        }

        debugger.logMessage("INFO", __LINE__, "CodeGen", "Tree Parsed");

        return;
    }

    llvm::Value *Generator::getInitilizerValue(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        CryoContext &cryoContext = compiler.getContext();
        Variables &variables = compiler.getVariables();
        Generator &generator = compiler.getGenerator();
        Types &types = compiler.getTypes();
        Arrays &arrays = compiler.getArrays();
        Functions &functions = compiler.getFunctions();
        BinaryExpressions &binaryExpressions = compiler.getBinaryExpressions();

        CryoNodeType nodeType = node->metaData->type;
        llvm::Value *llvmValue = nullptr;
        llvm::Constant *llvmConstant = nullptr;

        switch (nodeType)
        {
        case NODE_LITERAL_EXPR:
        {
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Literal Expression");
            llvmValue = generator.handleLiteralExpression(node);
            break;
        }
        case NODE_RETURN_STATEMENT:
        {
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Return Statement");
            CryoDataType dataType = node->data.returnStatement->returnType;
            std::cout << "Data Type: " << CryoDataTypeToString(dataType) << std::endl;

            switch (dataType)
            {
            case DATA_TYPE_INT:
            case DATA_TYPE_FLOAT:
            case DATA_TYPE_BOOLEAN:
            {
                debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Literal Expression (int/float/boolean)");
                llvmValue = generator.handleLiteralExpression(node->data.returnStatement->expression);
                break;
            }
            case DATA_TYPE_STRING:
            {
                debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Literal Expression (string)");
                llvmValue = generator.handleLiteralExpression(node->data.returnStatement->expression);
                break;
            }
            case DATA_TYPE_VOID:
            {
                debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Literal Expression (void)");
                llvmValue = llvm::UndefValue::get(llvm::Type::getVoidTy(cryoContext.context));
                debugger.logMessage("INFO", __LINE__, "CodeGen", "Returning Constant");
                break;
            }
            default:
            {
                debugger.logMessage("ERROR", __LINE__, "CodeGen", "Unknown data type");
                std::cout << "Received: " << CryoDataTypeToString(dataType) << std::endl;
                exit(1);
            }
            }
            break;
        }
        case NODE_VAR_DECLARATION:
        {
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Variable Declaration");
            llvm::Value *locatedVar = variables.getVariable(node->data.varDecl->name);
            if (locatedVar)
            {
                debugger.logMessage("INFO", __LINE__, "CodeGen", "Variable already exists");
                return locatedVar;
            }
            else
            {
                debugger.logMessage("INFO", __LINE__, "CodeGen", "Variable does not exist");
                if (node->metaData->type == NODE_PARAM)
                {
                    DEBUG_BREAKPOINT;
                    return llvmValue;
                }
                llvmValue = variables.createLocalVariable(node);
            }
            break;
        }
        case NODE_VAR_NAME:
        {
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Variable Name");
            llvmValue = variables.getVariable(node->data.varName->varName);
            break;
        }
        case NODE_INDEX_EXPR:
        {
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Index Expression");
            llvmValue = arrays.getIndexExpressionValue(node);
            if (!llvmValue)
            {
                debugger.logMessage("ERROR", __LINE__, "CodeGen", "Index Expression Value not found");
                exit(1);
            }
            break;
        }
        case NODE_BINARY_EXPR:
        {
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Binary Expression");
            llvmValue = binaryExpressions.handleComplexBinOp(node);
            break;
        }
        case NODE_FUNCTION_CALL:
        {
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Function Call");
            generator.handleFunctionCall(node);
            break;
        }
        case NODE_PARAM:
        {
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Parameter");
            // llvmValue = functions.createParameter(node);
            DEBUG_BREAKPOINT;
            break;
        }
        case NODE_ARRAY_LITERAL:
        {
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Array Literal");
            llvmValue = arrays.createArrayLiteral(node);
            break;
        }
        default:
            debugger.logMessage("ERROR", __LINE__, "CodeGen", "Unknown node type");
            std::cout << "Received: " << CryoNodeTypeToString(nodeType) << std::endl;
            exit(1);
        }

        return llvmValue;
    }

    std::string Generator::getNamespace(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "CodeGen", "Getting Namespace");

        std::string namespaceName = "";

        int count = node->data.program->statementCount;
        for (int i = 0; i < count; i++)
        {
            CryoNodeType nodeType = node->data.program->statements[i]->metaData->type;
            if (nodeType == NODE_NAMESPACE)
            {
                namespaceName = node->data.program->statements[i]->data.cryoNamespace->name;
                break;
            }
        }
        std::cout << "Namespace: " << namespaceName << std::endl;

        return namespaceName;
    }

    void Generator::printCurrentNamedValues(void)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Variables &variables = compiler.getVariables();
        debugger.logMessage("INFO", __LINE__, "CodeGen", "Printing Current Named Values");

        llvm::Function *currentFunction = compiler.getContext().builder.GetInsertBlock()->getParent();
        llvm::Function::arg_iterator args = currentFunction->arg_begin();
        llvm::Function::arg_iterator end = currentFunction->arg_end();

        for (; args != end; ++args)
        {
            llvm::Value *argValue = &*args;
            std::string argName = argValue->getName().str();
            std::cout << "Arg Name: " << argName << std::endl;
        }

        llvm::Function::iterator block = currentFunction->begin();
        llvm::Function::iterator endBlock = currentFunction->end();

        for (; block != endBlock; ++block)
        {
            llvm::BasicBlock::iterator inst = block->begin();
            llvm::BasicBlock::iterator endInst = block->end();

            for (; inst != endInst; ++inst)
            {
                llvm::Instruction *instruction = &*inst;
                std::string instName = instruction->getName().str();
                std::cout << "Inst Name: " << instName << std::endl;
            }
        }

        debugger.logMessage("INFO", __LINE__, "CodeGen", "Named Values Printed");
    }

    llvm::Value *Generator::getNamedValue(std::string name)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "CodeGen", "Getting Named Value");

        llvm::Function *currentFunction = compiler.getContext().builder.GetInsertBlock()->getParent();
        llvm::Function::arg_iterator args = currentFunction->arg_begin();
        llvm::Function::arg_iterator end = currentFunction->arg_end();
        llvm::Value *llvmValue = nullptr;

        for (; args != end; ++args)
        {
            llvm::Value *argValue = &*args;
            std::string argName = argValue->getName().str();
            if (argName == name)
            {
                llvmValue = argValue;
                break;
            }
        }

        llvm::Function::iterator block = currentFunction->begin();
        llvm::Function::iterator endBlock = currentFunction->end();

        for (; block != endBlock; ++block)
        {
            llvm::BasicBlock::iterator inst = block->begin();
            llvm::BasicBlock::iterator endInst = block->end();

            for (; inst != endInst; ++inst)
            {
                llvm::Instruction *instruction = &*inst;
                std::string instName = instruction->getName().str();
                if (instName == name)
                {
                    llvmValue = instruction;
                    break;
                }
            }
        }

        if (!llvmValue)
        {
            debugger.logMessage("ERROR", __LINE__, "CodeGen", "Named Value not found");
            return nullptr;
        }

        if (llvmValue != nullptr)
        {
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Named Value Found");
        }

        return llvmValue;
    }

} // namespace Cryo
