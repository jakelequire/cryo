
#include "compiler/parser.h"

#ifndef HAVE_STRNDUP
// <strndup>
char* strndup(const char* s, size_t n) {
    size_t len = strnlen(s, n);
    char* new_str = (char*)malloc(len + 1);
    if (new_str == NULL) {
        return NULL;
    }
    memcpy(new_str, s, len);
    new_str[len] = '\0';
    return new_str;
}
// </strndup>
#endif


char* tokenTypeToString(CryoTokenType type) {
    switch (type) {
        case TOKEN_INT_LITERAL:
            return "INT_LITERAL";
        case TOKEN_STRING_LITERAL:
            return "STRING_LITERAL";
        case TOKEN_BOOLEAN_LITERAL:
            return "BOOLEAN_LITERAL";
        case TOKEN_IDENTIFIER:
            return "IDENTIFIER";
        case TOKEN_OP_PLUS:
            return "OP_PLUS";
        case TOKEN_OP_MINUS:
            return "OP_MINUS";
        case TOKEN_OP_STAR: 
            return "OP_STAR";
        case TOKEN_OP_SLASH:
            return "OP_SLASH";
        case TOKEN_KW_CONST:
            return "KW_CONST";
        case TOKEN_KW_MUT:
            return "KW_MUT";
        case TOKEN_COLON:
            return "COLON";
        case TOKEN_ASSIGN:
            return "ASSIGN";
        case TOKEN_SEMICOLON:
            return "SEMICOLON";
        case TOKEN_LPAREN:
            return "LPAREN";
        case TOKEN_RPAREN:
            return "RPAREN";
        case TOKEN_ITER_STEP:
            return "ITER_STEP";
        case TOKEN_ITER_VAR:
            return "ITER_VAR";
        case TOKEN_EOF:
            return "EOF";
        default:
            return "UNKNOWN";
    }
}


// Scope-Declared Current Token
Token currentToken;


// Error handling function
// <error>
void error(const char* message) {
    fprintf(stderr, "[Parser] Error: %s at line %d, column %d\n", message, currentToken.line, currentToken.column);
    exit(1);
}
// </error>


// Function to get the next token from the lexer
// <getNextToken>
void getNextToken(Lexer *lexer) {
    if (isAtEnd(lexer)) {
        currentToken.type = TOKEN_EOF;
        currentToken.start = lexer->current;
        currentToken.length = 0;
        return;
    }
    currentToken = get_next_token(lexer);
}
// </getNextToken>



// Function to consume a token and check its type
// <consume>
void consume(Lexer *lexer, CryoTokenType type, const char *message) {
    printf("[Parser] Consuming token: %s (Expecting: %s)\n",
            tokenTypeToString(currentToken.type), tokenTypeToString(type));
    if (currentToken.type == type) {
        getNextToken(lexer);
    } else {
        error(message);
    }
}
// </consume>


// Helper function to map type strings to CryoDataType
// <getCryoDataType>
CryoDataType getCryoDataType(const char* typeStr) {
    if (strcmp(typeStr, "int") == 0) {
        return DATA_TYPE_INT;
    } else if (strcmp(typeStr, "float") == 0) {
        return DATA_TYPE_FLOAT;
    } else if (strcmp(typeStr, "string") == 0) {
        return DATA_TYPE_STRING;
    } else if (strcmp(typeStr, "boolean") == 0) {
        return DATA_TYPE_BOOLEAN;
    } else if (strcmp(typeStr, "void") == 0) {
        return DATA_TYPE_VOID;
    } else if (strcmp(typeStr, "null") == 0) {
        return DATA_TYPE_NULL;
    } else {
        printf("\n\n[!!Parser!!] Unknown data type: %s\n\n", typeStr); // Debugging
        return DATA_TYPE_UNKNOWN;
    }
}
// </getCryoDataType>


