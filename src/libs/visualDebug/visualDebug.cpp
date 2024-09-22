/********************************************************************************
 *  Copyright 2024 Jacob LeQuire                                                *
 *  SPDX-License-Identifier: Apache-2.0                                         *
 *    Licensed under the Apache License, Version 2.0 (the "License");           *
 *    you may not use this file except in compliance with the License.          *
 *    You may obtain a copy of the License at                                   *
 *                                                                              *
 *    http://www.apache.org/licenses/LICENSE-2.0                                *
 *                                                                              *
 *    Unless required by applicable law or agreed to in writing, software       *
 *    distributed under the License is distributed on an "AS IS" BASIS,         *
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
 *    See the License for the specific language governing permissions and       *
 *    limitations under the License.                                            *
 *                                                                              *
 ********************************************************************************/
#include "visualDebug.hpp"

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900
#define NODE_WIDTH 180
#define NODE_HEIGHT 100
#define VERTICAL_SPACING 150
#define HORIZONTAL_SPACING 50

// C Interface
#ifdef __cplusplus
extern "C"
{
#endif

    ASTNode *getProgramNode(const char *source);
    char *readFile(const char *path);

#ifdef __cplusplus
}
#endif

// Implement this function based on your CryoNodeType enum
std::string getNodeTypeString(CryoNodeType type)
{
    switch (type)
    {
    case NODE_PROGRAM:
    {
        return "Program";
    }
    case NODE_FUNCTION_DECLARATION:
    {
        return "Func Declaration";
    }
    case NODE_VAR_DECLARATION:
    {
        return "Var Declaration";
    }
    case NODE_STATEMENT:
    {
        return "Statement";
    }
    case NODE_EXPRESSION:
    {
        return "Expression";
    }
    case NODE_BINARY_EXPR:
    {
        return "Binary Expression";
    }
    case NODE_UNARY_EXPR:
    {
        return "Unary Expression";
    }
    case NODE_LITERAL_EXPR:
    {
        return "Literal Expression";
    }
    case NODE_VAR_NAME:
    {
        return "Variable Name";
    }
    case NODE_FUNCTION_CALL:
    {
        return "Function Call";
    }
    case NODE_IF_STATEMENT:
    {
        return "If Statement";
    }
    case NODE_WHILE_STATEMENT:
    {
        return "While Statement";
    }
    case NODE_FOR_STATEMENT:
    {
        return "For Statement";
    }
    case NODE_RETURN_STATEMENT:
    {
        return "Return Statement";
    }
    case NODE_BLOCK:
    {
        return "Block";
    }
    case NODE_FUNCTION_BLOCK:
    {
        return "Function Block";
    }
    case NODE_EXPRESSION_STATEMENT:
    {
        return "Expression Statement";
    }
    case NODE_ASSIGN:
    {
        return "Assignment";
    }
    case NODE_PARAM_LIST:
    {
        return "Parameter List";
    }
    case NODE_TYPE:
    {
        return "Type";
    }
    case NODE_STRING_LITERAL:
    {
        return "String Literal";
    }
    case NODE_STRING_EXPRESSION:
    {
        return "String Expression";
    }
    case NODE_BOOLEAN_LITERAL:
    {
        return "Boolean Literal";
    }
    case NODE_ARRAY_LITERAL:
    {
        return "Array Literal";
    }
    case NODE_IMPORT_STATEMENT:
    {
        return "Import Statement";
    }
    case NODE_EXTERN_STATEMENT:
    {
        return "Extern Statement";
    }
    case NODE_EXTERN_FUNCTION:
    {
        return "Extern Function";
    }
    case NODE_ARG_LIST:
    {
        return "Argument List";
    }
    case NODE_NAMESPACE:
    {
        return "Namespace";
    }
    case NODE_INDEX_EXPR:
    {
        return "Index Expression";
    }
    case NODE_VAR_REASSIGN:
    {
        return "Variable Reassignment";
    }
    case NODE_UNKNOWN:
    {
        return "Unknown";
    }
    default:
        return "DEFAULTED";
    }
}

