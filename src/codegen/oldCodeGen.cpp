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
#include "tools/logger/logger_config.h"

int generateCodeWrapper(ASTNode *node, CompilerState *state, CryoLinker *cLinker)
{
    DEBUG_PRINT_FILTER({
        std::cout << ">===------------- CPP Code Generation -------------===<\n"
                  << std::endl;
    });

    Cryo::CryoCompiler compiler;
    compiler.setCompilerState(state);
    compiler.setCompilerSettings(state->settings);
    compiler.isPreprocessing = false;

    std::string moduleName = state->fileName;
    compiler.setModuleIdentifier(moduleName);

    // Convert C opaque pointer back to C++ type
    Cryo::Linker *cppLinker = reinterpret_cast<Cryo::Linker *>(cLinker);
    compiler.setLinker(cppLinker);
    compiler.initDependencies();

    compiler.compile(node);

    return 0;
}

int preprocessRuntimeIR(ASTNode *runtimeNode, CompilerState *state, const char *outputPath, CryoLinker *cLinker)
{
    DEBUG_PRINT_FILTER({
        std::cout << ">===------------- CPP Runtime Generation -------------===<\n"
                  << std::endl;
        std::cout << "Output Path: " << outputPath << std::endl;
    });

    std::string moduleName = "runtime";
    Cryo::CryoCompiler compiler;

    compiler.setCompilerState(state);
    compiler.setCompilerSettings(state->settings);
    compiler.setModuleIdentifier(moduleName);

    // Convert C opaque pointer back to C++ type
    Cryo::Linker *cppLinker = reinterpret_cast<Cryo::Linker *>(cLinker);
    compiler.setLinker(cppLinker);

    // Set the output path for the runtime
    compiler.setCustomOutputPath(outputPath);
    // Compile Runtime Node
    compiler.compile(runtimeNode);

    return 0;
}

namespace Cryo
{

    /**
     * @brief The entry point to the generation process (passed from the top-level compiler).
     * @param node
     */
    void CodeGen::executeCodeGeneration(ASTNode *root)
    {
        CryoCompiler &compiler = this->compiler;
        assert(root != nullptr);

        compiler.getGenerator().generateCode(root);
    }

    /// @private
    void Generator::preprocess(ASTNode *root)
    {
        Declarations &declarations = compiler.getDeclarations();
        DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Preprocessing Code Generation");

        // Get / Set the symbol table for the module and its state
        std::string namespaceName = getNamespace(root);
        compiler.getSymTable().initModule(root, namespaceName);
        compiler.getContext().currentNamespace = namespaceName;

        DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Linting Tree");
        bool validateTree = DevDebugger::lintTree(root);
        if (validateTree == false)
        {
            std::cerr << "[CPP] Tree is invalid!" << std::endl;
            CONDITION_FAILED;
        }

        // Declare all functions in the AST tree
        // declarations.preprocessDeclare(root); <- TODO: Implement this function
        DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Preprocessing Complete");
        return;
    }

    /**
     * Generates code from the given AST tree.
     *
     * @param root The root node of the AST tree.
     */
    void Generator::generateCode(ASTNode *root)
    {
        CryoContext &cryoContext = compiler.getContext();
        Compilation compileCode = Compilation(compiler);

        // Check if the module is initialized
        assert(cryoContext.module != nullptr);

        // Preprocess the AST tree
        DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Preprocessing Code Generation");
        preprocess(root);

        // Parse the AST tree
        DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Parsing Tree");
        parseTree(root);

        // Compile the IR file
        DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Compiling IR File");
        compileCode.compileIRFile();

        DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Code CodeGen Complete");
        return;
    }

    // Just a name wrapper around `parseTree` to look cleaner
    void Generator::generateBlock(ASTNode *node)
    {
        parseTree(node);
    }

