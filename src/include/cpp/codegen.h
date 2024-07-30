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
#ifndef CODEGEN_H
#define CODEGEN_H

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <assert.h>
#include <malloc.h>
#include <any>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/IR/ValueHandle.h"
#include "llvm/Support/FileSystem.h"
#include <llvm/Support/raw_ostream.h>

#include "cpp/cppLogger.h"

extern "C"
{
#include "compiler/ast.h"
#include "compiler/symtable.h"
}

namespace Cryo
{
    // Forward declarations


    /**
     * @class CodeGen
     * @brief Responsible for generating LLVM Intermediate Representation (IR) from the abstract syntax tree (AST).
     */
    class CodeGen
    {
    public:
        /**
         * @brief Constructs a CodeGen object and initializes the code generation process.
         * @param root The root of the abstract syntax tree (AST) to be processed.
         */
        CodeGen(ASTNode *root) 
        : context(), builder(context), module(nullptr)
        {
            module = std::make_unique<llvm::Module>("main", context);
            cryoSyntaxInstance = new class CryoSyntax(context, builder, module, root);
            cryoTypesInstance = new class CryoTypes(context, builder, module, root);
            cryoModulesInstance = new class CryoModules(context, builder, module, root);
        }

        /**
         * @brief Destructs the CodeGen object and cleans up the code generation process.
         */
        ~CodeGen();

        /**
         * @brief The Entry Point to the generation process.
         */
        void executeCodeGeneration(ASTNode *root);

    protected:
        /**
         * @brief The LLVM Context is the main entry point for the LLVM API.
         */
        llvm::LLVMContext context;

        /**
         * @brief The IRBuilder is used to create new instructions in the LLVM IR.
         */
        llvm::IRBuilder<> builder;

        /**
         * @brief The Module is the top-level container for all other LLVM IR objects.
         */
        std::unique_ptr<llvm::Module> module;

        /**
         * @brief A map for storing global variables with their corresponding LLVM values.
         */
        std::unordered_map<std::string, llvm::Value *> namedValues;

        /**
         * @brief Instances of the subclasses to handle specific code generation tasks.
         */

        CryoSyntax* cryoSyntaxInstance;
        CryoTypes* cryoTypesInstance;
        CryoModules* cryoModulesInstance;



        /**
         * @brief Identifies and processes the expressions in the AST nodes.
         * @param root The root of the abstract syntax tree (AST) to be processed.
         */
        void identifyNodeExpression(ASTNode *root);


    private:
    };

    /// -----------------------------------------------------------------------------------------------
    /**
     * @class CryoSyntax
     * @brief Extends CodeGen to provide additional syntax-specific code generation features.
     */
    class CryoSyntax : public CodeGen
    {
    public:
        /**
         * @brief Constructs a CryoSyntax object and initializes the syntax-specific code generation process.
         * @param root The root of the abstract syntax tree (AST) to be processed.
         */
        CryoSyntax(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, std::unique_ptr<llvm::Module>& module, ASTNode *root);

        ///
        /// Function Syntax @ syntax/functions.cpp
        ///
        /**
         * @brief Creates a default main function if one is not found in the program.
         */
        void createDefaultMainFunction();
        
        /**
         * @brief Generates the function prototype for the given ASTNode.
         * @param node The ASTNode representing the function.
         */
        void generateFunctionPrototype(ASTNode* node);
        
        /**
         * @brief Generates the LLVM code for a function.
         * @param node The ASTNode representing the function.
         */
        void generateFunction(ASTNode* node);

        /**
         * @brief Generates the LLVM code for an external declaration.
         * @param node The ASTNode representing the external declaration.
         */
        void generateExternalDeclaration(ASTNode* node);

        /**
         * @brief Generates the LLVM code for a return statement.
         * @param node The ASTNode representing the return statement.
         */
        void generateReturnStatement(ASTNode* node);

        /**
         * @brief Generates the LLVM code for a function call.
         * @param node The ASTNode representing the function call.
         */
        void generateFunctionCall(ASTNode* node);

        /**
         * @brief Generates the LLVM code for a function block.
         * @param node The ASTNode representing the function block.
         */
        void generateFunctionBlock(ASTNode* node);

        
        ///
        /// Expression Syntax @ syntax/expressions.cpp
        ///
        /**
         * @brief Generates the LLVM code for an expression.
         * @param node The ASTNode representing the expression.
         * @return The generated LLVM value.
         */
        llvm::Value* generateExpression(ASTNode* node);

        /**
         * @brief Generates the LLVM code for a statement.
         * @param node The ASTNode representing the statement.
         */
        void generateStatement(ASTNode* node);

