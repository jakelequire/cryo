#ifndef CPP_MAIN_H
#define CPP_MAIN_H

#include "compiler/token.h"
#include "compiler/ast.h"

#ifdef __cplusplus
extern "C" {
#endif

int cppFunction(ASTNode* node);

#ifdef __cplusplus
}
#endif

#endif // CPP_MAIN_H