iNode *createINode(ASTNode *astNode)
{
    if (astNode == nullptr)
        return nullptr;

    iNode *iNode = new struct iNode();
    iNode->nodeType = astNode->metaData->type;

    switch (astNode->metaData->type)
    {
    case NODE_PROGRAM:
    {
        std::cout << "Program" << std::endl;
        iNode->name = "Program";
        break;
    }
    case NODE_NAMESPACE:
    {
        std::cout << "Namespace: " << astNode->data.cryoNamespace->name << std::endl;
        iNode->name = std::string(astNode->data.cryoNamespace->name);
        break;
    }
    case NODE_FUNCTION_DECLARATION:
    {
        std::cout << "Function Declaration: " << astNode->data.functionDecl->name << std::endl;
        iNode->name = std::string(astNode->data.functionDecl->name);
        iNode->dataType = astNode->data.functionDecl->returnType;
        break;
    }
    case NODE_VAR_DECLARATION:
    {
        std::cout << "Variable Declaration: " << astNode->data.varDecl->name << std::endl;
        iNode->name = std::string(astNode->data.varDecl->name);
        CryoDataType dataType = astNode->data.varDecl->type;
        CryoNodeType exprType = astNode->data.varDecl->initializer->metaData->type;
        iNode->dataType = dataType;
        if (exprType == NODE_LITERAL_EXPR)
        {
            iNode->value = createINode(astNode->data.varDecl->initializer)->value;
        }
        else
        {
            iNode->value = "Expression";
        }
        break;
    }
    case NODE_LITERAL_EXPR:
    {
        std::cout << "Literal Expression: " << std::endl;
        iNode->name = "Literal";
        CryoDataType dataType = astNode->data.literal->dataType;
        iNode->dataType = dataType;
        if (dataType == DATA_TYPE_INT)
        {
            iNode->value = std::to_string(astNode->data.literal->value.intValue);
        }
        else if (dataType == DATA_TYPE_FLOAT)
        {
            iNode->value = std::to_string(astNode->data.literal->value.floatValue);
        }
        else if (dataType == DATA_TYPE_STRING)
        {
            iNode->value = std::string(astNode->data.literal->value.stringValue);
        }
        else if (dataType == DATA_TYPE_BOOLEAN)
        {
            iNode->value = astNode->data.literal->value.booleanValue ? "true" : "false";
        }
        break;
    }
    case NODE_VAR_NAME:
    {
        std::cout << "Variable Name: " << astNode->data.varName->varName << std::endl;
        iNode->name = std::string(astNode->data.varName->varName);
        break;
    }
    case NODE_BINARY_EXPR:
    {
        std::cout << "Binary Expression: " << CryoOperatorToString(astNode->data.bin_op->op) << std::endl;
        iNode->name = CryoOperatorToString(astNode->data.bin_op->op);
        break;
    }
    case NODE_EXTERN_FUNCTION:
    {
        std::cout << "Extern Function: " << astNode->data.externFunction->name << std::endl;
        iNode->name = std::string(astNode->data.externFunction->name);
        break;
    }

    default:
        std::cout << "Unhandled node type: " << CryoNodeTypeToString(astNode->metaData->type) << std::endl;
        break;
    }

    return iNode;
}

