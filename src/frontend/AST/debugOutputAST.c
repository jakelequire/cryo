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

#define AST_OUTPUT_EXT ".txt"
#define AST_OUTPUT_FILENAME "ast_debug"
#define AST_DEBUG_VIEW_NODE_COUNT 128
#define __LINE_AND_COLUMN__ \
    int line = line;        \
    int column = column;

#define BUFFER_FAILED_ALLOCA_CATCH                                                        \
    if (!buffer)                                                                          \
    {                                                                                     \
        logMessage("ERROR", __LINE__, "AST", "Failed to allocate memory for AST buffer"); \
        return NULL;                                                                      \
    }

#define BUFFER_CHAR_SIZE sizeof(char) * 1024
#define MALLOC_BUFFER (char *)malloc(BUFFER_CHAR_SIZE)
#define AST_BUFFER_SIZE 5012
#define MALLOC_AST_BUFFER (char *)malloc(sizeof(char) * AST_BUFFER_SIZE)

int initASTDebugOutput(ASTNode *root, CompilerSettings *settings)
{
    const char *fileExt = AST_OUTPUT_EXT;
    const char *cwd = settings->rootDir;

    const char *outDir = (char *)malloc(sizeof(char) * 1024);
    sprintf((char *)outDir, "%s/%s", cwd, "build/debug");

    DebugASTOutput *output = createDebugASTOutput(settings->inputFile, outDir, fileExt, cwd);
    createASTDebugView(root, output, 0);
    createASTDebugOutputFile(output);

    return 0;
}

DebugASTOutput *createDebugASTOutput(const char *fileName, const char *filePath, const char *fileExt, const char *cwd)
{
    DebugASTOutput *output = (DebugASTOutput *)malloc(sizeof(DebugASTOutput));
    output->short_fileName = strstr(fileName, "/");
    output->fileName = strdup(fileName);
    output->filePath = strdup(filePath);
    output->fileExt = strdup(fileExt);
    output->cwd = strdup(cwd);
    output->nodes = (ASTDebugNode *)malloc(sizeof(ASTDebugNode) * AST_DEBUG_VIEW_NODE_COUNT);
    output->nodeCount = 0;
    return output;
}

