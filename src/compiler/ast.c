
#include "compiler/ast.h"




// Define the global program node
ASTNode* programNode = NULL;


// <printAST>
void printAST(ASTNode* node, int indent) {
    if (!node) return;

    for (int i = 0; i < indent; i++) {
        printf(" ");
    }

    printf("Node type: %d\n", node->type);

    switch (node->type) {
        case NODE_PROGRAM:
            printf("Program with %d statements (capacity: %d):\n", node->data.program.stmtCount, node->data.program.stmtCapacity);
            for (int i = 0; i < node->data.program.stmtCount; i++) {
                printAST(node->data.program.statements[i], indent + 2);
            }
            break;

        case NODE_BLOCK:
            printf("Block with %d statements:\n", node->data.block.stmtCount);
            for (int i = 0; i < node->data.block.stmtCount; i++) {
                printAST(node->data.block.statements[i], indent + 1);
            }
            break;

        case NODE_FUNCTION_DECLARATION:
            printf("Function Declaration: %s\n", node->data.functionDecl.name);
            printAST(node->data.functionDecl.body, indent + 2);
            break;

        case NODE_FUNCTION_CALL:
            printf("Function Call: %s\n", node->data.functionCall.name);
            for (int i = 0; i < node->data.functionCall.argCount; i++) {
                printAST(node->data.functionCall.args[i], indent + 2);
            }
            break;

        case NODE_FUNCTION_BLOCK:
            printf("Function Block with %d statements:\n", node->data.functionBlock.block->data.block.stmtCount);
            for (int i = 0; i < node->data.functionBlock.block->data.block.stmtCount; i++) {
                printAST(node->data.functionBlock.block->data.block.statements[i], indent + 2);
            }
            break;
        case NODE_VAR_DECLARATION:
            printf("Variable Declaration: %s\n", node->data.varDecl.name);
            printAST(node->data.varDecl.initializer, indent + 2);
            break;
        case NODE_RETURN_STATEMENT:
            printf("Return Statement:\n");
            if (node->data.returnStmt.returnValue) {
                printAST(node->data.returnStmt.returnValue, indent + 2);
            } else {
                printf("%*svoid\n", indent + 4, "");
            }
            break;

        case NODE_LITERAL_EXPR:
            switch (node->data.literalExpression.dataType) {
                case DATA_TYPE_INT:
                    printf("Literal (int): %d\n", node->data.literalExpression.intValue);
                    break;
                case DATA_TYPE_STRING:
                    printf("Literal (string): %s\n", node->data.literalExpression.stringValue);
                    break;
                case DATA_TYPE_BOOLEAN:
                    printf("Literal (boolean): %s\n", node->data.literalExpression.booleanValue ? "true" : "false");
                    break;
                default:
                    printf("Literal (unknown type)\n");
                    break;
            }
            break;

        case NODE_UNARY_EXPR:
            printf("Unary Expression: %s\n", tokenTypeToString(node->data.unary_op.op));
            printAST(node->data.unary_op.operand, indent + 1);
            break;

        case NODE_BINARY_EXPR:
            printf("Binary Expression: %s\n", operatorToString(node->data.bin_op.op));
            printAST(node->data.bin_op.left, indent + 1);
            printAST(node->data.bin_op.right, indent + 1);
            break;

        case NODE_BOOLEAN_LITERAL:
            printf("Boolean Literal: %s\n", node->data.boolean.value ? "true" : "false");
            break;

        case NODE_STRING_LITERAL:
            printf("String Literal: '%s'\n", node->data.str.str);
            break;

        case NODE_VAR_NAME:
            printf("Variable Name: %s\n", node->data.varName.varName);
            break;

        case NODE_IF_STATEMENT:
            printf("If Statement:\n");
            printAST(node->data.ifStmt.condition, indent + 2);
            printAST(node->data.ifStmt.thenBranch, indent + 2);
            if (node->data.ifStmt.elseBranch) {
                printAST(node->data.ifStmt.elseBranch, indent + 2);
            }
            break;

        case NODE_EXPRESSION_STATEMENT:
            printf("Expression Statement:\n");
            printAST(node->data.expr.expr, indent + 2);
            break;
        
        case NODE_EXPRESSION:
            printf("Expression:\n");
            printAST(node->data.expr.expr, indent + 2);
            break;
        
        case NODE_STATEMENT:
            printf("Statement:\n");
            printAST(node->data.stmt.stmt, indent + 2);
            break;

        case NODE_PARAM_LIST:
            printf("Parameter List:\n");
            for (int i = 0; i < node->data.paramList.paramCount; i++) {
                printAST(node->data.paramList.params[i], indent + 2);
            }
            break;
        case NODE_WHILE_STATEMENT:
            printf("While Statement:\n");
            printAST(node->data.whileStmt.condition, indent + 2);
            printAST(node->data.whileStmt.body, indent + 2);
            break;

        case NODE_FOR_STATEMENT:
            printf("For Statement:\n");
            printAST(node->data.forStmt.initializer, indent + 2);
            printAST(node->data.forStmt.condition, indent + 2);
            printAST(node->data.forStmt.increment, indent + 2);
            printAST(node->data.forStmt.body, indent + 2);
            break;

        case NODE_TYPE:
            printf("Data Type: <UNIMPLEMENTED>\n");
            break;

        default:
            printf("[ERROR] Unknown node type: %d\n", node->data);
            fprintf(stderr, "[ERROR] Unknown node type: %d\n", node->type);
            break;
    }
}
// </printAST>


