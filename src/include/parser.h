#ifndef PARSER_H
#define PARSER_H

#define INITIAL_STATEMENT_CAPACITY 256

#include "lexer.h"
#include "ast.h"
#include "token.h"

ASTNode* parseParamList(Lexer* lexer);
ASTNode* parseType(Lexer* lexer);

// Function prototypes
ASTNode* parseProgram(Lexer* lexer);
ASTNode* parseStatement(Lexer* lexer);
ASTNode* parseFunctionDeclaration(Lexer* lexer);
ASTNode* parseBlock(Lexer* lexer);
ASTNode* parseExpression(Lexer* lexer);
int getOperatorPrecedence(CryoTokenType type);
ASTNode* parseIfStatement(Lexer* lexer);
ASTNode* parseReturnStatement(Lexer* lexer);
ASTNode* parseWhileStatement(Lexer* lexer);
ASTNode* parseForStatement(Lexer* lexer);
ASTNode* parseVarDeclaration(Lexer* lexer);
ASTNode* parseExpressionStatement(Lexer* lexer);

#endif // PARSER_H