ASTDebugNode *createASTDebugNode(const char *nodeType, const char *nodeName, CryoDataType dataType, int line, int column, int indent)
{
    ASTDebugNode *node = (ASTDebugNode *)malloc(sizeof(ASTDebugNode));
    node->nodeType = nodeType;
    node->nodeName = nodeName;
    node->dataType = dataType;
    node->line = 0;
    node->column = 0;
    node->children = (ASTDebugNode *)malloc(sizeof(ASTDebugNode) * AST_DEBUG_VIEW_NODE_COUNT);
    node->childCount = 0;
    node->indent = indent;
    node->namespaceName = (const char *)malloc(sizeof(char) * 1024);
    return node;
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

void createASTDebugOutputFile(DebugASTOutput *output)
{
    char *buffer = getASTBuffer(output);
    if (!buffer)
    {
        logMessage("ERROR", __LINE__, "AST", "Failed to get AST buffer for debug output");
        return;
    }

    const char *fileName = AST_OUTPUT_FILENAME;
    const char *ext = AST_OUTPUT_EXT;
    const char *filePath = output->cwd;

    if (!filePath)
    {
        logMessage("ERROR", __LINE__, "AST", "Failed to get file path for AST debug output");
        return;
    }
    if (!dirExists(filePath))
    {
        logMessage("WARN", __LINE__, "AST", "Creating directory for AST debug output: %s", filePath);
        createDir(filePath);
    }

    const char *outputPath = (char *)malloc(sizeof(char) * 1024);
    sprintf((char *)outputPath, "%s", output->cwd);

    const char *outputFilePath = (char *)malloc(sizeof(char) * 1024);
    sprintf((char *)outputFilePath, "%s/%s%s", outputPath, fileName, ext);

    removePrevASTOutput(outputFilePath);

    FILE *file = fopen(outputFilePath, "w");
    if (!file)
    {
        logMessage("ERROR", __LINE__, "AST", "Failed to open file for AST debug output");
        return;
    }

    fprintf(file, "%s", buffer);
    fclose(file);
}

void removePrevASTOutput(const char *filePath)
{
    // Remove the previous output file
    if (fileExists(filePath))
    {
        removeFile(filePath);
    }

    return;
}

char *seekNamespaceName(ASTNode *node)
{
    if (!node)
        return NULL;

    for (int i = 0; i < node->data.program->statementCount; i++)
    {
        if (node->data.program->statements[i]->metaData->type == NODE_NAMESPACE)
        {
            return node->data.program->statements[i]->data.cryoNamespace->name;
        }
    }
}

// # ============================================================ #
// # Output File Buffer                                           #
// # ============================================================ #

char *getASTBuffer(DebugASTOutput *output)
{
    char *buffer = MALLOC_AST_BUFFER;
    if (!buffer)
    {
        logMessage("ERROR", __LINE__, "AST", "Failed to allocate memory for AST buffer");
        return NULL;
    }

    sprintf(buffer, "AST Debug Output for %s\n", output->fileName);

    for (int i = 0; i < output->nodeCount; i++)
    {
        ASTDebugNode *node = &output->nodes[i];
        char *formattedNode = formatASTNode(node, output, node->indent);
        if (formattedNode)
        {
            sprintf(buffer, "%s\n%s", buffer, formattedNode);
            free(formattedNode);
        }
    }
    sprintf(buffer, "%s\n\n", buffer);

    return buffer;
}

// # ============================================================ #
// # Formatting Functions                                         #
// # ============================================================ #

char *formatASTNode(ASTDebugNode *node, DebugASTOutput *output, int indentLevel)
{
    const char *nodeType = node->nodeType;
    char *formattedNode = NULL;

    // Create indentation string
    char indent[128];
    memset(indent, 0, sizeof(indent));
    for (int i = 0; i < indentLevel; i++)
    {
        strcat(indent, "|   ");
    }
    if (indentLevel == 0)
    {
        strcat(indent, "\n");
    }

    if (strcmp(nodeType, "Program") == 0)
    {
        formattedNode = formatProgramNode(node, output);
    }
    else if (strcmp(nodeType, "FunctionDecl") == 0)
    {
        formattedNode = formatFunctionDeclNode(node, output);
    }
    else if (strcmp(nodeType, "ParamList") == 0)
    {
        formattedNode = formatParamListNode(node, output);
    }
    else if (strcmp(nodeType, "Block") == 0)
    {
        formattedNode = formatBlockNode(node, output);
    }
    else if (strcmp(nodeType, "VarDecl") == 0)
    {
        formattedNode = formatVarDeclNode(node, output);
    }
    else if (strcmp(nodeType, "Expression") == 0)
    {
        formattedNode = formatExpressionNode(node, output);
    }
    else if (strcmp(nodeType, "LiteralExpr") == 0)
    {
        formattedNode = formatLiteralExprNode(node, output);
    }
    else if (strcmp(nodeType, "ReturnStatement") == 0)
    {
        formattedNode = formatReturnStatementNode(node, output);
    }
    else if (strcmp(nodeType, "FunctionCall") == 0)
    {
        formattedNode = formatFunctionCallNode(node, output);
    }
    else if (strcmp(nodeType, "Param") == 0)
    {
        formattedNode = formatParamNode(node, output);
    }
    else if (strcmp(nodeType, "Property") == 0)
    {
        formattedNode = formatPropertyNode(node, output);
    }
    else if (strcmp(nodeType, "VarName") == 0)
    {
        formattedNode = formatVarNameNode(node, output);
    }
    else if (strcmp(nodeType, "StructDecl") == 0)
    {
        formattedNode = formatStructNode(node, output);
    }
    else if (strcmp(nodeType, "ExternFunction") == 0)
    {
        formattedNode = formatExternFunctionNode(node, output);
    }
    else if (strcmp(nodeType, "FunctionBlock") == 0)
    {
        formattedNode = formatFunctionBlock(node, output);
    }
    else
    {
        logMessage("ERROR", __LINE__, "AST", "Unhandled node type: %s", nodeType);
        return NULL;
    }

    // Add indentation to the formatted node
    char *indentedNode = MALLOC_BUFFER;
    if (!indentedNode)
    {
        logMessage("ERROR", __LINE__, "AST", "Failed to allocate memory for indented node");
        return NULL;
    }
    sprintf(indentedNode, "%s%s", indent, formattedNode);
    free(formattedNode);

    return indentedNode;
}

char *formatProgramNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <Program : [NAMESPACE]> { FILE_NAME }
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<Program : [%s]> { %s }", node->namespaceName, output->fileName);

    return buffer;
}

