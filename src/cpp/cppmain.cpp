
#include "cpp/cppmain.h"
#include <iostream>

int cppFunction(ASTNode* node) {
    std::cout << "Processing ASTNode of type: " << node->type << " on line: " << node->line << std::endl;

    return 1;
}