// Function to get the precedence of an operator
// <parsePublicDeclaration>
ASTNode* parsePublicDeclaration(Lexer* lexer, ParsingContext* context) {
    printf("[Parser] Parsing public declaration...\n");
    getNextToken(lexer); // Consume 'public'

    if (currentToken.type == TOKEN_KW_FN) {
        return parseFunctionDeclaration(lexer, VISIBILITY_PUBLIC, context);
    } else if (currentToken.type == TOKEN_KW_CONST || currentToken.type == TOKEN_KW_MUT) {
        return parseVarDeclaration(lexer, context);
    } else {
        error("Expected function or variable declaration after 'public'");
        return NULL;
    }
}
// </parsePublicDeclaration>


// Function to parse a primary expression
// <parsePrimaryExpression>
ASTNode* parsePrimaryExpression(Lexer* lexer, ParsingContext* context) {
    switch (currentToken.type) {
        case TOKEN_INT_LITERAL: {
            char buffer[32];
            strncpy(buffer, currentToken.start, currentToken.length);
            buffer[currentToken.length] = '\0';
            int value = atoi(buffer);
            getNextToken(lexer);
            return createLiteralExpr(value);
        }
        case TOKEN_STRING_LITERAL: {
            char* value = strndup(currentToken.start + 1, currentToken.length - 2);
            getNextToken(lexer);
            return createStringLiteralExpr(value);
        }
        case TOKEN_BOOLEAN_LITERAL: {
            int value = (strncmp(currentToken.start, "true", currentToken.length) == 0) ? 1 : 0;
            getNextToken(lexer);
            return createBooleanLiteralExpr(value);
        }
        case TOKEN_IDENTIFIER: {
            char* identifier = strndup(currentToken.start, currentToken.length);
            getNextToken(lexer);

            // Check if the next token is a left parenthesis and context is not parsing an if condition

            if (currentToken.type == TOKEN_LPAREN && context->isParsingIfCondition) {
                printf("\n\n\n[!!Parser!!] Function calls are not allowed in if conditions\n\n\n"); // Debugging
            }
            else if (currentToken.type == TOKEN_LPAREN && !context->isParsingIfCondition) {
                return parseFunctionCall(lexer, identifier, context);
            } 
            else {
                return createVariableExpr(identifier);
            }
        }
        case NODE_RETURN_STATEMENT: {
            return parseReturnStatement(lexer, context);
        }

        case TOKEN_ITER_VAR: {
            char* identifier = strndup(currentToken.start, currentToken.length);
            getNextToken(lexer);
            return createVariableExpr(identifier);
        }

        default:
            error("Expected an expression");
            return NULL;
    }
}
// </parsePrimaryExpression>


// Entry point for parsing expressions
ASTNode* parseExpression(Lexer* lexer, ParsingContext* context) {
    printf("[Parser] Parsing expression...\n");
    ASTNode* left = parsePrimaryExpression(lexer, context);

    while (currentToken.type == OPERATOR_ADD || currentToken.type == OPERATOR_SUB || 
           currentToken.type == OPERATOR_MUL || currentToken.type == OPERATOR_DIV || 
           currentToken.type == OPERATOR_LT || currentToken.type == OPERATOR_GT || 
           currentToken.type == OPERATOR_LTE || currentToken.type == OPERATOR_GTE || 
           currentToken.type == OPERATOR_EQ || currentToken.type == OPERATOR_NEQ) {
        CryoTokenType operatorType = currentToken.type;
        printf("[Parser] Found operator: %s\n", tokenTypeToString(operatorType));
        getNextToken(lexer);
        ASTNode* right = parsePrimaryExpression(lexer, context);
        left = createBinaryExpr(left, right, operatorType);
    }

    return left;
}