    /**
     * Parses the given AST tree starting from the root node.
     *
     * @param root The root node of the AST tree.
     */
    void Generator::parseTree(ASTNode *root)
    {

        DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Parsing Tree");

        if (!root)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "CodeGen", "Root is null!");
            CONDITION_FAILED;
        }

        Generator &generator = this->compiler.getGenerator();

        switch (root->metaData->type)
        {
        case NODE_PROGRAM:
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Program");
            generator.handleProgram(root);
            break;
        case NODE_IMPORT_STATEMENT:
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Import Statement");
            generator.handleImportStatement(root);
            break;
        case NODE_FUNCTION_DECLARATION:
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Function Declaration");
            generator.handleFunctionDeclaration(root);
            break;
        case NODE_FUNCTION_BLOCK:
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Function Block");
            generator.handleFunctionBlock(root);
            break;
        case NODE_RETURN_STATEMENT:
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Return Statement");
            generator.handleReturnStatement(root);
            break;
        case NODE_FUNCTION_CALL:
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Function Call");
            generator.handleFunctionCall(root);
            break;
        case NODE_VAR_DECLARATION:
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Variable Declaration");
            generator.handleVariableDeclaration(root);
            break;
        case NODE_BINARY_EXPR:
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Binary Expression");
            generator.handleBinaryExpression(root);
            break;
        case NODE_UNARY_EXPR:
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Unary Expression");
            generator.handleUnaryExpression(root);
            break;
        case NODE_LITERAL_EXPR:
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Literal Expression");
            generator.handleLiteralExpression(root);
            break;
        case NODE_IF_STATEMENT:
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling If Statement");
            generator.handleIfStatement(root);
            break;
        case NODE_WHILE_STATEMENT:
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling While Statement");
            generator.handleWhileStatement(root);
            break;
        case NODE_FOR_STATEMENT:
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling For Statement");
            generator.handleForStatement(root);
            break;
        case NODE_EXTERN_FUNCTION:
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Extern Function");
            generator.handleExternFunction(root);
            break;
        case NODE_NAMESPACE:
        {
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Namespace");
            std::string moduleName = std::string(root->data.cryoNamespace->name);
            compiler.getContext().module->setModuleIdentifier(moduleName);
            break;
        }
        case NODE_BLOCK:
        {
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Block");
            ASTNode **statements = root->data.block->statements;
            int statementCount = root->data.block->statementCount;
            if (statementCount == 0)
            {
                DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Empty Block");
                break;
            }
            for (int i = 0; i < statementCount; i++)
            {
                generator.parseTree(statements[i]);
            }
            break;
        }
        case NODE_INDEX_EXPR:
        {
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Index Expression");
            break;
        }
        case NODE_VAR_REASSIGN:
        {
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Variable Reassignment");
            generator.handleReassignment(root);
            break;
        }
        case NODE_VAR_NAME:
        {
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Variable Name");
            break;
        }
        case NODE_STRUCT_DECLARATION:
        {
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Struct Declaration");
            generator.handleStruct(root);
            break;
        }
        case NODE_SCOPED_FUNCTION_CALL:
        {
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Scoped Function Call");
            generator.handleScopedFunctionCall(root);
            break;
        }
        case NODE_METHOD_CALL:
        {
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Method Call");
            compiler.getFunctions().handleMethodCall(root);
            break;
        }
        case NODE_CLASS:
        {
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Class");
            compiler.getClasses().handleClassDeclaration(root);
            break;
        }
        case NODE_USING:
        {
            // Skip the using statement
            break;
        }
        case NODE_MODULE:
        {
            // Skip the module statement
            break;
        }
        default:
            DevDebugger::logMessage("ERROR", __LINE__, "CodeGen", "Unknown Node Type");
            std::cout << "Received: " << CryoNodeTypeToString(root->metaData->type) << std::endl;
            CONDITION_FAILED;
            break;
        }

        DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Tree Parsed");

        return;
    }

    llvm::Value *Generator::getInitilizerValue(ASTNode *node)
    {

        CryoContext &cryoContext = compiler.getContext();
        Variables &variables = compiler.getVariables();
        Generator &generator = compiler.getGenerator();
        OldTypes &types = compiler.getTypes();
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
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Literal Expression");
            llvmValue = generator.handleLiteralExpression(node);
            break;
        }
        case NODE_RETURN_STATEMENT:
        {
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Return Statement");
            DataType *dataType = node->data.returnStatement->type;

            llvmValue = functions.createReturnNode(node);
            if (!llvmValue)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "CodeGen", "Return Statement Value not found");
                exit(1);
            }

            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Return Statement Value Found");
            break;
        }
        case NODE_VAR_DECLARATION:
        {
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Variable Declaration");
            llvm::Value *locatedVar = variables.getVariable(node->data.varDecl->name);
            if (locatedVar)
            {
                DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Variable already exists");
                return locatedVar;
            }
            else
            {
                DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Variable does not exist");
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
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Variable Name");
            llvmValue = variables.getVariable(node->data.varName->varName);
            break;
        }
        case NODE_INDEX_EXPR:
        {
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Index Expression");
            llvmValue = arrays.getIndexExpressionValue(node);
            if (!llvmValue)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "CodeGen", "Index Expression Value not found");
                exit(1);
            }
            break;
        }
        case NODE_BINARY_EXPR:
        {
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Binary Expression");
            llvmValue = binaryExpressions.handleComplexBinOp(node);
            break;
        }
        case NODE_FUNCTION_CALL:
        {
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Function Call");
            llvmValue = functions.createFunctionCall(node);
            break;
        }
        case NODE_PARAM:
        {
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Parameter");
            llvmValue = functions.createParamFromParamNode(node);
            break;
        }
        case NODE_ARRAY_LITERAL:
        {
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Array Literal");
            llvmValue = arrays.createArrayLiteral(node);
            break;
        }
        case NODE_NULL_LITERAL:
        {
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Null Literal");
            llvmValue = llvm::ConstantPointerNull::get(llvm::PointerType::get(llvm::Type::getInt8Ty(cryoContext.context), 0));
            break;
        }
        case NODE_PROPERTY_ACCESS:
        {
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Property Access");
            llvmValue = functions.createPropertyAccessCall(node->data.propertyAccess);
            break;
        }
        case NODE_BOOLEAN_LITERAL:
        {
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Boolean Literal");
            bool boolValue = node->data.literal->value.booleanValue;
            llvmValue = llvm::ConstantInt::get(llvm::Type::getInt1Ty(cryoContext.context), boolValue, true);
            break;
        }
        default:
            DevDebugger::logMessage("ERROR", __LINE__, "CodeGen", "Unknown node type");
            std::cout << "Received: " << CryoNodeTypeToString(nodeType) << std::endl;
            exit(1);
        }

        return llvmValue;
    }

    llvm::Value *Generator::getLiteralValue(LiteralNode *literalNode)
    {
        OldTypes &types = compiler.getTypes();
        DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Getting Literal Value");

        llvm::Value *llvmValue = nullptr;
        llvm::Constant *llvmConstant = nullptr;

        DataType *dataType = literalNode->type;
        if (dataType->container->baseType == PRIMITIVE_TYPE)
        {
            switch (dataType->container->primitive)
            {
            case PRIM_INT:
            {
                DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Integer Literal");
                llvmValue = llvm::ConstantInt::get(compiler.getContext().context, llvm::APInt(32, literalNode->value.intValue, true));
                break;
            }
            case PRIM_FLOAT:
            {
                DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Float Literal");
                llvmValue = llvm::ConstantFP::get(compiler.getContext().context, llvm::APFloat(literalNode->value.floatValue));
                break;
            }
            case PRIM_BOOLEAN:
            {
                DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Boolean Literal");
                llvmValue = llvm::ConstantInt::get(compiler.getContext().context, llvm::APInt(1, literalNode->value.booleanValue, true));
                break;
            }
            case PRIM_STRING:
            {
                DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling String Literal");
                const char *cString = strdup(literalNode->value.stringValue);
                std::string formattedString = formatString(std::string(cString));
                llvmValue = llvm::ConstantDataArray::getString(compiler.getContext().context, formattedString);
                break;
            }
            case PRIM_ANY:
            {
                DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Any Literal");
                // Handle Any type
                DEBUG_BREAKPOINT;
                break;
            }
            case PRIM_VOID:
            {
                DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Handling Void Literal");
                llvmValue = llvm::UndefValue::get(llvm::Type::getVoidTy(compiler.getContext().context));
                break;
            }
            default:
            {
                DevDebugger::logMessage("ERROR", __LINE__, "CodeGen", "Unknown data type");
                std::cout << "Received: " << DataTypeToString(dataType) << std::endl;
                exit(1);
            }
            }
        }

        return llvmValue;
    }

    std::string Generator::formatString(std::string str)
    {
        std::string formattedString = "";
        for (int i = 0; i < str.length(); i++)
        {
            if (str[i] == '\\')
            {
                if (str[i + 1] == 'n')
                {
                    formattedString += '\n';
                    i++;
                }
                else if (str[i + 1] == 't')
                {
                    formattedString += '\t';
                    i++;
                }
                else if (str[i + 1] == 'r')
                {
                    formattedString += '\r';
                    i++;
                }
                else if (str[i + 1] == '0')
                {
                    formattedString += '\0';
                    i++;
                }
                else
                {
                    formattedString += str[i];
                }
            }
            else
            {
                formattedString += str[i];
            }
        }

        return formattedString;
    }

    std::string Generator::getNamespace(ASTNode *node)
    {

        DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Getting Namespace");

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
        return namespaceName;
    }

    void Generator::printCurrentNamedValues(void)
    {

        Variables &variables = compiler.getVariables();
        DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Printing Current Named Values");

        llvm::Function *currentFunction = compiler.getContext().builder.GetInsertBlock()->getParent();
        llvm::Function::arg_iterator args = currentFunction->arg_begin();
        llvm::Function::arg_iterator end = currentFunction->arg_end();

        for (; args != end; ++args)
        {
            llvm::Value *argValue = &*args;
            std::string argName = argValue->getName().str();
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
            }
        }

        DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Named Values Printed");
    }

    llvm::Value *Generator::getNamedValue(std::string name)
    {

        DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Getting Named Value");

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
            DevDebugger::logMessage("ERROR", __LINE__, "CodeGen", "Named Value not found");
            return nullptr;
        }

        if (llvmValue != nullptr)
        {
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Named Value Found");
        }

        return llvmValue;
    }

} // namespace Cryo
