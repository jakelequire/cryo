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

void initASTConsoleOutput(ASTNode *root, const char *filePath)
{
    DebugASTOutput *output = createDebugASTOutput(filePath, filePath, "txt", "console");
    createASTDebugView(root, output, 0);
    char *buffer = getASTBuffer(output, true);
    if (!buffer)
    {
        logMessage("ERROR", __LINE__, "AST", "Failed to get AST buffer for debug output");
        return;
    }
    printf("%s\n", buffer);
}

void logASTNodeDebugView(ASTNode *node)
{
    // Log the specific AST node in the formatted debug view (console view)
    DebugASTOutput *output = createDebugASTOutput("console", "console", "txt", "console");
    createASTDebugView(node, output, 0);
    char *buffer = logASTBuffer(output, true);
    if (!buffer)
    {
        logMessage("ERROR", __LINE__, "AST", "Failed to get AST buffer for debug output");
        return;
    }
    printf("%s\n", buffer);
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

ASTDebugNode *createASTDebugNode(const char *nodeType, const char *nodeName, CryoDataType dataType, int line, int column, int indent, ASTNode *sourceNode)
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
    node->sourceNode = sourceNode;
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
    char *buffer = getASTBuffer(output, false);
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

bool propHasDefault(PropertyNode *prop)
{
    if (prop->defaultProperty)
    {
        printf("Property %s has default value\n", prop->name);
        return true;
    }
    printf("Property %s does not have default value\n", prop->name);
    return false;
}

// # ============================================================ #
// # Output File Buffer                                           #
// # ============================================================ #

char *getASTBuffer(DebugASTOutput *output, bool console)
{
    char *buffer = MALLOC_AST_BUFFER;
    if (!buffer)
    {
        logMessage("ERROR", __LINE__, "AST", "Failed to allocate memory for AST buffer");
        return NULL;
    }

    // Start the buffer with the AST Tree header
    sprintf(buffer, GREEN "\n\n╔═══════════════════════════════ AST Tree ═══════════════════════════════╗" COLOR_RESET);

    for (int i = 0; i < output->nodeCount; i++)
    {
        ASTDebugNode *node = &output->nodes[i];
        char *formattedNode = formatASTNode(node, output, node->indent, console);
        if (formattedNode)
        {
            sprintf(buffer, "%s\n%s", buffer, formattedNode);
            free(formattedNode);
        }
    }
    sprintf(buffer, "%s\n\n", buffer);

    // End the buffer with the AST Tree footer
    sprintf(buffer, "%s" GREEN "╚════════════════════════════════════════════════════════════════════════╝\n\n" COLOR_RESET, buffer);

    return buffer;
}

char *logASTBuffer(DebugASTOutput *output, bool console)
{
    char *buffer = MALLOC_AST_BUFFER;
    if (!buffer)
    {
        logMessage("ERROR", __LINE__, "AST", "Failed to allocate memory for AST buffer");
        return NULL;
    }

    for (int i = 0; i < output->nodeCount; i++)
    {
        ASTDebugNode *node = &output->nodes[i];
        char *formattedNode = formatASTNode(node, output, node->indent, console);
        if (formattedNode)
        {
            sprintf(buffer, "%s\n%s", buffer, formattedNode);
            free(formattedNode);
        }
    }
    sprintf(buffer, "%s\n", buffer);

    return buffer;
}

// # ============================================================ #
// # Formatting Functions                                         #
// # ============================================================ #

char *formatASTNode(ASTDebugNode *node, DebugASTOutput *output, int indentLevel, bool console)
{
    const char *nodeType = node->nodeType;
    char *formattedNode = NULL;

    // Create indentation string
    char indent[128];
    memset(indent, 0, sizeof(indent));
    for (int i = 0; i < indentLevel; i++)
    {
        if (console)
        {
            char *coloredBar = (char *)malloc(sizeof(char) * 8);
            sprintf(coloredBar, LIGHT_MAGENTA BOLD "|    " COLOR_RESET);
            strcat(indent, coloredBar);
        }
        else
        {
            strcat(indent, "   ");
        }
    }
    if (indentLevel == 0)
    {
        strcat(indent, "\n");
    }

    if (strcmp(nodeType, "Program") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatProgramNode(node, output);
        }
        else
        {
            formattedNode = formatProgramNode(node, output);
        }
    }
    else if (strcmp(nodeType, "FunctionDecl") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatFunctionDeclNode(node, output);
        }
        else
        {
            formattedNode = formatFunctionDeclNode(node, output);
        }
    }
    else if (strcmp(nodeType, "ParamList") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatParamListNode(node, output);
        }
        else
        {
            formattedNode = formatParamListNode(node, output);
        }
    }
    else if (strcmp(nodeType, "Block") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatBlockNode(node, output);
        }
        else
        {
            formattedNode = formatBlockNode(node, output);
        }
    }
    else if (strcmp(nodeType, "VarDecl") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatVarDeclNode(node, output);
        }
        else
        {
            formattedNode = formatVarDeclNode(node, output);
        }
    }
    else if (strcmp(nodeType, "Expression") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatExpressionNode(node, output);
        }
        else
        {
            formattedNode = formatExpressionNode(node, output);
        }
    }
    else if (strcmp(nodeType, "LiteralExpr") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatLiteralExprNode(node, output);
        }
        else
        {
            formattedNode = formatLiteralExprNode(node, output);
        }
    }
    else if (strcmp(nodeType, "ReturnStatement") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatReturnStatementNode(node, output);
        }
        else
        {
            formattedNode = formatReturnStatementNode(node, output);
        }
    }
    else if (strcmp(nodeType, "FunctionCall") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatFunctionCallNode(node, output);
        }
        else
        {
            formattedNode = formatFunctionCallNode(node, output);
        }
    }
    else if (strcmp(nodeType, "Param") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatParamNode(node, output);
        }
        else
        {
            formattedNode = formatParamNode(node, output);
        }
    }
    else if (strcmp(nodeType, "Property") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatPropertyNode(node, output);
        }
        else
        {
            formattedNode = formatPropertyNode(node, output);
        }
    }
    else if (strcmp(nodeType, "VarName") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatVarNameNode(node, output);
        }
        else
        {
            formattedNode = formatVarNameNode(node, output);
        }
    }
    else if (strcmp(nodeType, "StructDecl") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatStructNode(node, output);
        }
        else
        {
            formattedNode = formatStructNode(node, output);
        }
    }
    else if (strcmp(nodeType, "ExternFunction") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatExternFunctionNode(node, output);
        }
        else
        {
            formattedNode = formatExternFunctionNode(node, output);
        }
    }
    else if (strcmp(nodeType, "FunctionBlock") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatFunctionBlock(node, output);
        }
        else
        {
            formattedNode = formatFunctionBlock(node, output);
        }
    }
    else if (strcmp(nodeType, "Namespace") == 0)
    {
        // Skip namespace nodes
        return NULL;
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

// ============================================================
// <ProgramNode>
char *formatProgramNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <Program : [NAMESPACE]> { FILE_NAME }
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<Program : [%s]> { %s }", node->namespaceName, output->fileName);
    return buffer;
}
char *CONSOLE_formatProgramNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <Program : [NAMESPACE]> { FILE_NAME }
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<Program : %s>%s %s{ %s }%s",
            BOLD, LIGHT_RED, node->namespaceName, COLOR_RESET,
            YELLOW, output->fileName, COLOR_RESET);
    return buffer;
}
// </ProgramNode>
// ============================================================
// ============================================================
// <FunctionDecl>
char *formatFunctionDeclNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <FunctionDecl> [NAME] [RETURN_TYPE] <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<FunctionDecl> [%s] { RetType: %s } <%i:%i>", node->nodeName, CryoDataTypeToString(node->dataType), node->line, node->column);
    return buffer;
}
char *CONSOLE_formatFunctionDeclNode(ASTDebugNode *node, DebugASTOutput *output)
{
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<FunctionDecl>%s %s[%s]%s %s%s{ %s }%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET, YELLOW, node->nodeName, COLOR_RESET,
            BOLD, LIGHT_CYAN, CryoDataTypeToString(node->dataType), COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);

    return buffer;
}
// </FunctionDecl>
// ============================================================
// ============================================================
// <ParamList>
char *formatParamListNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <ParamList> [NAME] [TYPE] <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<ParamList> <%i:%i>", node->line, node->column);
    return buffer;
}
char *CONSOLE_formatParamListNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <ParamList> [NAME] [TYPE] <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<ParamList>%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </ParamList>
// ============================================================
// ============================================================
// <Block>
char *formatBlockNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <Block> [L:C]
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<Block> <%i:%i>", node->line, node->column);
    return buffer;
}
char *CONSOLE_formatBlockNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <Block> [L:C]
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<Block>%s %s%s<%i:%i>%s",
            BOLD, LIGHT_GREEN, COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </Block>
