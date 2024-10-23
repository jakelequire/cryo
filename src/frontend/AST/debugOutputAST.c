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
#include "frontend/AST.h"

/*
Rough Draft of AST Debug Output:

<Program : [NAMESPACE]> { FILE_NAME }
<FunctionDecl> [NAME] [RETURN_TYPE] <L:C>
|-> <ParamList> [NAME] [TYPE] <L:C>
|-> <Block> [L:C]
|   |-> <VarDecl> [NAME] [TYPE] <L:C>
|   |-> <VarDecl> [NAME] [TYPE] <L:C>
*/

#define AST_DEBUG_VIEW_NODE_COUNT 128

DebugASTOutput *createDebugASTOutput(const char *fileName, const char *filePath, const char *fileExt, const char *cwd)
{
    DebugASTOutput *output = (DebugASTOutput *)malloc(sizeof(DebugASTOutput));
    output->fileName = strdup(fileName);
    output->filePath = strdup(filePath);
    output->fileExt = strdup(fileExt);
    output->cwd = strdup(cwd);
    output->nodes = (ASTDebugNode *)malloc(sizeof(ASTDebugNode) * AST_DEBUG_VIEW_NODE_COUNT);
    return output;
}

ASTDebugNode *createASTDebugNode(const char *nodeType, const char *nodeName, CryoDataType dataType, int line, int column)
{
    ASTDebugNode *node = (ASTDebugNode *)malloc(sizeof(ASTDebugNode));
    node->nodeType = nodeType;
    node->nodeName = nodeName;
    node->dataType = dataType;
    node->line = 0;
    node->column = 0;
    node->children = (ASTDebugNode *)malloc(sizeof(ASTDebugNode) * AST_DEBUG_VIEW_NODE_COUNT);
    node->childCount = 0;
    node->indent = 0;
    return node;
}

int initASTDebugOutput(ASTNode *root, CompilerSettings *settings)
{
    const char *fileExt = ".txt";
    const char *cwd = settings->rootDir;

    DebugASTOutput *output = createDebugASTOutput(settings->inputFile, settings->inputFilePath, fileExt, cwd);
    createASTDebugView(root, output);
    return 0;
}

void createASTDebugView(ASTNode *node, DebugASTOutput *output)
{
    if (output->nodeCount >= AST_DEBUG_VIEW_NODE_COUNT)
    {
        logMessage("ERROR", __LINE__, "AST", "Exceeded maximum node count for debug output");
        logMessage("ERROR", __LINE__, "AST", "Node Count: %d", output->nodeCount);
        return;
    }

    int indent = 0;

    CryoNodeType nodeType = node->metaData->type;

    switch (nodeType)
    {
    case NODE_PROGRAM:
    {
        ASTDebugNode *programNode = createASTDebugNode("Program", "Program", DATA_TYPE_VOID, node->metaData->line, node->metaData->column);
        output->nodes[output->nodeCount] = *programNode;
        output->nodeCount++;
        for (int i = 0; i < node->data.program->statementCount; i++)
        {
            createASTDebugView(node->data.program->statements[i], output);
        }
        break;
    }

    case NODE_NAMESPACE:
    {
        ASTDebugNode *namespaceNode = createASTDebugNode("Namespace", "Namespace", DATA_TYPE_VOID, node->metaData->line, node->metaData->column);
        output->nodes[output->nodeCount] = *namespaceNode;
        output->nodeCount++;
        break;
    }

    case NODE_FUNCTION_DECLARATION:
    {
        ASTDebugNode *functionNode = createASTDebugNode("FunctionDecl", "FunctionDecl", DATA_TYPE_VOID, node->metaData->line, node->metaData->column);
        output->nodes[output->nodeCount] = *functionNode;
        output->nodeCount++;
        for (int i = 0; i < node->data.functionDecl->paramCount; i++)
        {
            createASTDebugView(node->data.functionDecl->params[i], output);
        }
        createASTDebugView(node->data.functionDecl->body, output);
        break;
    }

    case NODE_VAR_DECLARATION:
    {
        ASTDebugNode *varDeclNode = createASTDebugNode("VarDecl", "VarDecl", DATA_TYPE_VOID, node->metaData->line, node->metaData->column);
        output->nodes[output->nodeCount] = *varDeclNode;
        output->nodeCount++;
        break;
    }

    case NODE_EXPRESSION:
    {
        ASTDebugNode *expressionNode = createASTDebugNode("Expression", "Expression", DATA_TYPE_VOID, node->metaData->line, node->metaData->column);
        output->nodes[output->nodeCount] = *expressionNode;
        output->nodeCount++;
        break;
    }

    case NODE_LITERAL_EXPR:
    {
        ASTDebugNode *literalNode = createASTDebugNode("LiteralExpr", "LiteralExpr", DATA_TYPE_VOID, node->metaData->line, node->metaData->column);
        output->nodes[output->nodeCount] = *literalNode;
        output->nodeCount++;
        break;
    }

    case NODE_EXTERN_FUNCTION:
    {
        ASTDebugNode *externFuncNode = createASTDebugNode("ExternFunction", "ExternFunction", DATA_TYPE_VOID, node->metaData->line, node->metaData->column);
        output->nodes[output->nodeCount] = *externFuncNode;
        output->nodeCount++;
        for (int i = 0; i < node->data.externFunction->paramCount; i++)
        {
            createASTDebugView(node->data.externFunction->params[i], output);
        }
        break;
    }

    case NODE_BLOCK:
    {
        ASTDebugNode *blockNode = createASTDebugNode("Block", "Block", DATA_TYPE_VOID, node->metaData->line, node->metaData->column);
        output->nodes[output->nodeCount] = *blockNode;
        output->nodeCount++;
        for (int i = 0; i < node->data.block->statementCount; i++)
        {
            createASTDebugView(node->data.block->statements[i], output);
        }
        break;
    }

    case NODE_RETURN_STATEMENT:
    {
        ASTDebugNode *returnNode = createASTDebugNode("ReturnStatement", "ReturnStatement", DATA_TYPE_VOID, node->metaData->line, node->metaData->column);
        output->nodes[output->nodeCount] = *returnNode;
        output->nodeCount++;
        break;
    }

    case NODE_FUNCTION_CALL:
    {
        ASTDebugNode *functionCallNode = createASTDebugNode("FunctionCall", "FunctionCall", DATA_TYPE_VOID, node->metaData->line, node->metaData->column);
        output->nodes[output->nodeCount] = *functionCallNode;
        output->nodeCount++;
        break;
    }

    case NODE_PARAM_LIST:
    {
        ASTDebugNode *paramListNode = createASTDebugNode("ParamList", "ParamList", DATA_TYPE_VOID, node->metaData->line, node->metaData->column);
        output->nodes[output->nodeCount] = *paramListNode;
        output->nodeCount++;
        for (int i = 0; i < node->data.paramList->paramCount; i++)
        {
            createASTDebugView(node, output);
        }
        break;
    }

    default:
    {
        printf("Unknown Node Type @debugOutputAST.c | Node Type: %s\n", CryoNodeTypeToString(nodeType));
        break;
    }
    }
}

