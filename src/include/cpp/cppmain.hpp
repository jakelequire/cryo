#ifndef CPP_MAIN_H
#define CPP_MAIN_H

#include "compiler/token.h"
#include "compiler/ast.h"
#include "cpp/codegen.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

    int generateCodeWrapper(ASTNode *node, CompilerState *state);

#ifdef __cplusplus
}
#endif

#endif // CPP_MAIN_H
