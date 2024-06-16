#include "cppASTNode.h"

void IntLiteral::print() const {
    std::cout << ">==--- [START IntLiteral::print()] ---==<" << std::endl;
    std::cout << "\n" << value << "\n";
    std::cout << ">==--- [END IntLiteral::print()] ---==<" << std::endl;
}

void BinaryOp::print() const {
    std::cout << ">==--- [START BinaryOp::print()] ---==<" << std::endl;
    lhs->print();
    std::cout << "\n" << op << "\n";
    rhs->print();
    std::cout << ">==--- [END BinaryOp::print()] ---==<" << std::endl;
}