char *formatFunctionDeclNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <FunctionDecl> [NAME] [RETURN_TYPE] <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<FunctionDecl> [%s] { RetType: %s } <%i:%i>", node->nodeName, CryoDataTypeToString(node->dataType), node->line, node->column);

    return buffer;
}

char *formatParamListNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <ParamList> [NAME] [TYPE] <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<ParamList> [%s] { Type: %s } <%i:%i>", node->nodeName, CryoDataTypeToString(node->dataType), node->line, node->column);

    return buffer;
}

char *formatBlockNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <Block> [L:C]
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<Block> <%i:%i>", node->line, node->column);

    return buffer;
}

char *formatVarDeclNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <VarDecl> [NAME] [TYPE] <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<VarDecl> [%s] { Type: %s } <%i:%i>", node->nodeName, CryoDataTypeToString(node->dataType), node->line, node->column);

    return buffer;
}

char *formatExpressionNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <Expression> <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<Expression> <%i:%i>", node->line, node->column);

    return buffer;
}

char *formatLiteralExprNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <LiteralExpr> <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<LiteralExpr> <%i:%i>", node->line, node->column);

    return buffer;
}

char *formatReturnStatementNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <ReturnStatement> <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<ReturnStatement> <%i:%i>", node->line, node->column);

    return buffer;
}

char *formatFunctionCallNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <FunctionCall> <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<FunctionCall> <%i:%i>", node->line, node->column);

    return buffer;
}

char *formatParamNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <Param> <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<Param> { Type: %s } <%i:%i>", node->dataType, node->line, node->column);

    return buffer;
}

char *formatPropertyNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <Property> <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<Property> [%s] { Type: %s } <%i:%i>", node->nodeName, CryoDataTypeToString(node->dataType), node->line, node->column);

    return buffer;
}

char *formatVarNameNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <VarName> [NAME] <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<VarName> [%s] <%i:%i>", node->nodeName, node->line, node->column);

    return buffer;
}

char *formatStructNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <StructDecl> [NAME] <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<StructDecl> [%s] <%i:%i>", node->nodeName, node->line, node->column);

    return buffer;
}

char *formatExternFunctionNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <ExternFunction> [NAME] [RETURN_TYPE] <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<ExternFunction> [%s] { RetType: %s } <%i:%i>", node->nodeName, CryoDataTypeToString(node->dataType), node->line, node->column);

    return buffer;
}

char *formatFunctionBlock(ASTDebugNode *node, DebugASTOutput *output)
{
    // <FunctionBlock> <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH

    sprintf(buffer, "<FunctionBlock> <%i:%i>", node->line, node->column);

    return buffer;
}

// # ============================================================ #
// # AST Tree Traversal                                           #
// # ============================================================ #

