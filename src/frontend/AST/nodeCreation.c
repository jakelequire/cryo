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
#include "diagnostics/diagnostics.h"

ASTNode *createASTNode(CryoNodeType type, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
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
    node->metaData->line = lexer->currentToken.line;
    node->metaData->column = lexer->currentToken.column;
    node->metaData->lexeme = strndup(lexer->currentToken.start, lexer->currentToken.length);
    node->print = logASTNode;

    node->firstChild = NULL;
    node->nextSibling = NULL;

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
    case NODE_ANNOTATION:
        node->data.annotation = createAnnotationNodeContainer(arena, state);
        break;
    case NODE_TYPE:
        node->data.typeDecl = createTypeDeclContainer(arena, state);
        break;
    case NODE_TYPE_CAST:
        node->data.typeCast = createTypeCastContainer(arena, state);
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
    __STACK_FRAME__
    printf("Adding child node\n");
    if (!parent || !child)
    {
        logMessage(LMI, "ERROR", "AST", "Parent or child node is NULL");
        return;
    }

    // Make sure the child node's nextSibling is NULL
    child->nextSibling = NULL;

    if (!parent->firstChild)
    {
        printf("Parent has no children\n");
        parent->firstChild = child;
    }
    else
    {
        ASTNode *current = parent->firstChild;
        while (current->nextSibling)
        {
            printf("Iterating through siblings\n");
            current = current->nextSibling;
        }
        current->nextSibling = child;
    }
}
// </addChildNode>

// <addStatementToBlock>
void addStatementToBlock(ASTNode *blockNode, ASTNode *statement, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
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
    __STACK_FRAME__
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
    __STACK_FRAME__
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

void buildASTTreeLinks(ASTNode *root)
{
    if (!root)
        return;

    // Handle program nodes specially since they use a statements array
    if (root->metaData->type == NODE_PROGRAM)
    {
        CryoProgram *program = root->data.program;
        ASTNode *lastChild = NULL;

        // Link all statements as children
        for (int i = 0; i < program->statementCount; i++)
        {
            ASTNode *statement = program->statements[i];
            if (!root->firstChild)
            {
                root->firstChild = statement;
            }
            if (lastChild)
            {
                lastChild->nextSibling = statement;
            }
            lastChild = statement;

            // Recursively build links for the statement
            buildASTTreeLinks(statement);
        }
    }
    // Handle block nodes
    else if (root->metaData->type == NODE_BLOCK)
    {
        CryoBlockNode *block = root->data.block;
        ASTNode *lastChild = NULL;

        for (int i = 0; i < block->statementCount; i++)
        {
            ASTNode *statement = block->statements[i];
            if (!root->firstChild)
            {
                root->firstChild = statement;
            }
            if (lastChild)
            {
                lastChild->nextSibling = statement;
            }
            lastChild = statement;

            buildASTTreeLinks(statement);
        }
    }
    // Handle function nodes
    else if (root->metaData->type == NODE_FUNCTION_DECLARATION)
    {
        FunctionDeclNode *func = root->data.functionDecl;

        // Link parameters as children
        ASTNode *lastChild = NULL;
        for (int i = 0; i < func->paramCount; i++)
        {
            ASTNode *param = func->params[i];
            if (!root->firstChild)
            {
                root->firstChild = param;
            }
            if (lastChild)
            {
                lastChild->nextSibling = param;
            }
            lastChild = param;

            buildASTTreeLinks(param);
        }

        // Link function body
        if (func->body)
        {
            if (lastChild)
            {
                lastChild->nextSibling = func->body;
            }
            else
            {
                root->firstChild = func->body;
            }
            buildASTTreeLinks(func->body);
        }
    }
    // Handle other node types appropriately
    else
    {
        // Binary expressions
        if (root->metaData->type == NODE_BINARY_EXPR)
        {
            CryoBinaryOpNode *binOp = root->data.bin_op;
            if (binOp->left)
            {
                root->firstChild = binOp->left;
                buildASTTreeLinks(binOp->left);
            }
            if (binOp->right)
            {
                if (binOp->left)
                {
                    binOp->left->nextSibling = binOp->right;
                }
                else
                {
                    root->firstChild = binOp->right;
                }
                buildASTTreeLinks(binOp->right);
            }
        }
        // Add similar handling for other node types...
    }
}

void addChildToNode(ASTNode *parent, ASTNode *child)
{
    if (!parent || !child)
        return;

    if (!parent->firstChild)
    {
        parent->firstChild = child;
    }
    else
    {
        ASTNode *sibling = parent->firstChild;
        while (sibling->nextSibling)
        {
            sibling = sibling->nextSibling;
        }
        sibling->nextSibling = child;
    }
}

ASTNode *getFirstChild(ASTNode *node)
{
    return node ? node->firstChild : NULL;
}

ASTNode *getNextSibling(ASTNode *node)
{
    return node ? node->nextSibling : NULL;
}

void traverseAST(ASTNode *node, void (*visitor)(ASTNode *))
{
    if (!node)
        return;

    // Visit the current node
    visitor(node);

    // Visit all children
    ASTNode *child = node->firstChild;
    while (child)
    {
        traverseAST(child, visitor);
        child = child->nextSibling;
    }
}