        ///
        /// Variable Syntax @ syntax/variables.cpp
        ///
        /**
         * @brief Generates the LLVM code for a variable declaration.
         * @param node The ASTNode representing the variable declaration.
         */
        void generateVarDeclaration(ASTNode* node);
        
        /**
         * @brief Looks up a variable by name.
         * @param name The name of the variable to look up.
         * @return The LLVM value of the variable.
         */
        llvm::Value* lookupVariable(char* name);

        /**
         * @brief Creates a variable declaration in the LLVM IR.
         * @param node The ASTNode representing the variable declaration.
         * @return The LLVM value of the variable.
         */
        llvm::Value* createVariableDeclaration(ASTNode* node);
        
        /**
         * @brief Gets the value of a variable by name.
         * @param name The name of the variable.
         * @return The LLVM value of the variable.
         */
        llvm::Value* getVariableValue(char* name);
        
        /**
         * @brief Creates a global variable in the LLVM IR.
         * @param varType The LLVM type of the variable.
         * @param initialValue The initial value of the variable.
         * @param varName The name of the variable.
         * @return The created global variable.
         */
        llvm::GlobalVariable* createGlobalVariable(llvm::Type* varType, llvm::Constant* initialValue, char* varName);
        
        /**
         * @brief Loads a global variable from the LLVM IR.
         * @param globalVar The global variable to load.
         * @param name The name of the variable.
         * @return The loaded global variable.
         */
        llvm::Value* loadGlobalVariable(llvm::GlobalVariable* globalVar, char* name);


        /**
         * @brief Generates the LLVM code for an array literal.
         * @param node The ASTNode representing the array literal.
         * @return The generated LLVM value.
         */
        void generateArrayLiteral(ASTNode* node);

        /**
         * @brief Generates the LLVM code for the elements of an array literal.
         * @param arrayLiteral The ASTNode representing the array literal.
         * @return The generated LLVM values.
         */
        std::vector<llvm::Constant*> generateArrayElements(ASTNode* arrayLiteral);

        ///
        /// Binary Expression Syntax @ syntax/operations.cpp
        ///
        /**
         * @brief Generates the LLVM code for a binary operation.
         * @param node The ASTNode representing the binary operation.
         * @return The generated LLVM value.
         */
        llvm::Value* generateBinaryOperation(ASTNode* node);
    };


    /// -----------------------------------------------------------------------------------------------
    /**
     * @class CryoTypes
     * @brief Extends CodeGen to handle the generation and management of LLVM types.
     */
    class CryoTypes : public CodeGen
    {
    public:
        /**
         * @brief Constructs a CryoTypes object and initializes the type management process.
         * @param root The root of the abstract syntax tree (AST) to be processed.
         */
        CryoTypes(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, std::unique_ptr<llvm::Module>& module, ASTNode *root);

        /**
         * @brief Converts an LLVM type to a string for debugging purposes.
         * @param type The LLVM type to be converted.
         * @return A string representation of the LLVM type.
         */
        std::string LLVMTypeToString(llvm::Type *type);

        /**
         * @brief Converts a CryoDataType to an LLVM type.
         * @param type The CryoDataType to be converted.
         * @return The converted LLVM type.
         */
        llvm::Type* getLLVMType(CryoDataType type);

        /**
         * @brief Creates a new LLVM constant type based on the given CryoDataType.
         * @param type The CryoDataType to be converted.
         * @return The created LLVM type.
         */
        llvm::Type *createLLVMConstantType(CryoDataType type);

        /**
         * @brief Creates a new LLVM pointer type based on the given CryoDataType.
         * @param type The CryoDataType to be converted.
         * @return The created LLVM pointer type.
         */
        llvm::PointerType *createLLVMPointerType(CryoDataType type);

        /**
         * @brief Creates a new LLVM array type based on the given CryoDataType and size.
         * @param elementType The CryoDataType of the array elements.
         * @param size The size of the array.
         * @return The created LLVM array type.
         */
        llvm::ArrayType *createLLVMArrayType(CryoDataType elementType, unsigned int size);

        /**
         * @brief Creates a new LLVM struct type based on the given member types and optional name.
         * @param memberTypes The vector of CryoDataType representing the struct members.
         * @param name Optional name for the struct type.
         * @return The created LLVM struct type.
         */
        llvm::StructType *createLLVMStructType(const std::vector<CryoDataType> &memberTypes, const std::string &name = "");