void createASTDebugView(ASTNode *node, DebugASTOutput *output, int indentLevel)
{
    if (output->nodeCount >= AST_DEBUG_VIEW_NODE_COUNT)
    {
        logMessage("ERROR", __LINE__, "AST", "Exceeded maximum node count for debug output");
        logMessage("ERROR", __LINE__, "AST", "Node Count: %d", output->nodeCount);
        return;
    }

    CryoNodeType nodeType = node->metaData->type;

    switch (nodeType)
    {
    case NODE_PROGRAM:
    {
        __LINE_AND_COLUMN__
        ASTDebugNode *programNode = createASTDebugNode("Program", "Program", DATA_TYPE_VOID, line, column, indentLevel);
        char *namespaceName = seekNamespaceName(node);
        programNode->namespaceName = (const char *)namespaceName;
        output->nodes[output->nodeCount] = *programNode;
        output->nodeCount++;
        for (int i = 0; i < node->data.program->statementCount; i++)
        {
            createASTDebugView(node->data.program->statements[i], output, indentLevel);
        }
        break;
    }

    case NODE_FUNCTION_DECLARATION:
    {
        __LINE_AND_COLUMN__
        const char *funcName = strdup(node->data.functionDecl->name);
        CryoDataType returnType = node->data.functionDecl->returnType;
        ASTDebugNode *functionNode = createASTDebugNode("FunctionDecl", funcName, returnType, line, column, indentLevel);
        output->nodes[output->nodeCount] = *functionNode;
        output->nodeCount++;
        for (int i = 0; i < node->data.functionDecl->paramCount; i++)
        {
            if (node->data.functionDecl->paramCount == 0)
                break;
            createASTDebugView(node->data.functionDecl->params[i], output, indentLevel);
            indentLevel--;
        }

        indentLevel++;
        createASTDebugView(node->data.functionDecl->body, output, indentLevel);
        indentLevel--;
        break;
    }

    case NODE_EXTERN_FUNCTION:
    {
        __LINE_AND_COLUMN__
        char *funcName = strdup(node->data.externFunction->name);
        CryoDataType returnType = node->data.externFunction->returnType;

        ASTDebugNode *externFuncNode = createASTDebugNode("ExternFunction", funcName, returnType, line, column, indentLevel);
        output->nodes[output->nodeCount] = *externFuncNode;
        output->nodeCount++;
        for (int i = 0; i < node->data.externFunction->paramCount; i++)
        {
            if (node->data.externFunction->paramCount == 0)
                break;
            indentLevel++;
            createASTDebugView(node->data.externFunction->params[i], output, indentLevel);
            indentLevel--;
        }
        break;
    }

    case NODE_BLOCK:
    {
        __LINE_AND_COLUMN__
        ASTDebugNode *blockNode = createASTDebugNode("Block", "Block", DATA_TYPE_VOID, line, column, indentLevel);
        output->nodes[output->nodeCount] = *blockNode;
        output->nodeCount++;
        for (int i = 0; i < node->data.block->statementCount; i++)
        {
            indentLevel++;
            createASTDebugView(node->data.block->statements[i], output, indentLevel);
            indentLevel--;
        }
        break;
    }

    case NODE_FUNCTION_BLOCK:
    {
        __LINE_AND_COLUMN__
        ASTDebugNode *functionBlockNode = createASTDebugNode("FunctionBlock", "FunctionBlock", DATA_TYPE_VOID, line, column, indentLevel);
        output->nodes[output->nodeCount] = *functionBlockNode;
        output->nodeCount++;
        for (int i = 0; i < node->data.functionBlock->statementCount; i++)
        {
            indentLevel++;
            createASTDebugView(node->data.functionBlock->statements[i], output, indentLevel);
            indentLevel--;
        }
        break;
    }

    case NODE_STRUCT_DECLARATION:
    {
        __LINE_AND_COLUMN__
        char *structName = strdup(node->data.structNode->name);
        ASTDebugNode *structNode = createASTDebugNode("StructDecl", structName, DATA_TYPE_VOID, line, column, indentLevel);
        output->nodes[output->nodeCount] = *structNode;
        output->nodeCount++;
        for (int i = 0; i < node->data.structNode->propertyCount; i++)
        {
            indentLevel++;
            createASTDebugView(node->data.structNode->properties[i], output, indentLevel);
            indentLevel--;
        }
        break;
    }

    case NODE_PARAM_LIST:
    {
        __LINE_AND_COLUMN__
        ASTDebugNode *paramListNode = createASTDebugNode("ParamList", "ParamList", DATA_TYPE_VOID, line, column, indentLevel);
        output->nodes[output->nodeCount] = *paramListNode;
        output->nodeCount++;
        for (int i = 0; i < node->data.paramList->paramCount; i++)
        {
            indentLevel++;
            createASTDebugView(node, output, indentLevel);
            indentLevel--;
        }
        break;
    }

    case NODE_NAMESPACE:
    {
        __LINE_AND_COLUMN__
        char *namespaceName = strdup(node->data.cryoNamespace->name);
        ASTDebugNode *namespaceNode = createASTDebugNode("Namespace", namespaceName, DATA_TYPE_VOID, line, column, indentLevel);
        output->nodes[output->nodeCount] = *namespaceNode;
        output->nodeCount++;
        namespaceNode->indent = indentLevel;
        break;
    }

    case NODE_VAR_DECLARATION:
    {
        __LINE_AND_COLUMN__
        char *varName = strdup(node->data.varDecl->name);
        CryoDataType dataType = node->data.varDecl->type;
        ASTDebugNode *varDeclNode = createASTDebugNode("VarDecl", varName, dataType, line, column, indentLevel);
        output->nodes[output->nodeCount] = *varDeclNode;
        output->nodeCount++;
        break;
    }

    case NODE_EXPRESSION:
    {
        __LINE_AND_COLUMN__
        ASTDebugNode *expressionNode = createASTDebugNode("Expression", "Expression", DATA_TYPE_VOID, line, column, indentLevel);
        output->nodes[output->nodeCount] = *expressionNode;
        output->nodeCount++;
        break;
    }

    case NODE_LITERAL_EXPR:
    {
        __LINE_AND_COLUMN__
        ASTDebugNode *literalNode = createASTDebugNode("LiteralExpr", "LiteralExpr", DATA_TYPE_VOID, line, column, indentLevel);
        output->nodes[output->nodeCount] = *literalNode;
        output->nodeCount++;
        break;
    }

    case NODE_RETURN_STATEMENT:
    {
        __LINE_AND_COLUMN__
        ASTDebugNode *returnNode = createASTDebugNode("ReturnStatement", "ReturnStatement", DATA_TYPE_VOID, line, column, indentLevel);
        output->nodes[output->nodeCount] = *returnNode;
        output->nodeCount++;
        break;
    }

    case NODE_FUNCTION_CALL:
    {
        __LINE_AND_COLUMN__
        char *funcName = strdup(node->data.functionCall->name);
        ASTDebugNode *functionCallNode = createASTDebugNode("FunctionCall", funcName, DATA_TYPE_VOID, line, column, indentLevel);
        output->nodes[output->nodeCount] = *functionCallNode;
        output->nodeCount++;
        break;
    }

    case NODE_PARAM:
    {
        __LINE_AND_COLUMN__
        char *paramName = strdup(node->data.param->name);
        CryoDataType paramType = node->data.param->type;

        ASTDebugNode *paramNode = createASTDebugNode("Param", paramName, paramType, line, column, indentLevel);
        output->nodes[output->nodeCount] = *paramNode;
        output->nodeCount++;
        break;
    }

    case NODE_PROPERTY:
    {
        __LINE_AND_COLUMN__
        char *propertyName = strdup(node->data.property->name);
        CryoDataType propertyType = node->data.property->type;

        ASTDebugNode *propertyNode = createASTDebugNode("Property", propertyName, propertyType, line, column, indentLevel);
        output->nodes[output->nodeCount] = *propertyNode;
        output->nodeCount++;
        break;
    }

    case NODE_VAR_NAME:
    {
        __LINE_AND_COLUMN__
        char *varName = strdup(node->data.varName->varName);
        ASTDebugNode *varNameNode = createASTDebugNode("VarName", varName, DATA_TYPE_VOID, line, column, indentLevel);
        output->nodes[output->nodeCount] = *varNameNode;
        output->nodeCount++;
        break;
    }

    case NODE_ARG_LIST:
    {
        __LINE_AND_COLUMN__
        ASTDebugNode *argListNode = createASTDebugNode("ArgList", "ArgList", DATA_TYPE_VOID, line, column, indentLevel);
        output->nodes[output->nodeCount] = *argListNode;
        output->nodeCount++;
        break;
    }

    default:
    {
        printf("Unknown Node Type @debugOutputAST.c | Node Type: %s\n", CryoNodeTypeToString(nodeType));
        break;
    }
    }

    return;
}
