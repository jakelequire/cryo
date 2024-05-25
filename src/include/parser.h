#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"
#include "token.h"

void consume(Lexer *lexer, TokenType type, const char *message);
ASTNode* createBlock();
ASTNode* parsePrimaryExpression(Lexer* lexer);
ASTNode* parseUnaryExpression(Lexer* lexer);
ASTNode* parseBinaryExpression(Lexer* lexer, int precedence);
int getOperatorPrecedence(TokenType type);
// New function declarations for function and return statement parsing
ASTNode* parseProgram(Lexer* lexer);
ASTNode* parseBlock(Lexer* lexer);
ASTNode* parseExpression(Lexer* lexer);
ASTNode* parseStatement(Lexer* lexer);
ASTNode* parseIfStatement(Lexer* lexer);
ASTNode* parseReturnStatement(Lexer* lexer);
ASTNode* parseExpressionStatement(Lexer* lexer);
ASTNode* parseWhileStatement(Lexer* lexer);
ASTNode* parseForStatement(Lexer* lexer);
ASTNode* parseVarDeclaration(Lexer* lexer);
ASTNode* parseFunctionDeclaration(Lexer* lexer);


#endif // PARSER_H
