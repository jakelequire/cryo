#include "cpp/cppLogger.h"

#include "cpp/cppLogger.h"

void logNode(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case CryoNodeType::NODE_PROGRAM:
            std::cout   << "\n=== Program Node ==="
                        << "\nType:\t" << node->type
                        << "\nStatements:\t" << node->data.program.stmtCount
                        << "\nCapacity:\t" << node->data.program.stmtCapacity
                        << "\nStatements:\t"
                        << std::endl;
            logNode(*node->data.program.statements);
            break;
        
        case CryoNodeType::NODE_STATEMENT:
            std::cout   << "\n=== Statement Node ==="
                        << "\nType:\t" << node->type
                        << "\nStatement:\t"
                        << std::endl;
            logNode(node->data.stmt.stmt);
            break;

        case CryoNodeType::NODE_EXPRESSION:
            std::cout   << "\n=== Expression Node ==="
                        << "\nType:\t" << node->type
                        << "\nExpression:\t"
                        << std::endl;
            logNode(node->data.expr.expr);
            break;

        case CryoNodeType::NODE_LITERAL_EXPR:
            std::cout   << "\n=== Literal Node ==="
                        << "\nType:\t" << node->type
                        << "\nValue:\t" << node->data.literalExpression.value
                        << std::endl;
            break; 

        case CryoNodeType::NODE_FUNCTION_DECLARATION:
            std::cout   << "\n=== Function Node ==="
                        << "\nType:\t" << node->type
                        << "\nVisibility:\t" << node->data.functionDecl.visibility
                        << "\nName:\t" << node->data.functionDecl.name
                        << "\nParams:\t" << node->data.functionDecl.params
                        << "\nReturn Type:\t" << node->data.functionDecl.returnType
                        << "\nBody:\t" << node->data.functionDecl.body
                        << std::endl;
            logNode(node->data.functionDecl.body);
            break;
        
        case CryoNodeType::NODE_VAR_DECLARATION:
            std::cout   << "\n=== Variable Node ==="
                        << "\nType:\t" << node->type
                        << "\nName:\t" << node->data.varDecl.name
                        << "\nVar Type:\t" << node->data.varDecl.type
                        << "\nInitializer:\t"
                        << std::endl;
            logNode(node->data.varDecl.initializer);
            break;

        case CryoNodeType::NODE_PARAM_LIST:
            std::cout   << "\n=== Parameter Node ==="
                        << "\nType:\t" << node->type
                        << "\nParams:\t" << node->data.paramList.params
                        << "\nParam Count:\t" << node->data.paramList.paramCount
                        << "\nParam Capacity:\t" << node->data.paramList.paramCapacity
                        << std::endl;
            break;
        
        case CryoNodeType::NODE_BLOCK:
            std::cout   << "\n=== Block Node ==="
                        << "\nType:\t" << node->type
                        << "\nStatements:\t" << node->data.block.statements
                        << "\nStmt Count:\t" << node->data.block.stmtCount
                        << "\nStmt Capacity:\t" << node->data.block.stmtCapacity
                        << std::endl;
            logNode(*node->data.block.statements);
            break;

        case CryoNodeType::NODE_BINARY_EXPR:
            std::cout   << "\n=== Binary Operation Node ==="
                        << "\nType:\t" << node->type
                        << "\nLeft:\t" << node->data.bin_op.left
                        << "\nRight:\t" << node->data.bin_op.right
                        << "\nOperator:\t" << node->data.bin_op.op
                        << "\nOperator Text:\t" << node->data.bin_op.operatorText
                        << std::endl;
            logNode(node->data.bin_op.left);
            logNode(node->data.bin_op.right);
            break;

        default:
            std::cerr << "Unknown node type\n";
            break;
    }
}