// Function to parse a variable declaration
// <parseVarDeclaration>
ASTNode* parseVarDeclaration(Lexer* lexer, ParsingContext* context) {
    printf("[Parser] Entering parseVarDeclaration\n");
    getNextToken(lexer);

    if (currentToken.type != TOKEN_IDENTIFIER) {
        error("[Parser] Expected variable name");
    }
    char* varName = strndup(currentToken.start, currentToken.length);
    getNextToken(lexer);

    CryoDataType dataType = DATA_TYPE_UNKNOWN;

    if (currentToken.type == TOKEN_COLON) {
        getNextToken(lexer);
        if (currentToken.type != TOKEN_IDENTIFIER) {
            error("[Parser] Expected type name");
        }
        char* varType = strndup(currentToken.start, currentToken.length);
        dataType = getCryoDataType(varType);
        free(varType);
        getNextToken(lexer);
    } else {
        error("[Parser] Expected ':' after variable name");
    }

    if (currentToken.type != TOKEN_ASSIGN) {
        error("[Parser] Expected '=' after variable name");
    }
    getNextToken(lexer);

    ASTNode* initializer = parseExpression(lexer, context);
    if (initializer == NULL) {
        error("[Parser] Expected expression after '='");
    } 

    consume(lexer, TOKEN_SEMICOLON, "Expected ';' after variable declaration");

    ASTNode* varDeclNode = createVarDeclarationNode(varName, dataType, initializer, currentToken.line);
    printf("[Parser] Created Variable Declaration Node: %s\n", varName);
    printf("[Parser] Variable Declaration Node Type: %d\n", varDeclNode->type);
    return varDeclNode;
}
// </parseVarDeclaration>


// <parseFunctionDeclaration>
ASTNode* parseFunctionDeclaration(Lexer* lexer, CryoVisibilityType visibility, ParsingContext* context) {
    printf("[Parser] Parsing function declaration...\n");
    consume(lexer, TOKEN_KW_FN, "Expected 'function' keyword");

    if (currentToken.type != TOKEN_IDENTIFIER) {
        error("Expected function name");
    }
    char* functionName = strndup(currentToken.start, currentToken.length);
    getNextToken(lexer);

    consume(lexer, TOKEN_LPAREN, "Expected '(' after function name");
    ASTNode* params = parseParameterList(lexer, context);
    if (params->data.paramList.paramCapacity == 0) {
        params = NULL;  // Set to NULL if no parameters are present
    }
    consume(lexer, TOKEN_RPAREN, "Expected ')' after parameters");

    CryoDataType returnType = DATA_TYPE_VOID;  // Default to void
    if (currentToken.type == TOKEN_RESULT_ARROW) {
        getNextToken(lexer);
        if (currentToken.type != TOKEN_IDENTIFIER) {
            error("Expected return type");
        }
        char* returnTypeStr = strndup(currentToken.start, currentToken.length);
        returnType = getCryoDataType(returnTypeStr);
        printf("\n\n[Parser] Return type identified: %s, Enum: %d\n", returnTypeStr, returnType);  // Add this line to debug
        free(returnTypeStr);
        getNextToken(lexer);
    }

    ASTNode* body = parseFunctionBlock(lexer, context);
    ASTNode* functionNode = createFunctionNode(functionName, params, body, returnType);
    functionNode->data.functionDecl.visibility = visibility;
    functionNode->data.functionDecl.returnType = returnType;

    printf("[Parser] Created Function Declaration Node: %s with return type: %d\n", functionName, returnType);
    return functionNode;
}
// </parseFunctionDeclaration>


// <parseParameterList>
ASTNode* parseParameterList(Lexer* lexer, ParsingContext* context) {
    // Parse parameter list
    ASTNode* paramList = createASTNode(NODE_PARAM_LIST);
    paramList->data.paramList.paramCount = 0;  // Initialize count
    paramList->data.functionDecl.paramCount = 0;  // Initialize count

    while (currentToken.type != TOKEN_RPAREN && currentToken.type != TOKEN_EOF) {
        if (currentToken.type == TOKEN_IDENTIFIER) {
            char* paramName = strndup(currentToken.start, currentToken.length);
            getNextToken(lexer);
            consume(lexer, TOKEN_COLON, "Expected ':' after parameter name");
            if (currentToken.type != TOKEN_IDENTIFIER) {
                error("Expected type name");
            }
            char* paramType = strndup(currentToken.start, currentToken.length);
            CryoDataType dataType = getCryoDataType(paramType);
            free(paramType);
            getNextToken(lexer);

            ASTNode* paramNode = createVarDeclarationNode(paramName, dataType, NULL, currentToken.line);
            addChildNode(paramList, paramNode);
            paramList->data.paramList.paramCount++;  // Increment count
            paramList->data.functionDecl.paramCount++;  // Increment count
            
            if (currentToken.type == TOKEN_COMMA) {
                getNextToken(lexer);
            } else if (currentToken.type != TOKEN_RPAREN) {
                error("Expected ',' or ')' after parameter");
            }
        } else {
            error("Expected parameter name");
        }
    }
    return paramList;
}
// </parseParameterList>


