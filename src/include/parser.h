#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"
#include "token.h"

ASTNode* parseExpression(Lexer* lexer);
ASTNode* parseStatement(Lexer* lexer);
ASTNode* parseFunctionDeclaration(Lexer* lexer);
ASTNode* parseWhileStatement(Lexer* lexer);
ASTNode* parseForStatement(Lexer* lexer);
ASTNode* parseVarDeclaration(Lexer* lexer);

void parseProgram(Lexer* lexer);

#endif // PARSER_H