VisualNode *createVisualTree(ASTNode *astNode, sf::Font &font, int depth)
{
    if (astNode == nullptr)
    {
        std::cout << std::string(depth * 2, ' ') << "Null node encountered" << std::endl;
        return nullptr;
    }

    std::cout << std::string(depth * 2, ' ') << "Processing node: " << CryoNodeTypeToString(astNode->metaData->type) << std::endl;

    VisualNode *visualNode = new VisualNode();
    visualNode->shape = sf::RectangleShape(sf::Vector2f(NODE_WIDTH, NODE_HEIGHT));
    visualNode->shape.setFillColor(sf::Color::White);
    visualNode->shape.setOutlineThickness(2);
    visualNode->shape.setOutlineColor(sf::Color::Black);

    iNode *info = createINode(astNode);

    std::string nodeText = getNodeTypeString(info->nodeType) + "\n";
    if (!info->name.empty())
        nodeText += "Name: " + info->name + "\n";
    if (info->dataType != DATA_TYPE_UNKNOWN)
    {
        char *dataType = CryoDataTypeToString(info->dataType);
        nodeText += "Type: " + std::string(dataType) + "\n";
    }
    if (!info->value.empty())
        nodeText += "Value: " + info->value;

    visualNode->text.setFont(font);
    visualNode->text.setCharacterSize(12);
    visualNode->text.setFillColor(sf::Color::Black);
    visualNode->text.setString(nodeText);

    // Traverse child nodes based on the node type
    switch (astNode->metaData->type)
    {
    case NODE_NAMESPACE:
        std::cout << std::string(depth * 2, ' ') << "Namespace: " << (astNode->data.cryoNamespace->name ? astNode->data.cryoNamespace->name : "unnamed") << std::endl;
        break;
    case NODE_PROGRAM:
        std::cout << std::string(depth * 2, ' ') << "Program with " << astNode->data.program->statementCount << " statements" << std::endl;
        for (size_t i = 0; i < astNode->data.program->statementCount; ++i)
        {
            VisualNode *childNode = createVisualTree(astNode->data.program->statements[i], font, depth + 1);
            if (childNode)
                visualNode->children.push_back(childNode);
        }
        break;
    case NODE_EXTERN_FUNCTION:
        std::cout << std::string(depth * 2, ' ') << "Extern Function: " << (astNode->data.externFunction->name ? astNode->data.externFunction->name : "unnamed") << std::endl;
        // Don't process children for extern functions to avoid potential loops
        break;
    case NODE_BLOCK:
    {
        for (int i = 0; i < astNode->data.block->statementCount; ++i)
        {
            VisualNode *childNode = createVisualTree(astNode->data.block->statements[i], font, depth + 1);
            if (childNode)
                visualNode->children.push_back(childNode);
        }
        break;
    }
    case NODE_FUNCTION_BLOCK:
    {
        for (int i = 0; i < astNode->data.functionBlock->statementCount; ++i)
        {
            VisualNode *childNode = createVisualTree(astNode->data.functionBlock->statements[i], font, depth + 1);
            if (childNode)
                visualNode->children.push_back(childNode);
        }
        break;
    }
    case NODE_FUNCTION_DECLARATION:
        std::cout << std::string(depth * 2, ' ') << "Function Declaration: " << (astNode->data.functionDecl->name ? astNode->data.functionDecl->name : "unnamed") << std::endl;
        for (int i = 0; i < astNode->data.functionDecl->paramCount; ++i)
        {
            VisualNode *paramNode = createVisualTree(astNode->data.functionDecl->params[i], font, depth + 1);
            if (paramNode)
                visualNode->children.push_back(paramNode);
        }
        if (astNode->data.functionDecl->body)
        {
            VisualNode *bodyNode = createVisualTree(astNode->data.functionDecl->body, font, depth + 1);
            if (bodyNode)
                visualNode->children.push_back(bodyNode);
        }
        break;
    case NODE_VAR_DECLARATION:
        std::cout << std::string(depth * 2, ' ') << "Variable Declaration: " << (astNode->data.varDecl->name ? astNode->data.varDecl->name : "unnamed") << std::endl;
        if (astNode->data.varDecl->initializer)
        {
            VisualNode *initNode = createVisualTree(astNode->data.varDecl->initializer, font, depth + 1);
            if (initNode)
                visualNode->children.push_back(initNode);
        }
        break;
    case NODE_BINARY_EXPR:
    {
        std::cout << std::string(depth * 2, ' ') << "Binary Expression: " << CryoOperatorToString(astNode->data.bin_op->op) << std::endl;
        VisualNode *leftNode = createVisualTree(astNode->data.bin_op->left, font, depth + 1);
        VisualNode *rightNode = createVisualTree(astNode->data.bin_op->right, font, depth + 1);
        if (leftNode)
            visualNode->children.push_back(leftNode);
        if (rightNode)
            visualNode->children.push_back(rightNode);

        break;
    }
    case NODE_UNARY_EXPR:
    {
        std::cout << std::string(depth * 2, ' ') << "Unary Expression: " << CryoTokenToString(astNode->data.unary_op->op) << std::endl;
        VisualNode *operandNode = createVisualTree(astNode->data.unary_op->operand, font, depth + 1);
        if (operandNode)
            visualNode->children.push_back(operandNode);
        break;
    }
    case NODE_IF_STATEMENT:
    {
        std::cout << std::string(depth * 2, ' ') << "If Statement" << std::endl;
        VisualNode *condNode = createVisualTree(astNode->data.ifStatement->condition, font, depth + 1);
        VisualNode *thenNode = createVisualTree(astNode->data.ifStatement->thenBranch, font, depth + 1);
        VisualNode *elseNode = createVisualTree(astNode->data.ifStatement->elseBranch, font, depth + 1);
        if (condNode)
            visualNode->children.push_back(condNode);
        if (thenNode)
            visualNode->children.push_back(thenNode);
        if (elseNode)
            visualNode->children.push_back(elseNode);

        break;
    }
    case NODE_FOR_STATEMENT:
    {
        std::cout << std::string(depth * 2, ' ') << "For Statement" << std::endl;
        VisualNode *initNode = createVisualTree(astNode->data.forStatement->initializer, font, depth + 1);
        VisualNode *condNode = createVisualTree(astNode->data.forStatement->condition, font, depth + 1);
        VisualNode *incrNode = createVisualTree(astNode->data.forStatement->increment, font, depth + 1);
        VisualNode *bodyNode = createVisualTree(astNode->data.forStatement->body, font, depth + 1);
        if (initNode)
            visualNode->children.push_back(initNode);
        if (condNode)
            visualNode->children.push_back(condNode);
        if (incrNode)
            visualNode->children.push_back(incrNode);
        if (bodyNode)
            visualNode->children.push_back(bodyNode);

        break;
    }
    case NODE_WHILE_STATEMENT:
    {
        std::cout << std::string(depth * 2, ' ') << "While Statement" << std::endl;
        VisualNode *condNode = createVisualTree(astNode->data.whileStatement->condition, font, depth + 1);
        VisualNode *bodyNode = createVisualTree(astNode->data.whileStatement->body, font, depth + 1);
        if (condNode)
            visualNode->children.push_back(condNode);
        if (bodyNode)
            visualNode->children.push_back(bodyNode);

        break;
    }
    case NODE_FUNCTION_CALL:
    {
        std::cout << std::string(depth * 2, ' ') << "Function Call: " << astNode->data.functionCall->name << std::endl;
        for (int i = 0; i < astNode->data.functionCall->argCount; ++i)
        {
            VisualNode *argNode = createVisualTree(astNode->data.functionCall->args[i], font, depth + 1);
            if (argNode)
                visualNode->children.push_back(argNode);
        }
        break;
    }
    case NODE_RETURN_STATEMENT:
    {
        std::cout << std::string(depth * 2, ' ') << "Return Statement" << std::endl;
        VisualNode *retNode = createVisualTree(astNode->data.returnStatement->returnValue, font, depth + 1);
        if (retNode)
            visualNode->children.push_back(retNode);
        break;
    }
    case NODE_ARRAY_LITERAL:
    {
        std::cout << std::string(depth * 2, ' ') << "Array Literal" << std::endl;
        for (int i = 0; i < astNode->data.array->elementCount; ++i)
        {
            VisualNode *elemNode = createVisualTree(astNode->data.array->elements[i], font, depth + 1);
            if (elemNode)
                visualNode->children.push_back(elemNode);
        }
        break;
    }
    case NODE_INDEX_EXPR:
    {
        std::cout << std::string(depth * 2, ' ') << "Index Expression" << std::endl;
        VisualNode *arrayNode = createVisualTree(astNode->data.indexExpr->array, font, depth + 1);
        VisualNode *indexNode = createVisualTree(astNode->data.indexExpr->index, font, depth + 1);
        if (arrayNode)
            visualNode->children.push_back(arrayNode);
        if (indexNode)
            visualNode->children.push_back(indexNode);
        break;
    }
    case NODE_VAR_REASSIGN:
    {
        std::cout << std::string(depth * 2, ' ') << "Variable Reassignment" << std::endl;
        VisualNode *varNode = createVisualTree(astNode->data.varReassignment->existingVarNode, font, depth + 1);
        VisualNode *exprNode = createVisualTree(astNode->data.varReassignment->newVarNode, font, depth + 1);
        if (varNode)
            visualNode->children.push_back(varNode);
        if (exprNode)
            visualNode->children.push_back(exprNode);
        break;
    }
    // Nodes that typically don't have children
    case NODE_LITERAL_EXPR:
    case NODE_VAR_NAME:
    case NODE_PARAM_LIST:
    case NODE_ARG_LIST:
    case NODE_TYPE:
    case NODE_STRING_LITERAL:
    case NODE_STRING_EXPRESSION:
    case NODE_BOOLEAN_LITERAL:
    case NODE_IMPORT_STATEMENT:
    case NODE_EXTERN_STATEMENT:
        // These nodes typically don't have children, so we don't need to process anything
        break;
    default:
        std::cout << "Unhandled node type in createVisualTree: " << getNodeTypeString(astNode->metaData->type) << std::endl;
        break;
    }

    delete info; // Clean up the iNode
    return visualNode;
}

