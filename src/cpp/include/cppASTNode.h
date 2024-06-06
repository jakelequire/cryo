#ifndef CPP_ASTNODE_H
#define CPP_ASTNODE_H

#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include <fstream>

// Base class for all AST Nodes
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void print() const = 0;
};

class BinaryOp : public ASTNode {
    char op;
    std::unique_ptr<ASTNode> lhs, rhs;

public:
    BinaryOp(char op, std::unique_ptr<ASTNode> lhs, std::unique_ptr<ASTNode> rhs)
    : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {};
    char getOp() const { return op; }
    const ASTNode* getLHS() const { return lhs.get(); };
    const ASTNode* getRHS() const { return rhs.get(); };
    void print() const override;
};

#endif