DebugASTOutput *addDebugNodesToOutput(ASTDebugNode *node, DebugASTOutput *output)
{
    if (!node)
        return output;

    output->nodes[output->nodeCount] = *node;
    output->nodeCount++;

    for (int i = 0; i < node->childCount; i++)
    {
        addDebugNodesToOutput(&node->children[i], output);
    }

    return output;
}

char *getASTBuffer(DebugASTOutput *output)
{
    char *buffer = (char *)malloc(sizeof(char) * 1024);
    if (!buffer)
    {
        logMessage("ERROR", __LINE__, "AST", "Failed to allocate memory for AST buffer");
        return NULL;
    }

    sprintf(buffer, "AST Debug Output for %s\n", output->fileName);

    for (int i = 0; i < output->nodeCount; i++)
    {
        ASTDebugNode node = output->nodes[i];
        sprintf(buffer, "%s\n%s\n", buffer, node.nodeType);
    }

    return buffer;
}

void createASTDebugOutputFile(DebugASTOutput *output)
{
    char *buffer = getASTBuffer(output);
    if (!buffer)
    {
        logMessage("ERROR", __LINE__, "AST", "Failed to create AST debug output buffer");
        return;
    }

    const char *fileName = "ast_debug";
    const char *ext = ".txt";
    const char *filePath = output->filePath;

    if (!filePath)
    {
        logMessage("ERROR", __LINE__, "AST", "Failed to get file path for AST debug output");
        return;
    }
    if (!dirExists(filePath))
    {
        logMessage("WARN", __LINE__, "AST", "Creating directory for AST debug output");
        createDir(filePath);
    }

    const char *outputPath = (char *)malloc(sizeof(char) * 1024);
    sprintf((char *)outputPath, "%s", output->cwd);

    const char *outputFilePath = (char *)malloc(sizeof(char) * 1024);
    sprintf((char *)outputFilePath, "%s/%s%s", outputPath, fileName, ext);

    FILE *file = fopen(outputFilePath, "w");
    if (!file)
    {
        logMessage("ERROR", __LINE__, "AST", "Failed to open file for AST debug output");
        return;
    }

    fprintf(file, "%s", buffer);
    fclose(file);
}