void layoutTree(VisualNode *root, float x, float y, float availableWidth)
{
    root->x = x;
    root->y = y;
    root->shape.setPosition(x, y);

    sf::FloatRect textBounds = root->text.getLocalBounds();
    root->text.setPosition(
        x + (NODE_WIDTH - textBounds.width) / 2,
        y + (NODE_HEIGHT - textBounds.height) / 2 - 5 // Slight vertical adjustment
    );

    if (root->children.empty())
        return;

    float childrenWidth = availableWidth - HORIZONTAL_SPACING;
    float childWidth = childrenWidth / root->children.size();

    for (size_t i = 0; i < root->children.size(); ++i)
    {
        float childX = x - availableWidth / 2 + childWidth * i + childWidth / 2;
        layoutTree(root->children[i], childX, y + VERTICAL_SPACING, childWidth);
    }
}

void drawTree(sf::RenderWindow &window, VisualNode *root)
{
    std::queue<VisualNode *> queue;
    queue.push(root);

    while (!queue.empty())
    {
        VisualNode *node = queue.front();
        queue.pop();

        window.draw(node->shape);
        window.draw(node->text);

        for (VisualNode *child : node->children)
        {
            queue.push(child);
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(node->x + NODE_WIDTH / 2, node->y + NODE_HEIGHT), sf::Color::Black),
                sf::Vertex(sf::Vector2f(child->x + NODE_WIDTH / 2, child->y), sf::Color::Black)};
            window.draw(line, 2, sf::Lines);
        }
    }
}
void renderWindow(ASTNode *programNode)
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "AST Visualizer");

    sf::Font font;
    if (!font.loadFromFile("assets/Geist-Regular.ttf"))
    {
        std::cerr << "Failed to load font." << std::endl;
        return;
    }

    VisualNode *root = createVisualTree(programNode, font, 0);
    layoutTree(root, WINDOW_WIDTH / 2, 50, WINDOW_WIDTH - 100);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::White);
        drawTree(window, root);
        window.display();
    }

    // TODO: Implement cleanup function
    // cleanupVisualTree(root);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <path_to_file>\n", argv[0]);
        return 1;
    }

    const char *filePath = argv[1];
    char *source = readFile(filePath);
    if (source == nullptr)
    {
        fprintf(stderr, "Failed to read source file.\n");
        return 1;
    }

    ASTNode *programNode = getProgramNode(source);
    if (programNode == nullptr)
    {
        fprintf(stderr, "Failed to parse source code.\n");
        return 1;
    }

    renderWindow(programNode);
    return 0;
}
