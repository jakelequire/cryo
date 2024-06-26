#include "cpp/cppLogger.h"

const char* logNode(ASTNode* node) {
    switch (node->type) {
        case CryoNodeType::NODE_PROGRAM:
            std::cout                   << "\n"
            << "\nProgram Node:\t"      << node->type
            << "\nstatements\t"         << node->data.program.stmtCount
            << "\ncapacity\t"           << node->data.program.stmtCapacity 
            << "\nstatements:\t"         << logNode(*node->data.program.statements)
            << std::endl;
            return "\n<Program>\n";
            break;
        case CryoNodeType::NODE_STATEMENT:
            std::cout                   << "\n"
            << "\nStatement Node:\t"    << node->type
            << "\nstmt\t"               << node->data.stmt.stmt
            << std::endl;
            return "\n<Statement>\n";
            break;

        case CryoNodeType::NODE_EXPRESSION:
            std::cout                   << "\n"
            << "\nExpression Node:\t"   << node->type
            << "\nexpr\t"               << node->data.expr.expr
            << std::endl;
            return "\n<Expression>\n";
            break;

        case CryoNodeType::NODE_LITERAL_EXPR:
            std::cout                   << "\n"
            << "\nLiteral Node:\t"      << node->type
            << "\nvalue\t"              << node->data.literalExpression.value
            << std::endl;
            return "\n<Literal>\n";
            break; 

        case CryoNodeType::NODE_FUNCTION_DECLARATION:
            std::cout                   << "\n"
            << "\nFunction Node:\t"     << node->type
            << "\nvisibility\t"         << node->data.functionDecl.visibility
            << "\nname\t"               << node->data.functionDecl.name
            << "\nparams\t"             << node->data.functionDecl.params
            << "\nreturnType\t"         << node->data.functionDecl.returnType
            << "\nbody\t"               << node->data.functionDecl.body
            << std::endl;
            return "\n<Function>\n";
            break;
        
        case CryoNodeType::NODE_VAR_DECLARATION:
            std::cout                   << "\n"
            << "\nVar Node:\t"          << node->type
            << "\nname\t"               << node->data.varDecl.name
            << "\ntype\t"               << node->data.varDecl.type
            << "\ninitializer\t"        << node->data.varDecl.initializer
            << std::endl;
            return "\n<Var>\n";
            break;

        case CryoNodeType::NODE_PARAM_LIST:
            std::cout                   << "\n"
            << "\nParam Node:\t"        << node->type
            << "\nparams\t"             << node->data.paramList.params
            << "\nparamCount\t"         << node->data.paramList.paramCount
            << "\nparamCapacity\t"      << node->data.paramList.paramCapacity
            << std::endl;
            return "\n<Param>\n";
            break;
        
        case CryoNodeType::NODE_BLOCK:
            std::cout                   << "\n"
            << "\nBlock Node:\t"        << node->type
            << "\nstmts\t"              << node->data.block.statements
            << "\nstmtCount\t"          << node->data.block.stmtCount
            << "\nstmtCapacity\t"       << node->data.block.stmtCapacity
            << std::endl;
            return "\n<Block>\n";
            break;

        case CryoNodeType::NODE_BINARY_EXPR:
            std::cout                   << "\n"
            << "\nBinOp Node:\t"        << node->type
            << "\nleft\t"               << node->data.bin_op.left
            << "\nright\t"              << node->data.bin_op.right
            << "\nop\t"                 << node->data.bin_op.op
            << "\noperatorText\t"       << node->data.bin_op.operatorText
            << std::endl;
            return "\n<BinOp>\n";
            break;

        default:
            return "<Unknown>";
            break;
    }

}