// <freeAST>
void freeAST(ASTNode* node) {
    if (!node) {
        printf("[AST] No node to free\n");
        return;
    }

    switch (node->type) {
        case NODE_PROGRAM:
            if (node->data.program.statements) {
                for (int i = 0; i < node->data.program.stmtCount; ++i) {
                    printf("[AST] Freeing program statement: %d\n", node->data.program.statements[i]->type);
                    freeAST(node->data.program.statements[i]);
                }
                printf("[AST] Freeing program statements\n");
                free(node->data.program.statements);
            }
            break;

        case NODE_FUNCTION_DECLARATION:
            printf("[AST] Freeing Function Declaration Node\n");
            printf("[AST] Freeing node: %d\n", node->type);
            free(node->data.functionDecl.name);
            freeAST(node->data.functionDecl.body);
            break;

        case NODE_BLOCK:
            printf("[AST] Freeing Block Node\n");
            if (node->data.block.statements) {
                for (int i = 0; i < node->data.block.stmtCount; ++i) {
                    printf("[AST] Freeing block statement: %d\n", node->data.block.statements[i]->type);
                    freeAST(node->data.block.statements[i]);
                }
                printf("[AST] Freeing block statements\n");
                free(node->data.block.statements);
            }
            printf("[AST] Freeing node: %d\n", node->type);
            break;

        case NODE_FUNCTION_BLOCK:
            printf("[AST] Freeing Function Block Node\n");
            freeAST(node->data.functionBlock.function);
            freeAST(node->data.functionBlock.block);
            break;

        case NODE_RETURN_STATEMENT:
            printf("[AST] Freeing Return Statement Node\n");
            freeAST(node->data.returnStmt.returnValue);
            break;

        case NODE_BINARY_EXPR:
            printf("[AST] Freeing Binary Expression Node\n");
            printf("[AST] Freeing node: %d\n", node->type);
            freeAST(node->data.bin_op.left);
            freeAST(node->data.bin_op.right);
            break;

        case NODE_LITERAL_EXPR:
            printf("[AST] Freeing Literal Expression Node\n");
            printf("[AST] Freeing node: %d\n", node->type);
            break;

        case NODE_VAR_DECLARATION:
            printf("[AST] Freeing Variable Declaration Node\n");
            printf("[AST] Freeing node: %d\n", node->type);
            free(node->data.varDecl.name);
            freeAST(node->data.varDecl.initializer);
            break;
            
        case NODE_STRING_LITERAL:
            printf("[AST] Freeing String Literal Node\n");
            free(node->data.str.str);
            break;

        case NODE_BOOLEAN_LITERAL:
            printf("[AST] Freeing Boolean Literal Node\n");
            break;
            
        case NODE_VAR_NAME:
            printf("[AST] Freeing Variable Name Node\n");
            free(node->data.varName.varName);
            break;

        case NODE_UNARY_EXPR:
            printf("[AST] Freeing Unary Expression Node\n");
            freeAST(node->data.unary_op.operand);
            break;

        case NODE_EXPRESSION_STATEMENT:
            printf("[AST] Freeing Expression Statement Node\n");
            freeAST(node->data.expr.expr);
            break;

        case NODE_FUNCTION_CALL:
            printf("[AST] Freeing Function Call Node\n");
            free(node->data.functionCall.name);
            for (int i = 0; i < node->data.functionCall.argCount; i++) {
                freeAST(node->data.functionCall.args[i]);
            }
            free(node->data.functionCall.args);
            break;

        case NODE_IF_STATEMENT:
            printf("[AST] Freeing If Statement Node\n");
            freeAST(node->data.ifStmt.condition);
            freeAST(node->data.ifStmt.thenBranch);
            freeAST(node->data.ifStmt.elseBranch);
            break;

        case NODE_WHILE_STATEMENT:
            printf("[AST] Freeing While Statement Node\n");
            freeAST(node->data.whileStmt.condition);
            freeAST(node->data.whileStmt.body);
            break;

        case NODE_FOR_STATEMENT:
            printf("[AST] Freeing For Statement Node\n");
            freeAST(node->data.forStmt.initializer);
            freeAST(node->data.forStmt.condition);
            freeAST(node->data.forStmt.increment);
            freeAST(node->data.forStmt.body);
            break;

        default:
            printf("[AST] Unknown Node Type. <DEFAULTED>\n");
            exit(0);
            break;
    }
    free(node);
}
// </freeAST>


