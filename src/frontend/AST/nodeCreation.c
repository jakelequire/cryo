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
#include "symbolTable/cInterfaceTable.h"
#include "frontend/AST.h"

ASTNode *createASTNode(CryoNodeType type, Arena *arena, CompilerState *state, TypeTable *typeTable, Lexer *lexer)
{
    ASTNode *node = (ASTNode *)ARENA_ALLOC(arena, sizeof(ASTNode));
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to allocate memory for AST node");
        return NULL;
    }

    node->metaData = createMetaDataContainer(arena, state);
    if (!node->metaData)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to allocate memory for AST node metadata");
        return NULL;
    }

    const char *moduleName = GetNamespace(state->globalTable);
    if (moduleName)
    {
        node->metaData->moduleName = (char *)moduleName;
    }
    node->metaData->type = type;
    node->metaData->position = getPosition(state->lexer);
    node->metaData->line = lexer->line;
    node->metaData->column = lexer->column;

    switch (type)
    {
    case NODE_NAMESPACE:
        node->data.cryoNamespace = createCryoNamespaceNodeContainer(arena, state);
        break;
    case NODE_PROGRAM:
        node->data.program = createCryoProgramContainer(arena, state);
        break;
    case NODE_IMPORT_STATEMENT:
        node->data.import = createCryoImportNodeContainer(arena, state);
        break;
    case NODE_BLOCK:
        node->data.block = createCryoBlockNodeContainer(arena, state);
        break;
    case NODE_FUNCTION_BLOCK:
        node->data.functionBlock = createCryoFunctionBlockContainer(arena, state);
        break;
    case NODE_RETURN_STATEMENT:
        node->data.returnStatement = createReturnNodeContainer(arena, state);
        break;
    case NODE_LITERAL_EXPR:
        node->data.literal = createLiteralNodeContainer(arena, state);
        break;
    case NODE_VAR_DECLARATION:
        node->data.varDecl = createVariableNodeContainer(arena, state);
        break;
    case NODE_VAR_NAME:
        node->data.varName = createVariableNameNodeContainer("", arena, state);
        break;
    case NODE_EXPRESSION:
        node->data.expression = createExpressionNodeContainer(arena, state);
        break;
    case NODE_FUNCTION_DECLARATION:
        node->data.functionDecl = createFunctionNodeContainer(arena, state);
        break;
    case NODE_EXTERN_FUNCTION:
        node->data.externNode = createExternNodeContainer(NODE_EXTERN_FUNCTION, arena, state);
        break;
    case NODE_FUNCTION_CALL:
        node->data.functionCall = createFunctionCallNodeContainer(arena, state);
        break;
    case NODE_IF_STATEMENT:
        node->data.ifStatement = createIfStatementContainer(arena, state);
        break;
    case NODE_FOR_STATEMENT:
        node->data.forStatement = createForStatementNodeContainer(arena, state);
        break;
    case NODE_WHILE_STATEMENT:
        node->data.whileStatement = createWhileStatementNodeContainer(arena, state);
        break;
    case NODE_BINARY_EXPR:
        node->data.bin_op = createBinaryOpNodeContainer(arena, state);
        break;
    case NODE_UNARY_EXPR:
        node->data.unary_op = createUnaryOpNodeContainer(arena, state);
        break;
    case NODE_PARAM_LIST:
        node->data.paramList = createParamNodeContainer(arena, state);
        break;
    case NODE_ARG_LIST:
        node->data.argList = createArgNodeContainer(arena, state);
        break;
    case NODE_ARRAY_LITERAL:
        node->data.array = createArrayNodeContainer(arena, state);
        break;
    case NODE_INDEX_EXPR:
        node->data.indexExpr = createIndexExprNodeContainer(arena, state);
        break;
    case NODE_VAR_REASSIGN:
        node->data.varReassignment = createVariableReassignmentNodeContainer(arena, state);
        break;
    case NODE_PARAM:
        node->data.param = createParameterNodeContainer(arena, state);
        break;
    case NODE_PROPERTY:
        node->data.property = createPropertyNodeContainer(arena, state);
        break;
    case NODE_STRUCT_DECLARATION:
        node->data.structNode = createStructNodeContainer(arena, state);
        break;
    case NODE_SCOPED_FUNCTION_CALL:
        node->data.scopedFunctionCall = createScopedFunctionCallNode(arena, state);
        break;
    case NODE_STRUCT_CONSTRUCTOR:
        node->data.structConstructor = createStructConstructorNodeContainer(arena, state);
        break;
    case NODE_PROPERTY_ACCESS:
        node->data.propertyAccess = createPropertyAccessNodeContainer(arena, state);
        break;
    case NODE_THIS:
        node->data.thisNode = createThisNodeContainer(arena, state);
        break;
    case NODE_PROPERTY_REASSIGN:
        node->data.propertyReassignment = createPropertyReassignmentNodeContainer(arena, state);
        break;
    case NODE_METHOD:
        node->data.method = createMethodNodeContainer(arena, state);
        break;
    case NODE_METHOD_CALL:
        node->data.methodCall = createMethodCallNodeContainer(arena, state);
        break;
    case NODE_GENERIC_DECL:
        node->data.genericDecl = createGenericDeclNodeContainer(arena, state);
        break;
    case NODE_GENERIC_INST:
        node->data.genericInst = createGenericInstNodeContainer(arena, state);
        break;
    case NODE_CLASS:
        node->data.classNode = createClassNodeContainer(arena, state);
        break;
    case NODE_CLASS_CONSTRUCTOR:
        node->data.classConstructor = createClassConstructorNodeContainer(arena, state);
        break;
    case NODE_OBJECT_INST:
        node->data.objectNode = createObjectNodeContainer(arena, state);
        break;
    case NODE_NULL_LITERAL:
        node->data.nullNode = createNullNodeContainer(arena, state);
        break;
    case NODE_TYPEOF:
        node->data.typeofNode = createTypeofNodeContainer(arena, state);
        break;
    case NODE_USING:
        node->data.usingNode = createUsingNodeContainer(arena, state);
        break;
    case NODE_MODULE:
        node->data.moduleNode = createModuleNodeContainer(arena, state);
        break;
    default:
        logMessage(LMI, "ERROR", "AST", "Unknown Node Type: %s", CryoNodeTypeToString(type));
        return NULL;
    }

    logMessage(LMI, "INFO", "AST", "Created AST node of type: %s", CryoNodeTypeToString(type));
    return node;
}