// <parseReturnStatement>
ASTNode* parseReturnStatement(Lexer* lexer, ParsingContext* context) {
    printf("[Parser] Parsing return statement...\n");
    ASTNode* returnNode = createASTNode(NODE_RETURN_STATEMENT);

    getNextToken(lexer); // Consume 'return'

    // Parse the return value if present
    if (currentToken.type != TOKEN_SEMICOLON) {
        ASTNode* returnValue = parseExpression(lexer, context);
        if (returnValue) {
            returnNode->data.returnStmt.returnValue = returnValue;
            returnNode->data.returnStmt.returnType = returnValue->type;
            printf("[Parser] Return value parsed with type: %d\n", returnValue->type);
        } else {
            printf("[Parser] Error: Failed to parse return value\n");
        }
    } else {
        // No return value, set return type to void
        returnNode->data.returnStmt.returnValue = NULL;
        returnNode->data.returnStmt.returnType = DATA_TYPE_VOID;
        printf("[Parser] No return value\n");
    }

    consume(lexer, TOKEN_SEMICOLON, "Expected ';' after return value");
    printf("[Parser] Created Return Statement Node with return value of type: %d\n", returnNode->data.returnStmt.returnValue ? returnNode->data.returnStmt.returnValue->type : -1);
    return returnNode;
}
// </parseReturnStatement>


// <parseFunctionCall>
ASTNode* parseFunctionCall(Lexer* lexer, const char* functionName, ParsingContext* context) {
    printf("[Parser] Parsing function call for function: %s\n", functionName);
    consume(lexer, TOKEN_LPAREN, "Expected '(' after function name");
    
    ASTNode** arguments = NULL;
    int argCount = 0;
    
    if (currentToken.type != TOKEN_RPAREN) {
        arguments = (ASTNode**)malloc(sizeof(ASTNode*) * MAX_ARGUMENTS);
        do {
            arguments[argCount++] = parseExpression(lexer, context);
            if (argCount >= MAX_ARGUMENTS) {
                error("Too many arguments in function call");
            }
        } while (currentToken.type == TOKEN_COMMA);
    }
    
    consume(lexer, TOKEN_RPAREN, "Expected ')' after function arguments");
    consume(lexer, TOKEN_SEMICOLON, "Expected ';' after function call");
    
    ASTNode* node = createASTNode(NODE_FUNCTION_CALL);
    node->data.functionCall.name = strdup(functionName);
    node->data.functionCall.args = arguments;
    node->data.functionCall.argCount = argCount;
    
    printf("[Parser] Created Function Call Node: %s with %d arguments\n", functionName, argCount);
    return node;
}
// </parseFunctionCall>


// <parseIfStatement>
ASTNode* parseIfStatement(Lexer* lexer, ParsingContext* context) {
    printf("[Parser] Parsing if statement...\n");
    context->isParsingIfCondition = true;
    consume(lexer, TOKEN_KW_IF, "Expected 'if'");
    consume(lexer, TOKEN_LPAREN, "Expected '(' after 'if'");

    printf("[Parser] Parsing condition...\n");
    ASTNode* condition = parseExpression(lexer, context);
    context->isParsingIfCondition = false;

    if (!condition) {
        error("Expected condition expression after '('");
    }
    printf("[Parser] Condition parsed.\n");

    consume(lexer, TOKEN_RPAREN, "Expected ')' after condition");

    printf("[Parser] Parsing then branch...\n");
    ASTNode* thenBranch = parseBlock(lexer, context);
    ASTNode* elseBranch = NULL;

    if (currentToken.type == TOKEN_KW_ELSE) {
        getNextToken(lexer);
        elseBranch = parseBlock(lexer, context);
    }

    return createIfStatement(condition, thenBranch, elseBranch);
}
// </parseIfStatement>