        /**
         * @brief Creates a new LLVM function type based on the given return type, parameter types, and variadic flag.
         * @param returnType The CryoDataType of the function's return type.
         * @param paramTypes The vector of CryoDataType representing the function's parameters.
         * @param isVarArg Flag indicating if the function is variadic.
         * @return The created LLVM function type.
         */
        llvm::FunctionType *createLLVMFunctionType(CryoDataType returnType, const std::vector<CryoDataType> &paramTypes, bool isVarArg = false);

        /**
         * @brief Creates a new LLVM struct type for the String struct.
         * @return The created LLVM struct type for the String struct.
         */
        llvm::StructType *createStringStruct();

        /**
         * @brief Creates a new LLVM struct type for the String type.
         * @return The created LLVM struct type for the String type.
         */
        llvm::StructType *createStringType();

        /**
         * @brief Creates a new LLVM struct type for the Array type.
         * @param elementType The LLVM type of the array elements.
         * @return The created LLVM struct type for the Array type.
         */
        llvm::StructType *createArrayType(llvm::Type *elementType);

        /**
         * @brief Creates a new LLVM struct type for the Pair type.
         * @param firstType The LLVM type of the first element.
         * @param secondType The LLVM type of the second element.
         * @return The created LLVM struct type for the Pair type.
         */
        llvm::StructType *createPairType(llvm::Type *firstType, llvm::Type *secondType);

        /**
         * @brief Creates a new LLVM struct type for the Tuple type.
         * @param types The vector of LLVM types representing the tuple elements.
         * @return The created LLVM struct type for the Tuple type.
         */
        llvm::StructType *createTupleType(const std::vector<llvm::Type *> &types);

        /**
         * @brief Creates a new LLVM struct type for the Optional type.
         * @param valueType The LLVM type of the optional value.
         * @return The created LLVM struct type for the Optional type.
         */
        llvm::StructType *createOptionalType(llvm::Type *valueType);

        /**
         * @brief Creates a new LLVM struct type for the Variant type.
         * @param types The vector of LLVM types representing the variant possibilities.
         * @return The created LLVM struct type for the Variant type.
         */
        llvm::StructType *createVariantType(const std::vector<llvm::Type *> &types);

        /**
         * @brief Creates a new LLVM struct type for the Function type.
         * @param returnType The LLVM type of the function's return value.
         * @param paramTypes The vector of LLVM types representing the function's parameters.
         * @return The created LLVM struct type for the Function type.
         */
        llvm::StructType *createFunctionType(llvm::Type *returnType, const std::vector<llvm::Type *> &paramTypes);

        /**
         * @brief Creates a new LLVM value for a string.
         * @param str The string to be converted.
         * @return The created LLVM value for the string.
         */
        llvm::Value *createString(const std::string &str);

        /**
         * @brief Creates a new LLVM value for a number.
         * @param num The number to be converted.
         * @return The created LLVM value for the number.
        */
        llvm::Value *createNumber(int num);

        /**
         * @brief Creates a new LLVM value for a boolean.
         * @param value The boolean value to be converted.
         * @return The created LLVM value for the boolean.
         */
        llvm::Constant* createConstantInt(int value);

        /**
         * @brief Creates a new LLVM value for a reference to an integer.
         * @param value The integer value to be referenced.
         * @return The created LLVM value for the reference.
         */
        llvm::Value* createReferenceInt(int value);

    private:
    };

    /// -----------------------------------------------------------------------------------------------
    /**
     * @class CryoModules
     * @brief Extends CodeGen to manage and generate LLVM code for modules.
     */
    class CryoModules : public CodeGen
    {
    public:
        /**
         * @brief Constructs a CryoModules object and initializes the module management process.
         * @param root The root of the abstract syntax tree (AST) to be processed.
         */
        CryoModules(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, std::unique_ptr<llvm::Module>& module, ASTNode *root);

        /**
         * @brief Declares all functions in the program.
         * @param node The ASTNode representing the program.
         * @return True if the main function exists, false otherwise.
         */
        bool declareFunctions(ASTNode* node);

        /**
         * @brief Generates the LLVM code for the program.
         * @param node The ASTNode representing the program.
         */
        void generateProgram(ASTNode* node);

        /**
         * @brief Generates the LLVM code for a block.
         * @param node The ASTNode representing the block.
         */
        void generateBlock(ASTNode* node);

        /**
         * @brief Generates the LLVM code for a function block.
         * @param node The ASTNode representing the function block.
         */
        void generateFunctionBlock(ASTNode* node);

        /**
         * @brief Generates the LLVM code for a function declaration.
         * @param node The ASTNode representing the function declaration.
         */
        void generateExternalDeclaration(ASTNode* node);
    };

}

#endif // CODEGEN_H