// ============================================================
// ============================================================
// <VarDecl>
char *formatVarDeclNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <VarDecl> [NAME] [TYPE] <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<VarDecl> [%s] { Type: %s } <%i:%i>", node->nodeName, CryoDataTypeToString(node->dataType), node->line, node->column);
    return buffer;
}
char *CONSOLE_formatVarDeclNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <VarDecl> [NAME] [TYPE] <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<VarDecl>%s %s[%s]%s %s%s{ %s }%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            YELLOW, node->nodeName, COLOR_RESET,
            BOLD, LIGHT_CYAN, CryoDataTypeToString(node->dataType), COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </VarDecl>
// ============================================================
// ============================================================
// <Expression>
char *formatExpressionNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <Expression> <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<Expression> <%i:%i>", node->line, node->column);
    return buffer;
}
char *CONSOLE_formatExpressionNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <Expression> <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<Expression>%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </Expression>
// ============================================================
// ============================================================
// <LiteralExpr>
char *formatLiteralExprNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <LiteralExpr> <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<LiteralExpr> <%i:%i>", node->line, node->column);
    return buffer;
}
char *CONSOLE_formatLiteralExprNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <LiteralExpr> <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<LiteralExpr>%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </LiteralExpr>
// ============================================================
// ============================================================
// <ReturnStatement>
char *formatReturnStatementNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <ReturnStatement> <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<ReturnStatement> <%i:%i>", node->line, node->column);
    return buffer;
}
char *CONSOLE_formatReturnStatementNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <ReturnStatement> <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<ReturnStatement>%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </ReturnStatement>
// ============================================================
// ============================================================
// <FunctionCall>
char *formatFunctionCallNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <FunctionCall> <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<FunctionCall> <%i:%i>", node->line, node->column);
    return buffer;
}
char *CONSOLE_formatFunctionCallNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <FunctionCall> <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<FunctionCall>%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </FunctionCall>
// ============================================================
// ============================================================
// <Param>
char *formatParamNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <Param> <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<Param> { Type: %s } <%i:%i>", CryoDataTypeToString(node->dataType), node->line, node->column);
    return buffer;
}
char *CONSOLE_formatParamNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <Param> <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<Param>%s %s%s{ %s }%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            BOLD, LIGHT_CYAN, CryoDataTypeToString(node->dataType), COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </Param>
// ============================================================
// ============================================================
// <Property>
char *formatPropertyNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <Property> <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<Property> [%s] { Type: %s } <%i:%i>", node->nodeName, CryoDataTypeToString(node->dataType), node->line, node->column);
    return buffer;
}
char *CONSOLE_formatPropertyNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <Property> <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    const char *defaultString = BOLD LIGHT_BLUE ":"
                                                "Default" COLOR_RESET;
    const char *propFlag = propHasDefault(node->sourceNode->data.property) ? defaultString : "";

    sprintf(buffer, "%s%s<Property>%s%s%s [%s]%s %s%s{ %s }%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET, (char *)propFlag,
            YELLOW, node->nodeName, COLOR_RESET,
            BOLD, LIGHT_CYAN, CryoDataTypeToString(node->dataType), COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </Property>
// ============================================================
// ============================================================
// <VarName>
char *formatVarNameNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <VarName> [NAME] <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<VarName> [%s] <%i:%i>", node->nodeName, node->line, node->column);
    return buffer;
}
char *CONSOLE_formatVarNameNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <VarName> [NAME] <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<VarName>%s %s[%s]%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            YELLOW, node->nodeName, COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </VarName>
// ============================================================
// ============================================================
// <StructDecl>
char *formatStructNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <StructDecl> [NAME] <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<StructDecl> [%s] <%i:%i>", node->nodeName, node->line, node->column);
    return buffer;
}
char *CONSOLE_formatStructNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <StructDecl> [NAME] <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<StructDecl>%s %s[%s]%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            YELLOW, node->nodeName, COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </StructDecl>
// ============================================================
// ============================================================
// <ExternFunction>
char *formatExternFunctionNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <ExternFunction> [NAME] [RETURN_TYPE] <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<ExternFunction> [%s] { RetType: %s } <%i:%i>", node->nodeName, CryoDataTypeToString(node->dataType), node->line, node->column);
    return buffer;
}
char *CONSOLE_formatExternFunctionNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <ExternFunction> [NAME] [RETURN_TYPE] <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<ExternFunction>%s %s[%s]%s %s%s{ %s }%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            YELLOW, node->nodeName, COLOR_RESET,
            BOLD, CYAN, CryoDataTypeToString(node->dataType), COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </ExternFunction>
