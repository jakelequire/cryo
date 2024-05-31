#ifndef PARSER_H
#define PARSER_H
#define INITIAL_STATEMENT_CAPACITY 256
/*------ <includes> ------*/
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*---<custom_includes>---*/
#include "lexer.h"
#include "ast.h"
#include "token.h"
/*---------<end>---------*/


/*-----<function_prototypes>-----*/
#ifndef HAVE_STRNDUP
char* strndup(const char* s, size_t n);
#endif
// Helper Functions
void error(const char* message);
void getNextToken(Lexer *lexer);
void consume(Lexer *lexer, CryoTokenType type, const char *message);
int getOperatorPrecedence(CryoTokenType type);
// AST Nodes
ASTNode* parsePrimaryExpression(Lexer* lexer);
ASTNode* parseUnaryExpression(Lexer* lexer);
ASTNode* parseBinaryExpression(Lexer* lexer, int precedence);
ASTNode* parseExpression(Lexer* lexer);
ASTNode* parseStatement(Lexer* lexer);
ASTNode* parseIfStatement(Lexer* lexer);
ASTNode* parseReturnStatement(Lexer* lexer);
ASTNode* parseExpressionStatement(Lexer* lexer);
ASTNode* parseWhileStatement(Lexer* lexer);
ASTNode* parseForStatement(Lexer* lexer);
ASTNode* parseVarDeclaration(Lexer* lexer);
ASTNode* parseBlock(Lexer* lexer);
ASTNode* parseFunctionDeclaration(Lexer* lexer);
ASTNode* parseParamList(Lexer* lexer);
ASTNode* createTypeNode(CryoTokenType type);
ASTNode* parseType(Lexer* lexer);
ASTNode* createParamNode(const char* name, ASTNode* type);
ASTNode* createFunctionDeclNode(const char* name, ASTNode* params, ASTNode* returnType, ASTNode* body);
// Entry Point
ASTNode* parseProgram(Lexer* lexer);
/*-----<end_prototypes>-----*/


#endif // PARSER_H