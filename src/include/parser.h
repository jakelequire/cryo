#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

#ifndef HAVE_STRNDUP
char* strndup(const char* s, size_t n);
#endif

void consume(Lexer *lexer, CryoTokenType type, const char *message);
ASTNode* createBlock();
ASTNode* parsePrimaryExpression(Lexer* lexer);
ASTNode* parseUnaryExpression(Lexer* lexer);
ASTNode* parseBinaryExpression(Lexer* lexer, int precedence);
int getOperatorPrecedence(CryoTokenType type);
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