// ============================================================
// ============================================================
// <FunctionBlock>
char *formatFunctionBlock(ASTDebugNode *node, DebugASTOutput *output)
{
    // <FunctionBlock> <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<FunctionBlock> <%i:%i>", node->line, node->column);
    return buffer;
}
char *CONSOLE_formatFunctionBlock(ASTDebugNode *node, DebugASTOutput *output)
{
    // <FunctionBlock> <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH

    sprintf(buffer, "%s%s<FunctionBlock>%s %s%s<%i:%i>%s",
            BOLD, LIGHT_GREEN, COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </FunctionBlock>
// ============================================================

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
        ASTDebugNode *programNode = createASTDebugNode("Program", "Program", DATA_TYPE_VOID, line, column, indentLevel, node);
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
        ASTDebugNode *functionNode = createASTDebugNode("FunctionDecl", funcName, returnType, line, column, indentLevel, node);
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

        ASTDebugNode *externFuncNode = createASTDebugNode("ExternFunction", funcName, returnType, line, column, indentLevel, node);
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
        ASTDebugNode *blockNode = createASTDebugNode("Block", "Block", DATA_TYPE_VOID, line, column, indentLevel, node);
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
        ASTDebugNode *functionBlockNode = createASTDebugNode("FunctionBlock", "FunctionBlock", DATA_TYPE_VOID, line, column, indentLevel, node);
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
        ASTDebugNode *structNode = createASTDebugNode("StructDecl", structName, DATA_TYPE_VOID, line, column, indentLevel, node);
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
        ASTDebugNode *paramListNode = createASTDebugNode("ParamList", "ParamList", DATA_TYPE_VOID, line, column, indentLevel, node);
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
        ASTDebugNode *namespaceNode = createASTDebugNode("Namespace", namespaceName, DATA_TYPE_VOID, line, column, indentLevel, node);
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
        ASTDebugNode *varDeclNode = createASTDebugNode("VarDecl", varName, dataType, line, column, indentLevel, node);
        output->nodes[output->nodeCount] = *varDeclNode;
        output->nodeCount++;
        break;
    }

    case NODE_EXPRESSION:
    {
        __LINE_AND_COLUMN__
        ASTDebugNode *expressionNode = createASTDebugNode("Expression", "Expression", DATA_TYPE_VOID, line, column, indentLevel, node);
        output->nodes[output->nodeCount] = *expressionNode;
        output->nodeCount++;
        break;
    }

    case NODE_LITERAL_EXPR:
    {
        __LINE_AND_COLUMN__
        ASTDebugNode *literalNode = createASTDebugNode("LiteralExpr", "LiteralExpr", DATA_TYPE_VOID, line, column, indentLevel, node);
        output->nodes[output->nodeCount] = *literalNode;
        output->nodeCount++;
        break;
    }

    case NODE_RETURN_STATEMENT:
    {
        __LINE_AND_COLUMN__
        ASTDebugNode *returnNode = createASTDebugNode("ReturnStatement", "ReturnStatement", DATA_TYPE_VOID, line, column, indentLevel, node);
        output->nodes[output->nodeCount] = *returnNode;
        output->nodeCount++;
        break;
    }

    case NODE_FUNCTION_CALL:
    {
        __LINE_AND_COLUMN__
        char *funcName = strdup(node->data.functionCall->name);
        ASTDebugNode *functionCallNode = createASTDebugNode("FunctionCall", funcName, DATA_TYPE_VOID, line, column, indentLevel, node);
        output->nodes[output->nodeCount] = *functionCallNode;
        output->nodeCount++;
        break;
    }

    case NODE_PARAM:
    {
        __LINE_AND_COLUMN__
        char *paramName = strdup(node->data.param->name);
        CryoDataType paramType = node->data.param->type;

        ASTDebugNode *paramNode = createASTDebugNode("Param", paramName, paramType, line, column, indentLevel, node);
        paramNode->sourceNode = node;
        output->nodes[output->nodeCount] = *paramNode;
        output->nodeCount++;
        break;
    }

    case NODE_PROPERTY:
    {
        __LINE_AND_COLUMN__
        char *propertyName = strdup(node->data.property->name);
        CryoDataType propertyType = node->data.property->type;

        ASTDebugNode *propertyNode = createASTDebugNode("Property", propertyName, propertyType, line, column, indentLevel, node);
        output->nodes[output->nodeCount] = *propertyNode;
        output->nodeCount++;
        break;
    }

    case NODE_VAR_NAME:
    {
        __LINE_AND_COLUMN__
        char *varName = strdup(node->data.varName->varName);
        ASTDebugNode *varNameNode = createASTDebugNode("VarName", varName, DATA_TYPE_VOID, line, column, indentLevel, node);
        output->nodes[output->nodeCount] = *varNameNode;
        output->nodeCount++;
        break;
    }

    case NODE_ARG_LIST:
    {
        __LINE_AND_COLUMN__
        ASTDebugNode *argListNode = createASTDebugNode("ArgList", "ArgList", DATA_TYPE_VOID, line, column, indentLevel, node);
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
