#include "cppASTNode.h"

void IntLiteral::print() const {
    std::cout << value;
}

void BinaryOp::print() const {
    std::cout << "(";
    lhs->print();
    std::cout << " " << op << " ";
    rhs->print();
    std::cout << ")";
}
