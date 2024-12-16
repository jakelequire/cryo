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

namespace Cryo
{
    void BinaryExpressions::handleBinaryExpression(ASTNode *node)
    {
        DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Handling Binary Expression");

        DEBUG_BREAKPOINT;
    }

    /// @brief
    /// This function should replace the `createBinaryExpression` function.
    /// It should be able to handle multiple expressions within the binary expression.
    llvm::Value *BinaryExpressions::handleComplexBinOp(ASTNode *node)
    {
        IRSymTable &symTable = compiler.getSymTable();
        OldTypes &types = compiler.getTypes();
        std::string namespaceName = compiler.getContext().currentNamespace;
        DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Handling Complex Binary Operation");

        if (node->metaData->type != NODE_BINARY_EXPR)
        {
            DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Not a binary expression");
            // If it's not a binary expression, just return the value
            return compiler.getGenerator().getInitilizerValue(node);
        }

        bool isStringBinOp = false;
        llvm::Value *result = nullptr;
        ASTNode *currentNode = node;
        CryoOperatorType op = currentNode->data.bin_op->op;

        compiler.getContext().builder.SetInsertPoint(compiler.getContext().builder.GetInsertBlock());

        while (currentNode->metaData->type == NODE_BINARY_EXPR)
        {
            ASTNode *leftNode = currentNode->data.bin_op->left;
            ASTNode *rightNode = currentNode->data.bin_op->right;

            bool isStringBinOp = isStringOperation(leftNode, rightNode);
            if (isStringBinOp)
            {
                DevDebugger::logMessage("INFO", __LINE__, "BinExp", "String Binary Operation");
                result = createStringBinOpInitializer(leftNode, rightNode, op, "unknown");

                if (!result)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to create string binary operation");
                    CONDITION_FAILED;
                }

                break;
            }

            // The left value:
            llvm::Value *leftValue;
            CryoNodeType leftNodeType = leftNode->metaData->type;
            switch (leftNodeType)
            {
            case NODE_VAR_NAME:
            {
                DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Getting left value");
                std::string varName = leftNode->data.varName->varName;
                std::cout << "<!> (left) Variable Name: " << varName << std::endl;

                STVariable *symTableVar = symTable.getVariable(namespaceName, varName);
                if (!symTableVar)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to get variable from symtable");
                    CONDITION_FAILED;
                }

                leftValue = symTableVar->LLVMValue;
                if (!leftValue)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to get left value");
                    CONDITION_FAILED;
                }

                if (leftValue->getType()->isPointerTy())
                {
                    DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Creating temporary value for pointer");

                    llvm::LoadInst *isLoadInst = symTableVar->LLVMLoadInst;
                    if (isLoadInst)
                    {
                        leftValue = isLoadInst;

                        DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Using LoadInst");

                        if (!leftValue)
                        {
                            DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to create temporary value for pointer");
                            CONDITION_FAILED;
                        }

                        break;
                    }
                    // -----------------------
                    leftValue = dereferenceElPointer(leftValue, varName);
                    if (!leftValue)
                    {
                        DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to create temporary value for pointer");
                        CONDITION_FAILED;
                    }
                }
                else
                {
                    DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Left value is not a pointer");

                    llvm::Value *tempValue = compiler.getGenerator().getInitilizerValue(leftNode);
                }
                break;
            }
            case NODE_LITERAL_EXPR:
            {
                DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Getting left value");
                leftValue = compiler.getGenerator().getInitilizerValue(leftNode);
                if (!leftValue)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to get left value");
                    CONDITION_FAILED;
                }
                break;
            }
            default:
            {
                DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Getting left value");
                std::cout << "Unknown node type: " << CryoNodeTypeToString(leftNodeType) << std::endl;

                leftValue = compiler.getGenerator().getInitilizerValue(leftNode);
                if (!leftValue)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to get left value");
                    CONDITION_FAILED;
                }
            }
            }

            // The right value:
            llvm::Value *rightValue;
            CryoNodeType rightNodeType = rightNode->metaData->type;
            switch (rightNodeType)
            {
            case NODE_VAR_NAME:
            {
                DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Getting right value");
                std::string varName = rightNode->data.varName->varName;
                std::cout << "<!> (right) VarName: " << varName << std::endl;
                rightValue = compiler.getVariables().getVariable(varName);
                if (!rightValue)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to get right value");
                    CONDITION_FAILED;
                }
                if (rightValue->getType()->isPointerTy())
                {
                    DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Creating temporary value for pointer");
                    rightValue = dereferenceElPointer(rightValue, varName);
                    if (!rightValue)
                    {
                        DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to create temporary value for pointer");
                        CONDITION_FAILED;
                    }
                }
                break;
            }
            case NODE_LITERAL_EXPR:
            {
                DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Getting right value");
                rightValue = compiler.getGenerator().getInitilizerValue(rightNode);
                if (!rightValue)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to get right value");
                    CONDITION_FAILED;
                }
                break;
            }
            default:
            {
                DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Getting right value");
                std::cout << "Unknown node type: " << CryoNodeTypeToString(rightNodeType) << std::endl;

                rightValue = compiler.getGenerator().getInitilizerValue(rightNode);
                if (!rightValue)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to get right value");
                    CONDITION_FAILED;
                }
            }
            }

            if (!result)
            {
                // First iteration
                DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Creating binary expression");
                result = createBinaryExpression(currentNode, leftValue, rightValue);
            }
            else
            {
                // Subsequent iterations
                DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Creating binary expression");
                result = createBinaryExpression(currentNode, result, rightValue);
            }

            if (!result)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to create binary expression");
                return nullptr;
            }
            // -----------------------
            // Move to the next node (if any)
            if (rightNode->metaData->type == NODE_BINARY_EXPR)
            {
                DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Moving to next binary expression");
                currentNode = rightNode;
            }
            else
            {
                break;
            }
        }

        assert(result != nullptr);

        DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Binary expression handled successfully");
        return result;
    }

    llvm::Value *BinaryExpressions::createBinaryExpression(ASTNode *node, llvm::Value *leftValue, llvm::Value *rightValue)
    {
        OldTypes &types = compiler.getTypes();
        DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Creating Binary Expression");

        if (!leftValue || !rightValue)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Invalid operands for binary expression");
            CONDITION_FAILED;
        }

        DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Getting data types for binary expression");
        logASTNode(node);
        // DataType *leftDataType = node->data.bin_op->left->data.literal->type;
        // DataType *rightDataType = node->data.bin_op->right->data.literal->type;
        // llvm::Type *leftType = types.getType(leftDataType, 0);
        // llvm::Type *rightType = types.getType(rightDataType, 0);
        // bool sameType = leftType == rightType;
        // if (!sameType)
        // {
        //     DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Operands are not the same type");
        //     CONDITION_FAILED;
        // }
        // if (!leftType || !rightType)
        // {
        //     DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to get types for binary expression");
        //     CONDITION_FAILED;
        // }

        llvm::Value *result = nullptr;
        CryoOperatorType operatorType = node->data.bin_op->op;
        switch (operatorType)
        {
        case OPERATOR_ADD:
        {
            DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Creating addition expression");
            result = compiler.getContext().builder.CreateAdd(leftValue, rightValue, "addtmp");
            DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Setting no signed wrap");
            break;
        }
        case OPERATOR_SUB:
            DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Creating subtraction expression");
            result = compiler.getContext().builder.CreateSub(leftValue, rightValue, "subtmp");
            break;
        case OPERATOR_MUL:
            DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Creating multiplication expression");
            result = compiler.getContext().builder.CreateMul(leftValue, rightValue, "multmp");
            break;
        case OPERATOR_DIV:
            DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Creating division expression");
            result = compiler.getContext().builder.CreateSDiv(leftValue, rightValue, "divtmp");
            break;
        case OPERATOR_MOD:
            DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Creating modulo expression");
            result = compiler.getContext().builder.CreateSRem(leftValue, rightValue, "modtmp");
            break;
        case OPERATOR_AND:
            DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Creating and expression");
            result = compiler.getContext().builder.CreateAnd(leftValue, rightValue, "andtmp");
            break;
        case OPERATOR_OR:
            DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Creating or expression");
            result = compiler.getContext().builder.CreateOr(leftValue, rightValue, "ortmp");
            break;
        case OPERATOR_LT:
            DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Creating less than expression");
            result = compiler.getContext().builder.CreateICmpSLT(leftValue, rightValue, "ltcmp");
            break;
        case OPERATOR_LTE:
            DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Creating less than or equal expression");
            result = compiler.getContext().builder.CreateICmpSLE(leftValue, rightValue, "ltecmp");
            break;
        case OPERATOR_GT:
            DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Creating greater than expression");
            result = compiler.getContext().builder.CreateICmpSGT(leftValue, rightValue, "gtcmp");
            break;
        case OPERATOR_GTE:
            DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Creating greater than or equal expression");
            result = compiler.getContext().builder.CreateICmpSGE(leftValue, rightValue, "gtecmp");
            break;
        case OPERATOR_EQ:
            DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Creating equal expression");
            result = compiler.getContext().builder.CreateICmpEQ(leftValue, rightValue, "eqcmp");
            break;
        case OPERATOR_NEQ:
            DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Creating not equal expression");
            result = compiler.getContext().builder.CreateICmpNE(leftValue, rightValue, "neqcmp");
            break;
        default:
            DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Unknown operator type");
            CONDITION_FAILED;
        }

        if (!result)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to create binary expression");
            CONDITION_FAILED;
        }

        DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Binary expression created successfully");
        return result;
    }

    llvm::Value *BinaryExpressions::dereferenceElPointer(llvm::Value *value, std::string varName)
    {
        IRSymTable &symTable = compiler.getSymTable();
        OldTypes &types = compiler.getTypes();
        std::string namespaceName = compiler.getContext().currentNamespace;
        DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Dereferencing pointer");

        STVariable *symTableNode = symTable.getVariable(namespaceName, varName);
        if (!symTableNode)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to get variable node from symtable");
            CONDITION_FAILED;
        }

        llvm::Value *stValue = symTableNode->LLVMValue;
        if (!stValue)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to get variable value from symtable");
            CONDITION_FAILED;
        }

        llvm::StoreInst *stStoreInst = symTableNode->LLVMStoreInst;
        if (!stStoreInst)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to get store instruction from symtable");
            compiler.dumpModule();
            CONDITION_FAILED;
        }

        llvm::LoadInst *stLoadInst = symTableNode->LLVMLoadInst;
        if (stLoadInst)
        {
            DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Value is already loaded");
            return stLoadInst;
        }

        // If the value is a pointer, we need to load the value
        llvm::Instruction *inst = llvm::dyn_cast<llvm::Instruction>(value);
        llvm::Type *instTy = types.parseInstForType(inst);
        std::string loadVarName = varName + ".load.binop";
        llvm::LoadInst *loadInst = compiler.getContext().builder.CreateLoad(instTy, value, loadVarName);
        loadInst->setAlignment(llvm::Align(8));
        llvm::Value *loadValue = llvm::dyn_cast<llvm::Value>(loadInst);
        if (!loadValue)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to load value from pointer");
            CONDITION_FAILED;
        }

        return loadValue;
    }

    llvm::Value *BinaryExpressions::createTempValueForPointer(llvm::Value *value, std::string varName)
    {
        IRSymTable &symTable = compiler.getSymTable();
        OldTypes &types = compiler.getTypes();
        std::string namespaceName = compiler.getContext().currentNamespace;
        DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Creating temporary value for pointer");

        // For these temporary variables, this is how the naming convention will work:
        // (varName) + ".temp"
        std::string tempVarName = varName + ".temp";

        STVariable *symTableNode = symTable.getVariable(namespaceName, varName);
        if (!symTableNode)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to get variable node from symtable");
            CONDITION_FAILED;
        }

        llvm::Value *stValue = symTableNode->LLVMValue;
        if (!stValue)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to get variable value from symtable");
            CONDITION_FAILED;
        }

        llvm::StoreInst *stStoreInst = symTableNode->LLVMStoreInst;
        if (!stStoreInst)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to get store instruction from symtable");
            CONDITION_FAILED;
        }

        llvm::Instruction *inst = llvm::dyn_cast<llvm::Instruction>(value);
        llvm::Type *instTy = types.parseInstForType(inst);

        llvm::AllocaInst *allocaInst = compiler.getContext().builder.CreateAlloca(instTy, nullptr, tempVarName);

        llvm::Value *tempValue = llvm::dyn_cast<llvm::Value>(allocaInst);
        llvm::LoadInst *loadInst = compiler.getContext().builder.CreateLoad(instTy, value, tempVarName + ".loadTest");
        llvm::StoreInst *storeInst = compiler.getContext().builder.CreateStore(loadInst, tempValue);
        storeInst->setAlignment(llvm::Align(8));

        return tempValue;
    }

    llvm::Value *BinaryExpressions::createComparisonExpression(ASTNode *left, ASTNode *right, CryoOperatorType op, llvm::BasicBlock *ifBlock)
    {
        llvm::IRBuilder<> &builder = compiler.getContext().builder;
        DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Creating Comparison Expression");

        llvm::Value *leftValue = compiler.getGenerator().getInitilizerValue(left);
        llvm::Value *rightValue = compiler.getGenerator().getInitilizerValue(right);

        CryoNodeType leftType = left->metaData->type;
        CryoNodeType rightType = right->metaData->type;

        std::cout << "Left Type: " << CryoNodeTypeToString(leftType) << std::endl;
        std::cout << "Right Type: " << CryoNodeTypeToString(rightType) << std::endl;
        std::cout << "Operator: " << CryoOperatorTypeToString(op) << std::endl;

        llvm::Value *leftEval = nullptr;
        llvm::Value *rightEval = nullptr;

        switch (leftType)
        {
        case NODE_VAR_NAME:
        {
            std::string varName = left->data.varName->varName;
            leftEval = compiler.getVariables().getVariable(varName);
            if (!leftEval)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to get left value: " + varName);
                compiler.dumpModule();
                CONDITION_FAILED;
            }
            if (leftEval->getType()->isPointerTy())
            {
                leftEval = dereferenceElPointer(leftEval, varName);
                if (!leftEval)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to dereference left pointer");
                    CONDITION_FAILED;
                }
            }
            break;
        }
        case NODE_LITERAL_EXPR:
        {
            leftEval = compiler.getGenerator().getInitilizerValue(left);
            if (!leftEval)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to get left value");
                CONDITION_FAILED;
            }
            break;
        }
        default:
        {
            DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Unknown left node type");
            CONDITION_FAILED;
        }
        }

        switch (rightType)
        {
        case NODE_VAR_NAME:
        {
            std::string varName = right->data.varName->varName;
            rightEval = compiler.getVariables().getVariable(varName);
            if (!rightEval)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to get right value");
                CONDITION_FAILED;
            }
            if (rightEval->getType()->isPointerTy())
            {
                rightEval = dereferenceElPointer(rightEval, varName);
                if (!rightEval)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to dereference right pointer");
                    CONDITION_FAILED;
                }
            }
            break;
        }
        case NODE_LITERAL_EXPR:
        {
            rightEval = compiler.getGenerator().getInitilizerValue(right);
            if (!rightEval)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to get right value");
                CONDITION_FAILED;
            }
            break;
        }
        case NODE_NULL_LITERAL:
        {
            rightEval = compiler.getGenerator().getInitilizerValue(right);
            if (!rightEval)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to get right value");
                CONDITION_FAILED;
            }
            break;
        }
        default:
        {
            DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Unknown right node type");
            CONDITION_FAILED;
        }
        }

        if (!leftEval || !rightEval)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to generate values for comparison expression");
            CONDITION_FAILED;
        }

        // Set the insert point to the if block
        builder.SetInsertPoint(ifBlock);

        // // Load value if right is a pointer (a variable)
        if (rightValue->getType()->isPointerTy())
        {
            DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Loading right operand");
            rightValue = builder.CreateLoad(rightValue->getType()->getPointerTo(), rightValue, "rightLoad");
        }
        if (leftValue->getType()->isPointerTy() && rightValue->getType()->isPointerTy())
        {
            DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Loading left operand");
            leftValue = builder.CreateLoad(leftValue->getType()->getPointerTo(), leftValue, "leftLoad");
        }

        // Ensure both operands have the same type
        if (leftValue->getType() != rightValue->getType())
        {
            DevDebugger::logMessage("WARNING", __LINE__, "BinExp", "Operand types don't match, attempting to cast");

            // If left is a pointer and right is an integer, we need to load the value from the pointer
            if (leftValue->getType()->isPointerTy() && rightValue->getType()->isIntegerTy())
            {
                DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Casting left operand to match right operand");
                leftValue = compiler.getContext().builder.CreateLoad(rightValue->getType(), leftValue, "leftLoad");
            }
            // If left is an integer and right is a pointer, we need to load the value from the pointer
            else if (leftValue->getType()->isIntegerTy() && rightValue->getType()->isPointerTy())
            {
                DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Casting right operand to match left operand");
                rightValue = compiler.getContext().builder.CreateLoad(leftValue->getType(), rightValue, "rightLoad");
            }
            // If both are integers but of different sizes, cast to the larger size
            else if (leftValue->getType()->isIntegerTy() && rightValue->getType()->isIntegerTy())
            {
                if (leftValue->getType()->getIntegerBitWidth() < rightValue->getType()->getIntegerBitWidth())
                {
                    DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Casting left operand to match right operand");
                    leftValue = compiler.getContext().builder.CreateIntCast(leftValue, rightValue->getType(), true, "leftCast");
                }
                else
                {
                    DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Casting right operand to match left operand");
                    rightValue = compiler.getContext().builder.CreateIntCast(rightValue, leftValue->getType(), true, "rightCast");
                }
            }
            else
            {
                DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Unable to cast operands to matching types");
                CONDITION_FAILED;
            }
        }

        llvm::CmpInst::Predicate predicate;
        switch (op)
        {
        case OPERATOR_EQ:
            predicate = llvm::CmpInst::ICMP_EQ;
            break;
        case OPERATOR_NEQ:
            predicate = llvm::CmpInst::ICMP_NE;
            break;
        case OPERATOR_LT:
            predicate = llvm::CmpInst::ICMP_SLT;
            break;
        case OPERATOR_GT:
            predicate = llvm::CmpInst::ICMP_SGT;
            break;
        case OPERATOR_LTE:
            predicate = llvm::CmpInst::ICMP_SLE;
            break;
        case OPERATOR_GTE:
            predicate = llvm::CmpInst::ICMP_SGE;
            break;
        default:
            DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Unknown comparison operator");
            CONDITION_FAILED;
        }

        // Set the insert point to the if block
        builder.SetInsertPoint(ifBlock);

        llvm::Value *compareResult = builder.CreateICmp(predicate, leftValue, rightValue, "compareResult");
        if (!compareResult)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to create comparison expression");
            CONDITION_FAILED;
        }

        return compareResult;
    }

    llvm::Value *BinaryExpressions::createStringBinOpInitializer(ASTNode *lhs, ASTNode *rhs, CryoOperatorType op, std::string varName)
    {
        DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Creating String Binary Operation");

        llvm::Value *leftValue = compiler.getGenerator().getInitilizerValue(lhs);
        llvm::Value *rightValue = compiler.getGenerator().getInitilizerValue(rhs);

        if (isStringLiteral(rhs))
        {
            // If the right value is a string literal, we need to create a global string
            std::string strContent = rhs->data.literal->value.stringValue;
            llvm::GlobalVariable *globalStr = compiler.getContext().getOrCreateGlobalString(strContent);
            if (!globalStr)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to create global string for concatenation");
                CONDITION_FAILED;
            }

            rightValue = globalStr;
        }

        if (isStringLiteral(lhs))
        {
            // If the left value is a string literal, we need to create a global string
            std::string strContent = lhs->data.literal->value.stringValue;
            llvm::GlobalVariable *globalStr = compiler.getContext().getOrCreateGlobalString(strContent);
            if (!globalStr)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to create global string for concatenation");
                CONDITION_FAILED;
            }

            leftValue = globalStr;
        }

        if (!leftValue || !rightValue)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to get values for string binary operation");
            CONDITION_FAILED;
        }

        llvm::Value *result = nullptr;
        switch (op)
        {
        case OPERATOR_ADD:
            result = createStringConcatenation(leftValue, rightValue, varName);
            break;
        default:
            DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "No Operator Overload for String Binary Operation");
            CONDITION_FAILED;
        }

        if (!result)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to create string binary operation");
            CONDITION_FAILED;
        }

        return result;
    }

    llvm::Value *BinaryExpressions::createStringConcatenation(llvm::Value *leftValue, llvm::Value *rightValue, std::string varName)
    {
        DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Creating String Concatenation");

        // Get the string contents from the left and right values
        llvm::GlobalVariable *leftGlobalStr = llvm::dyn_cast<llvm::GlobalVariable>(leftValue);
        llvm::GlobalVariable *rightGlobalStr = llvm::dyn_cast<llvm::GlobalVariable>(rightValue);

        if (!leftGlobalStr || !rightGlobalStr)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to get global strings for concatenation");
            CONDITION_FAILED;
        }

        llvm::ConstantDataArray *leftStrArray = llvm::dyn_cast<llvm::ConstantDataArray>(leftGlobalStr->getInitializer());
        llvm::ConstantDataArray *rightStrArray = llvm::dyn_cast<llvm::ConstantDataArray>(rightGlobalStr->getInitializer());

        if (!leftStrArray || !rightStrArray)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to get string arrays for concatenation");
            CONDITION_FAILED;
        }

        std::string leftStr = leftStrArray->getAsString().str();
        std::string rightStr = rightStrArray->getAsString().str();

        // Remove Null Terminator from both strings
        if (leftStr.back() == '\0')
        {
            leftStr.pop_back();
        }
        if (rightStr.back() == '\0')
        {
            rightStr.pop_back();
        }

        // Concatenate the strings
        std::string concatenatedStr = leftStr + rightStr;

        // Create a new global string with the concatenated content
        llvm::GlobalVariable *globalStr = compiler.getContext().getOrCreateGlobalString(concatenatedStr);

        if (!globalStr)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to create global string for concatenation");
            CONDITION_FAILED;
        }

        // Get pointer to the first character of the global string
        std::vector<llvm::Value *> indices = {
            llvm::ConstantInt::get(llvm::Type::getInt64Ty(compiler.getContext().context), 0),
            llvm::ConstantInt::get(llvm::Type::getInt64Ty(compiler.getContext().context), 0)};

        llvm::Value *strPtr = compiler.getContext().builder.CreateInBoundsGEP(
            globalStr->getValueType(),
            globalStr,
            indices,
            varName + ".str.ptr");

        if (!strPtr)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to create string pointer for concatenation");
            CONDITION_FAILED;
        }

        return strPtr;
    }

    bool BinaryExpressions::isStringOperation(ASTNode *lhs, ASTNode *rhs)
    {
        DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Checking if binary operation is a string operation");
        DataType *lhsType = getDataTypeFromASTNode(lhs);
        if (!lhsType)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to get data type for left operand");
            CONDITION_FAILED;
        }
        DataType *rhsType = getDataTypeFromASTNode(rhs);
        if (!rhsType)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to get data type for right operand");
            CONDITION_FAILED;
        }

        DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Checking data types");

        logDataType(lhsType);
        logDataType(rhsType);

        bool isLhsString = isStringDataType(lhsType);
        bool isRhsString = isStringDataType(rhsType);

        if (isLhsString && isRhsString)
        {
            DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Binary operation is a string operation");
            return true;
        }

        return false;
    }

    bool BinaryExpressions::isStringBinOp(ASTNode *binOpNode)
    {
        DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Checking if binary operation is a string operation");
        if (binOpNode->metaData->type != NODE_BINARY_EXPR)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Not a binary expression");
            CONDITION_FAILED;
        }

        ASTNode *lhs = binOpNode->data.bin_op->left;
        ASTNode *rhs = binOpNode->data.bin_op->right;

        bool isStringOp = isStringOperation(lhs, rhs);
        if (isStringOp)
        {
            DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Binary operation is a string operation");
            return true;
        }

        return false;
    }

    bool BinaryExpressions::isStringLiteral(ASTNode *node)
    {
        DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Checking if node is a string literal");
        CryoNodeType nodeType = node->metaData->type;
        if (nodeType != NODE_LITERAL_EXPR)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Not a literal expression");
            return false;
        }

        DataType *dataType = node->data.literal->type;
        if (!dataType)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "BinExp", "Failed to get data type for literal expression");
            return false;
        }

        bool isString = isStringDataType(dataType);
        if (isString)
        {
            DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Node is a string literal");
            return true;
        }

        DevDebugger::logMessage("INFO", __LINE__, "BinExp", "Node is not a string literal");
        return false;
    }

} // namespace Cryo