// <createASTNode>
ASTNode* createASTNode(CryoNodeType type) {
    printf("[AST_DEBUG] Creating node: %d\n", type);
    ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "[AST] [ERROR] Failed to allocate memory for AST node\n");
        return NULL;
    }
    node->type = type;
    // node->line = 0;
    node->firstChild = NULL;
    node->nextSibling = NULL;

    printf("[AST_DEBUG] Initialized node: %d\n", type);

    // Initialize based on node type
    switch (type) {
        case NODE_PROGRAM:
            node->data.program.statements = NULL;
            node->data.program.stmtCount = 0;
            node->data.program.stmtCapacity = 0;
            break;
        case NODE_FUNCTION_DECLARATION:
            node->data.functionDecl.name = NULL;
            node->data.functionDecl.params = NULL;
            node->data.functionDecl.body = NULL;
            node->data.functionDecl.returnType = DATA_TYPE_VOID;
            break;
        case NODE_FUNCTION_BLOCK:
            node->data.functionBlock.function = NULL;
            node->data.functionBlock.block = createASTNode(NODE_BLOCK); // Ensure block is initialized
            node->data.functionBlock.block->data.block.statements = (ASTNode**)malloc(sizeof(ASTNode*) * INITIAL_CAPACITY);
            node->data.functionBlock.block->data.block.stmtCount = 0;
            node->data.functionBlock.block->data.block.stmtCapacity = INITIAL_CAPACITY;
            break;
        case NODE_VAR_DECLARATION:
            node->data.varDecl.name = NULL;
            node->data.varDecl.initializer = NULL;
            break;
        case NODE_VAR_NAME:
            node->data.varName.varName = NULL;
            break;
        case NODE_RETURN_STATEMENT:
            node->data.returnStmt.returnValue = NULL;
            break;
        case NODE_IF_STATEMENT:
            node->data.ifStmt.condition = NULL;
            node->data.ifStmt.thenBranch = NULL;
            node->data.ifStmt.elseBranch = NULL;
            break;
        case NODE_BINARY_EXPR:
            node->data.bin_op.left = NULL;
            node->data.bin_op.right = NULL;
            node->data.bin_op.op = OPERATOR_NA;
            break;
        case NODE_LITERAL_EXPR:
            node->data.literalExpression.dataType = DATA_TYPE_UNKNOWN;
            break;
        case NODE_FUNCTION_CALL:
            node->data.functionCall.name = NULL;
            node->data.functionCall.args = NULL;
            node->data.functionCall.argCount = 0;
            break;
        case NODE_BLOCK:
            node->data.block.statements = NULL;
            node->data.block.stmtCount = 0;
            node->data.block.stmtCapacity = 0;
            break;
        default:
            fprintf(stderr, "[AST] [ERROR] Unknown node type during creation: %d\n", type);
            break;
    }

    printf("[DEBUG] Created node of type: %d\n", type);

    return node;
}
// </createASTNode>


