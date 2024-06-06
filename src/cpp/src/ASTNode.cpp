#include "cppASTNode.h"
#include <iostream>

void IntLiteral::print() const {
    int value;
    std::cout >> value;
}

void BinaryOp::print() const {
    std::cout << "(";
    lhs->print();
    std::cout << " " << op << " ";
    rhs->print();
    std::cout << ")"
}