// <addChildNode>
void addChildNode(ASTNode *parent, ASTNode *child, Arena *arena, CompilerState *state)
{
    if (!parent || !child)
    {
        logMessage(LMI, "ERROR", "AST", "Parent or child node is NULL");
        return;
    }

    if (!parent->metaData->firstChild)
    {
        parent->metaData->firstChild = child;
    }
    else
    {
        ASTNode *current = parent->metaData->firstChild;
        while (current->metaData->nextSibling)
        {
            current = current->metaData->nextSibling;
        }
        current->metaData->nextSibling = child;
    }
}
// </addChildNode>

// <addStatementToBlock>
void addStatementToBlock(ASTNode *blockNode, ASTNode *statement, Arena *arena, CompilerState *state, Lexer *lexer)
{
    if (blockNode->metaData->type != NODE_BLOCK && blockNode->metaData->type != NODE_FUNCTION_BLOCK)
    {
        logMessage(LMI, "ERROR", "AST", "Invalid block node");
        return;
    }

    CryoBlockNode *block = blockNode->data.block;

    // Debugging initial state
    if (block->statementCount >= block->statementCapacity)
    {
        block->statementCapacity *= 2;
        block->statements = (ASTNode **)realloc(block->statements, sizeof(ASTNode *) * block->statementCapacity);
        if (!block->statements)
        {
            logMessage(LMI, "ERROR", "AST", "Failed to reallocate memory for block statements");
            return;
        }
        logMessage(LMI, "INFO", "AST", "Block statement memory reallocated");
    }
    else
    {
        logMessage(LMI, "INFO", "AST", "Block statement memory is sufficient");
    }

    // THIS IS THROWING :)
    block->statements[block->statementCount++] = statement;
    // Debugging final state
    logMessage(LMI, "INFO", "AST", "Final state: stmtCount = %d, stmtCapacity = %d", block->statementCount);
}
// </addStatementToBlock>

void addStatementToFunctionBlock(ASTNode *functionBlock, ASTNode *statement, Arena *arena, CompilerState *state, Lexer *lexer)
{
    if (!functionBlock || !statement || !functionBlock->metaData || functionBlock->metaData->type != NODE_FUNCTION_BLOCK)
    {
        logMessage(LMI, "ERROR", "AST", "Invalid function block node");
        return;
    }

    CryoFunctionBlock *block = functionBlock->data.functionBlock;
    if (!block)
    {
        logMessage(LMI, "ERROR", "AST", "Function block data is NULL");
        return;
    }

    // Initialize statements array if it doesn't exist
    if (!block->statements)
    {
        block->statementCapacity = 64; // Start with a reasonable capacity
        block->statementCount = 0;
        block->statements = (ASTNode **)ARENA_ALLOC(arena, sizeof(ASTNode *) * block->statementCapacity);
        if (!block->statements)
        {
            logMessage(LMI, "ERROR", "AST", "Failed to allocate memory for function block statements");
            return;
        }
    }
    // Resize if necessary
    else if (block->statementCount >= block->statementCapacity)
    {
        size_t newCapacity = block->statementCapacity * 2;
        ASTNode **newStatements = (ASTNode **)realloc(block->statements, sizeof(ASTNode *) * newCapacity);
        if (!newStatements)
        {
            logMessage(LMI, "ERROR", "AST", "Failed to reallocate memory for function block statements");
            return;
        }
        block->statements = newStatements;
        block->statementCapacity = newCapacity;
    }

    // Add the new statement
    block->statements[block->statementCount++] = statement;

    // Debug output
    logMessage(LMI, "INFO", "AST", "Debug: block=%p, statementCount=%d, statementCapacity=%d",
               (void *)block, block->statementCount, block->statementCapacity);

    addChildNode(functionBlock, statement, arena, state);
}

// <addFunctionToProgram>
void addFunctionToProgram(ASTNode *program, ASTNode *function, Arena *arena, CompilerState *state, Lexer *lexer)
{
    if (!program || !function)
    {
        logMessage(LMI, "ERROR", "AST", "Program or function node is NULL");
        return;
    }

    if (program->metaData->type != NODE_PROGRAM)
    {
        logMessage(LMI, "ERROR", "AST", "Invalid program node");
        return;
    }

    CryoProgram *prog = program->data.program;

    if (prog->statementCount >= prog->statementCapacity)
    {
        prog->statementCapacity *= 2;
        prog->statements = (ASTNode **)realloc(prog->statements, sizeof(ASTNode *) * prog->statementCapacity);
        if (!prog->statements)
        {
            logMessage(LMI, "ERROR", "AST", "Failed to reallocate memory for program statements");
            return;
        }
    }

    prog->statements[prog->statementCount++] = function;
    addChildNode(program, function, arena, state);
}
// </addFunctionToProgram>