// <addChildNode>
void addChildNode(ASTNode* parent, ASTNode* child) {
    if (parent->type != NODE_BLOCK && parent->type != NODE_FUNCTION_BLOCK) {
        fprintf(stderr, "[AST_ERROR] Invalid parent node type for adding child node\n");
        return;
    }

    if (parent->data.block.stmtCount >= parent->data.block.stmtCapacity) {
        int newCapacity = parent->data.block.stmtCapacity == 0 ? 2 : parent->data.block.stmtCapacity * 2;
        parent->data.block.statements = realloc(parent->data.block.statements, newCapacity * sizeof(ASTNode*));
        if (!parent->data.block.statements) {
            fprintf(stderr, "[AST_ERROR] Failed to reallocate memory for block statements\n");
            return;
        }
        parent->data.block.stmtCapacity = newCapacity;
    }

    parent->data.block.statements[parent->data.block.stmtCount++] = child;
}
// </addChildNode>


// <createLiteralExpr>
ASTNode* createLiteralExpr(int value) {
    ASTNode* node = createASTNode(NODE_LITERAL_EXPR);
    if (!node) {
        fprintf(stderr, "[AST] [ERROR] Failed to allocate memory for literal expression node\n");
        return NULL;
    }
    node->data.literalExpression.dataType = DATA_TYPE_INT;
    node->data.literalExpression.intValue = value;
    printf("[Parser] Created IntLiteral Node with value: %d and type: %d\n", value, node->data.literalExpression.dataType);
    return node;
}
// </createLiteralExpr>


// <createStringLiteralExpr>
ASTNode* createStringLiteralExpr(const char* str) {
    ASTNode* node = createASTNode(NODE_LITERAL_EXPR);
    if (!node) {
        fprintf(stderr, "[AST] [ERROR] Failed to allocate memory for string literal expression node\n");
        return NULL;
    }
    node->data.literalExpression.dataType = DATA_TYPE_STRING;
    node->data.literalExpression.stringValue = strdup(str);
    return node;
}
// </createStringLiteralExpr


// <createVariableExpr>
ASTNode* createVariableExpr(const char* name) {
    ASTNode* node = createASTNode(NODE_VAR_NAME);
    if (!node) {
        fprintf(stderr, "[AST] [ERROR] Failed to allocate memory for variable expression node\n");
        return NULL;
    }
    node->data.varName.varName = strdup(name);
    return node;
}
// </createVariableExpr>


// <createBinaryExpr>
ASTNode* createBinaryExpr(ASTNode* left, ASTNode* right, CryoOperatorType op) {
    ASTNode* node = createASTNode(NODE_BINARY_EXPR);
    if (!node) {
        fprintf(stderr, "[AST] [ERROR] Failed to allocate memory for binary expression node\n");
        return NULL;
    }
    node->data.bin_op.left = left;
    node->data.bin_op.right = right;
    node->data.bin_op.op = op;
    node->data.bin_op.operatorText = strdup(operatorToString(op));
    printf("Created Binary Expression Node: left=%p, right=%p, operator=%d\n", left, right, op);
    return node;
}
// </createBinaryExpr>



