#include "cpp/codegen.h"

namespace Cryo
{
    /// @brief The entry point to the generation process.
    /// @param node
    void Generator::handleProgram(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Generator", "Handling Program");

        if (node->data.program->statementCount == 0)
        {
            debugger.logMessage("ERROR", __LINE__, "Generator", "No statements in program");
            return;
        }

        for (int i = 0; i < node->data.program->statementCount; ++i)
        {
            std::cout << "Processing Statement " << i + 1 << " of " << node->data.program->statementCount << std::endl;
            parseTree(node->data.program->statements[i]);
        }

        debugger.logMessage("INFO", __LINE__, "Generator", "Program Handled");
        return;
    }
    // -----------------------------------------------------------------------------------------------

    void Generator::handleFunctionDeclaration(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Functions &functions = compiler.getFunctions();
        debugger.logMessage("INFO", __LINE__, "Generator", "Handling Function Declaration");

        functions.handleFunction(node);

        debugger.logMessage("INFO", __LINE__, "Generator", "Function Declaration Handled");
        return;
    }

    void Generator::handleFunctionBlock(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Functions &functions = compiler.getFunctions();
        debugger.logMessage("INFO", __LINE__, "Generator", "Handling Function Block");

        functions.handleFunction(node);

        debugger.logMessage("INFO", __LINE__, "Generator", "Function Block Handled");
        return;
    }

    void Generator::handleReturnStatement(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Functions &functions = compiler.getFunctions();
        debugger.logMessage("INFO", __LINE__, "Generator", "Handling Return Statement");

        functions.handleFunction(node);

        debugger.logMessage("INFO", __LINE__, "Generator", "Return Statement Handled");
        return;
    }

    void Generator::handleFunctionCall(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Generator", "Handling Function Call");

        exit(1);
    }

    void Generator::handleVariableDeclaration(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Variables &variableInstance = compiler.getVariables();
        debugger.logMessage("INFO", __LINE__, "Generator", "Handling Variable Declaration");

        CryoVariableNode *varDecl = node->data.varDecl;

        bool isRef = varDecl->isReference;
        if (isRef)
        {
            // A Pointer variable
            variableInstance.handleRefVariable(node);
        }
        else
        {
            // A Constant variable
            variableInstance.handleConstVariable(node);
        }

        // exit(1);
    }

    void Generator::handleBinaryExpression(ASTNode *node)
    {
        std::cout << "[CPP] Handling Binary Expression" << std::endl;
        // TODO: Implement
    }

    void Generator::handleUnaryExpression(ASTNode *node)
    {
        std::cout << "[CPP] Handling Unary Expression" << std::endl;
        // TODO: Implement
    }

    void Generator::handleLiteralExpression(ASTNode *node)
    {
        std::cout << "[CPP] Handling Literal Expression" << std::endl;
        // TODO: Implement
    }

    void Generator::handleIfStatement(ASTNode *node)
    {
        std::cout << "[CPP] Handling If Statement" << std::endl;
        // TODO: Implement
    }

    void Generator::handleWhileStatement(ASTNode *node)
    {
        std::cout << "[CPP] Handling While Statement" << std::endl;
        // TODO: Implement
    }

    void Generator::handleForStatement(ASTNode *node)
    {
        std::cout << "[CPP] Handling For Statement" << std::endl;
        // TODO: Implement
    }

}
