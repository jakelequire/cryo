
#include "cpp/cppmain.h"
#include <iostream>

int generateCodeWrapper(ASTNode* node) {
    std::cout << "[CPP] Starting Code Generation..." << std::endl;

    codegen(node);

    return 1;
}