// <createUnaryExpr>
ASTNode* createUnaryExpr(CryoTokenType operator, ASTNode* operand) {
    ASTNode* node = createASTNode(NODE_UNARY_EXPR);
    if (!node) {
        return NULL;
    }
    node->data.unary_op.op = operator;
    node->data.unary_op.operand = operand;
    return node;
}
// </createUnaryExpr>


// <createFunctionNode>
ASTNode* createFunctionNode(const char* function_name, ASTNode* params, ASTNode* function_body, CryoDataType returnType) {
    ASTNode* node = createASTNode(NODE_FUNCTION_DECLARATION);
    if (!node) {
        return NULL;
    }
    node->data.functionDecl.name = strdup(function_name);
    node->data.functionDecl.params = params;
    node->data.functionDecl.paramCount = 0;
    node->data.functionDecl.body = function_body;
    node->data.functionDecl.returnType = returnType;
    node->data.functionDecl.visibility = VISIBILITY_PUBLIC;
    printf("\n[AST] Function Node Created: %s with return type: %d\n", function_name, returnType);
    return node;
}

// </createFunctionNode>


// <createReturnNode>
ASTNode* createReturnNode(ASTNode* returnValue) {
    ASTNode* node = createASTNode(NODE_RETURN_STATEMENT);
    node->data.returnStmt.returnValue = returnValue;
    printf("[AST] Return Node Created with return value of type: %d\n", returnValue ? returnValue->type : -1);
    return node;
}
// </createReturnNode>


// <createReturnStatement>
ASTNode* createReturnStatement(ASTNode* return_val) {
    ASTNode* node = createASTNode(NODE_RETURN_STATEMENT);
    if (!node) {
        return NULL;
    }
    node->data.returnStmt.returnValue = return_val;
    return node;
}
// </createReturnStatement>


// <createBlock>
ASTNode* createBlock() {
    ASTNode* node = createASTNode(NODE_BLOCK);
    if (!node) {
        return NULL;
    }
    node->data.block.statements = NULL;
    node->data.block.stmtCount = 0;
    node->data.block.stmtCapacity = 0;
    return node;
}
// </createBlock>


// <createIfStatement>
ASTNode* createIfStatement(ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch) {
    ASTNode* node = createASTNode(NODE_IF_STATEMENT);
    if (!node) {
        return NULL;
    }
    node->data.ifStmt.condition = condition;
    node->data.ifStmt.thenBranch = then_branch;
    node->data.ifStmt.elseBranch = else_branch;
    return node;
}
// </createIfStatement>


// <createWhileStatement>
ASTNode* createWhileStatement(ASTNode* condition, ASTNode* body) {
    ASTNode* node = createASTNode(NODE_WHILE_STATEMENT);
    if (!node) {
        return NULL;
    }
    node->data.whileStmt.condition = condition;
    node->data.whileStmt.body = body;
    return node;
}
// </createWhileStatement>


// <createForStatement>
ASTNode* createForStatement(ASTNode* initializer, ASTNode* condition, ASTNode* increment, ASTNode* body) {
    ASTNode* node = createASTNode(NODE_FOR_STATEMENT);
    if (!node) {
        return NULL;
    }
    node->data.forStmt.initializer = initializer;
    node->data.forStmt.condition = condition;
    node->data.forStmt.increment = increment;
    node->data.forStmt.body = body;
    return node;
}
// </createForStatement>


// <createVarDeclarationNode>
ASTNode* createVarDeclarationNode(const char* var_name, CryoDataType dataType, ASTNode* initializer, int line) {
    ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));  // Use calloc to initialize memory
    if (!node) {
        fprintf(stderr, "[AST] [ERROR] Failed to allocate memory for variable declaration node\n");
        return NULL;
    }

    node->type = NODE_VAR_DECLARATION;
    node->line = line;
    node->firstChild = NULL;
    node->nextSibling = NULL;

    node->data.varDecl.name = strdup(var_name);
    node->data.varDecl.dataType = dataType;  // Set the data type
    node->data.varDecl.initializer = initializer;

    return node;
}
// </createVarDeclarationNode>


