#ifndef CPP_ASTNODE_H
#define CPP_ASTNODE_H

#include <iostream>

class ASTNode {
    public:
        virtual ~ASTNode() = default;
        virtual void print() const = 0;  // Declare print method as pure virtual
};

class IntLiteral : public ASTNode {
    public:
        IntLiteral(int value) : value(value) {}
        int getValue() const { return value; }
        void print() const override;  // Override print method
    private:
        int value;
};

class BinaryOp : public ASTNode {
    public:
        BinaryOp(ASTNode* lhs, ASTNode* rhs, char op) : lhs(lhs), rhs(rhs), op(op) {}
        ASTNode* getLHS() const { return lhs; }
        ASTNode* getRHS() const { return rhs; }
        char getOp() const { return op; }
        void print() const override;  // Override print method
    private:
        ASTNode* lhs;
        ASTNode* rhs;
        char op;
};

#endif // CPP_ASTNODE_H