// Function to parse a for loop
// <parseForLoop>
ASTNode* parseForLoop(Lexer* lexer, ParsingContext* context) {
    printf("[Parser] Parsing for loop...\n");
    consume(lexer, TOKEN_KW_FOR, "Expected 'for'");
    consume(lexer, TOKEN_LPAREN, "Expected '(' after 'for'");
    
    // Parse the iterator variable
    Token iter_var_token = currentToken;

    consume(lexer, TOKEN_ITER_VAR, "Expected iterator variable '$'");

    // create the iterator variable node
    ASTNode* iter_var_init = createLiteralExpr(0);
    consume(lexer, TOKEN_ITER_STEP, "Expected '..' after iterator variable");
    
    // Parse the end expression
    ASTNode* end_expr = parseExpression(lexer, context);
    
    consume(lexer, TOKEN_RPAREN, "Expected ')' after end expression");
    
    // Parse the loop body
    ASTNode* body = parseBlock(lexer, context);
    
    // Create the for loop node in the AST
    ASTNode* for_node = createForStatement(createVariableExpr(strndup(iter_var_token.start, iter_var_token.length)), end_expr, createLiteralExpr(1), body);
    for_node->data.forStmt.initializer = iter_var_init;  // Set the initializer to 0
    return for_node;
}
// </parseForLoop>


// Function to parse a statement
// <parseStatement>
ASTNode* parseStatement(Lexer* lexer, ParsingContext* context) {
    printf("[Parser] Parsing statement...\n");


    switch(currentToken.type) {
        case TOKEN_KW_CONST:
        case TOKEN_KW_MUT:
            printf("[Parser] Parsing variable declaration.\n");
            return parseVarDeclaration(lexer, context);
        case TOKEN_KW_IF:
            return parseIfStatement(lexer, context);
        case TOKEN_IDENTIFIER:
            printf("[Parser] Identified Identifier: %.*s\n", currentToken.length, currentToken.start);
            return parsePrimaryExpression(lexer, context);
        case TOKEN_KW_PUBLIC:
            return parsePublicDeclaration(lexer, context);
        case TOKEN_KW_PRIVATE:
            return parsePublicDeclaration(lexer, context);
        case TOKEN_KW_RETURN:
            return parseReturnStatement(lexer, context);
        case TOKEN_KW_FOR:
            return parseForLoop(lexer, context);
        case TOKEN_EOF:
            printf("[Parser] Reached end of file.\n");
            return NULL;
        default:
            error("Expected statement");
            return NULL;
    }
}
// </parseStatement>



// <createProgramNode>
ASTNode* createProgramNode() {
    ASTNode* node = createASTNode(NODE_PROGRAM);
    if (!node) {
        fprintf(stderr, "[AST] [ERROR] Failed to allocate memory for program node\n");
        return NULL;
    }
    node->data.program.stmtCount = 0;
    node->data.program.stmtCapacity = 2;
    node->data.program.statements = (ASTNode**)calloc(node->data.program.stmtCapacity, sizeof(ASTNode*));
    if (!node->data.program.statements) {
        fprintf(stderr, "[AST] [ERROR] Failed to allocate memory for program statements\n");
        free(node);
        return NULL;
    }
    return node;
}
// </createProgramNode>