// <createStringExpr>
ASTNode* createStringExpr(const char* str) {
    ASTNode* node = createASTNode(NODE_STRING_LITERAL);
    if (!node) {
        return NULL;
    }
    node->data.str.str = strdup(str);
    return node;
}
// </createStringExpr>


// <createBooleanLiteralExpr>
ASTNode* createBooleanLiteralExpr(int value) {
    printf("[AST] Creating boolean literal node!");
    ASTNode* node = createASTNode(NODE_BOOLEAN_LITERAL);
    if (!node) {
        fprintf(stderr, "[AST] [ERROR] Failed to allocate memory for boolean literal node\n");
        return NULL;
    }
    node->data.literalExpression.dataType = DATA_TYPE_BOOLEAN;
    node->data.literalExpression.booleanValue = value;
    return node;
}
// </createBooleanLiteralExpr>


// <createExpressionStatement>
ASTNode* createExpressionStatement(ASTNode* expression) {
    ASTNode* node = createASTNode(NODE_EXPRESSION_STATEMENT);
    if (!node) {
        return NULL;
    }
    node->data.expr.expr = expression;
    return node;
}
// </createExpressionStatement>


// <createFunctionCallNode>
ASTNode* createFunctionCallNode(const char* name, ASTNode** args, int argCount) {
    ASTNode* node = createASTNode(NODE_FUNCTION_CALL);
    if (!node) {
        return NULL;
    }
    node->data.functionCall.name = strdup(name);
    node->data.functionCall.args = args;
    node->data.functionCall.argCount = argCount;
    return node;
}
// </createFunctionCallNode>


// <addStatementToBlock>
void addStatementToBlock(ASTNode* blockNode, ASTNode* statement) {
    if (blockNode->type != NODE_BLOCK) {
        printf("[AST] Error: addStatementToBlock called on non-block node\n");
        return;
    }
    if (blockNode->data.block.stmtCount >= blockNode->data.block.stmtCapacity) {
        blockNode->data.block.stmtCapacity *= 2;
        blockNode->data.block.statements = (ASTNode**)realloc(blockNode->data.block.statements, sizeof(ASTNode*) * blockNode->data.block.stmtCapacity);
    }
    blockNode->data.block.statements[blockNode->data.block.stmtCount++] = statement;
    printf("[AST] Statement added to block. Total statements: %d\n", blockNode->data.block.stmtCount);
}
// </addStatementToBlock>



// <addFunctionToProgram>
void addFunctionToProgram(ASTNode* program, ASTNode* function) {
    if (program->type != NODE_PROGRAM) {
        printf("[AST] Error: addFunctionToProgram called on non-program node\n");
        return;
    }
    printf("[AST_DEBUG] Before adding function: stmtCount = %d, stmtCapacity = %d\n", program->data.program.stmtCount, program->data.program.stmtCapacity);
    if (program->data.program.stmtCount >= program->data.program.stmtCapacity) {
        program->data.program.stmtCapacity *= 2;  // Double the capacity
        printf("[AST_DEBUG] Increasing stmtCapacity to: %d\n", program->data.program.stmtCapacity);
        program->data.program.statements = (ASTNode**)realloc(program->data.program.statements, program->data.program.stmtCapacity * sizeof(ASTNode*));
        if (!program->data.program.statements) {
            fprintf(stderr, "[AST] [ERROR] Failed to reallocate memory for program statements\n");
            return;
        }
    }
    program->data.program.statements[program->data.program.stmtCount++] = function;
    printf("[AST_DEBUG] After adding function: stmtCount = %d, stmtCapacity = %d\n", program->data.program.stmtCount, program->data.program.stmtCapacity);
}
// </addFunctionToProgram>