// Function to add a statement to the program node
// <addStatementToProgram>
void addStatementToProgram(ASTNode* programNode, ASTNode* statement) {
    if (!programNode || programNode->type != NODE_PROGRAM) {
        fprintf(stderr, "[AST_ERROR] Invalid program node\n");
        return;
    }

    printf("[AST_DEBUG] Before adding statement: stmtCount = %d, stmtCapacity = %d\n", programNode->data.program.stmtCount, programNode->data.program.stmtCapacity);
    if (programNode->data.program.stmtCount >= programNode->data.program.stmtCapacity) {
        int newCapacity = programNode->data.program.stmtCapacity == 0 ? 2 : programNode->data.program.stmtCapacity * 2;
        printf("[AST_DEBUG] Increasing stmtCapacity to: %d\n", newCapacity);
        programNode->data.program.statements = realloc(programNode->data.program.statements, newCapacity * sizeof(ASTNode*));
        if (!programNode->data.program.statements) {
            fprintf(stderr, "[AST_ERROR] Failed to reallocate memory for program statements\n");
            return;
        }
        programNode->data.program.stmtCapacity = newCapacity;
    }

    programNode->data.program.statements[programNode->data.program.stmtCount++] = statement;
    printf("[AST_DEBUG] After adding statement: stmtCount = %d, stmtCapacity = %d\n", programNode->data.program.stmtCount, programNode->data.program.stmtCapacity);
}
// </addStatementToProgram>


// <parseBlock>
ASTNode* parseBlock(Lexer* lexer, ParsingContext* context) {
    printf("[Parser] Parsing block...\n");
    ASTNode* blockNode = createASTNode(NODE_BLOCK);
    consume(lexer, TOKEN_LBRACE, "Expected '{' to start block");

    while (currentToken.type != TOKEN_RBRACE && currentToken.type != TOKEN_EOF) {
        ASTNode* stmt = parseStatement(lexer, context);
        if (stmt) {
            addChildNode(blockNode, stmt);
        } else {
            error("Failed to parse statement in block");
        }
    }

    consume(lexer, TOKEN_RBRACE, "Expected '}' to end block");
    return blockNode;
}
// </parseBlock>


ASTNode* parseFunctionBlock(Lexer* lexer, ParsingContext* context) {
    ASTNode* blockNode = createASTNode(NODE_FUNCTION_BLOCK);
    consume(lexer, TOKEN_LBRACE, "Expected '{' to start function block");

    while (currentToken.type != TOKEN_RBRACE && currentToken.type != TOKEN_EOF) {
        ASTNode* stmt = parseStatement(lexer, context);
        if (stmt) {
            addStatementToBlock(blockNode->data.functionBlock.block, stmt);
        }
    }

    consume(lexer, TOKEN_RBRACE, "Expected '}' to end function block");
    return blockNode;
}



// Function to parse the entire program
// <parseProgram>
ASTNode* parseProgram(Lexer* lexer) {
    printf("[Parser] Parsing program\n");

    ParsingContext context = {false}; // Initialize context

    ASTNode* programNode = createProgramNode();
    if (!programNode) {
        fprintf(stderr, "[Parser] [ERROR] Failed to create program node\n");
        return NULL;
    }

    getNextToken(lexer);

    while (currentToken.type != TOKEN_EOF) {
        ASTNode* stmt = parseStatement(lexer, &context);
        if (stmt) {
            addStatementToProgram(programNode, stmt);
        } else {
            fprintf(stderr, "[Parser] [ERROR] Failed to parse statement\n");
            freeAST(programNode);
            return NULL;
        }
    }

    return programNode;
}
// </parseProgram>




// ---------------------------------------------------------------------------------------------------------------------
// DEBUGGING FUNCTIONS

// <parser>
int parser(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "[Parser] Usage: %s <path_to_file>\n", argv[0]);
        return 1;
    }

    char* source = readFile(argv[1]);
    if (source == NULL) return 1;

    Lexer lexer;
    initLexer(&lexer, source);

    ASTNode* program = parseProgram(&lexer);
    if (!program) {
        fprintf(stderr, "[Parser] [ERROR] Failed to parse program\n");
        free(source);
        return 1;
    }

    printAST(program, 0);

    free(source);
    freeAST(program);
    return 0;
}
// </parser>
