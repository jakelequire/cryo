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
#include "tools/logger/logger_config.h"

#define AST_OUTPUT_EXT ".txt"
#define AST_OUTPUT_FILENAME "ast_debug"
#define AST_DEBUG_VIEW_NODE_COUNT 1024 * 32
#define __LINE_AND_COLUMN__          \
    int line = node->metaData->line; \
    int column = node->metaData->column;

#define BUFFER_FAILED_ALLOCA_CATCH                                                   \
    if (!buffer)                                                                     \
    {                                                                                \
        logMessage(LMI, "ERROR", "AST", "Failed to allocate memory for AST buffer"); \
        return NULL;                                                                 \
    }

#define BUFFER_CHAR_SIZE sizeof(char) * 10512 * 32
#define MALLOC_BUFFER (char *)malloc(BUFFER_CHAR_SIZE)
#define AST_BUFFER_SIZE 10512 * 32
#define MALLOC_AST_BUFFER (char *)malloc(sizeof(char) * AST_BUFFER_SIZE)

int initASTDebugOutput(ASTNode *root, CompilerSettings *settings)
{
    const char *fileExt = AST_OUTPUT_EXT;
    const char *cwd = settings->rootDir;

    char *outDir = (char *)malloc(sizeof(char) * BUFFER_CHAR_SIZE);
    sprintf(outDir, "%s/%s", cwd, "build/debug");

    DebugASTOutput *output = createDebugASTOutput("Debug Output", outDir, fileExt, cwd);
    createASTDebugView(root, output, 0);
    createASTDebugOutputFile(output);

    return 0;
}

void initASTConsoleOutput(ASTNode *root, const char *filePath)
{
    printf("Creating AST debug output for console\n");
    DebugASTOutput *output = createDebugASTOutput(filePath, filePath, "txt", "console");
    createASTDebugView(root, output, 0);
    char *buffer = getASTBuffer(output, true);
    if (!buffer)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to get AST buffer for debug output");
        return;
    }
    logMessage(LMI, "INFO", "AST", "AST debug output created for console");
    sprintf(buffer, "%s\n", buffer);
}

void logASTNodeDebugView(ASTNode *node)
{
    // Log the specific AST node in the formatted debug view (console view)
    DebugASTOutput *output = createDebugASTOutput("console", "console", "txt", "console");
    createASTDebugView(node, output, 0);
    char *buffer = logASTBuffer(output, true);
    if (!buffer)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to get AST buffer for debug output");
        return;
    }
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

ASTDebugNode *createASTDebugNode(const char *nodeType, const char *nodeName, DataType *dataType, int line, int column, int indent, ASTNode *sourceNode)
{
    ASTDebugNode *node = (ASTDebugNode *)malloc(sizeof(ASTDebugNode));
    node->nodeType = nodeType;
    node->nodeName = nodeName;
    node->dataType = dataType;
    node->line = line;
    node->column = column;
    node->children = (ASTDebugNode *)malloc(sizeof(ASTDebugNode) * AST_DEBUG_VIEW_NODE_COUNT);
    node->childCount = 0;
    node->indent = indent;
    node->namespaceName = (const char *)malloc(sizeof(char) * BUFFER_CHAR_SIZE);
    node->sourceNode = sourceNode;
    node->value = NULL;
    node->args = NULL;
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
    // char *buffer = getASTBuffer(output, false);
    // if (!buffer)
    //{
    //     logMessage(LMI, "ERROR", "AST", "Failed to get AST buffer for debug output");
    //     return;
    // }
    //
    // const char *fileName = AST_OUTPUT_FILENAME;
    // const char *ext = AST_OUTPUT_EXT;
    // const char *filePath = output->cwd;
    //
    // if (!filePath)
    //{
    //    logMessage(LMI, "ERROR", "AST", "Failed to get file path for AST debug output");
    //    return;
    //}
    // if (!dirExists(filePath))
    //{
    //    logMessage(LMI, "ERROR", "AST", "Directory does not exist for AST debug output");
    //    createDir(filePath);
    //}
    //
    // char *outputPath = (char *)malloc(sizeof(char) * 1024);
    // sprintf(outputPath, "%s", output->cwd);
    //
    // char *outputFilePath = (char *)malloc(sizeof(char) * 1024);
    // sprintf(outputFilePath, "%s/%s%s", outputPath, fileName, ext);
    //
    // removePrevASTOutput(outputFilePath);
    //
    // FILE *file = fopen(outputFilePath, "w");
    // if (!file)
    //{
    //    logMessage(LMI, "ERROR", "AST", "Failed to open file for AST debug output");
    //    return;
    //}
    //
    // fprintf(file, "%s", buffer);
    // fclose(file);
    return;
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
    DEBUG_PRINT_FILTER({
        printf("Creating AST buffer\n");
        char *buffer = (char *)malloc(sizeof(char) * AST_BUFFER_SIZE);
        if (!buffer)
        {
            logMessage(LMI, "ERROR", "AST", "Failed to allocate memory for AST buffer");
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
    });

    return NULL;
}

char *logASTBuffer(DebugASTOutput *output, bool console)
{
    DEBUG_PRINT_FILTER({
        char *buffer = (char *)malloc(sizeof(char) * AST_BUFFER_SIZE);
        if (!buffer)
        {
            logMessage(LMI, "ERROR", "AST", "Failed to allocate memory for AST buffer");
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
        printf("%s\n", buffer);

        return buffer;
    });

    return NULL;
}

void logASTNode(ASTNode *node)
{
    DEBUG_PRINT_FILTER({
        if (!node)
        {
            logMessage(LMI, "ERROR", "AST", "Node is null in logASTNode");
            return;
        }

        DebugASTOutput *output = createDebugASTOutput("console", "console", "txt", "console");
        createASTDebugView(node, output, 0);
        char *buffer = logASTBuffer(output, true);
        if (!buffer)
        {
            logMessage(LMI, "ERROR", "AST", "Failed to get AST buffer for debug output");
            return;
        }
    });
}

// # ============================================================ #
// # Formatting Functions                                         #
// # ============================================================ #

char *formatASTNode(ASTDebugNode *node, DebugASTOutput *output, int indentLevel, bool console)
{
    const char *nodeType = node->nodeType;
    char *formattedNode = (char *)malloc(sizeof(char) * BUFFER_CHAR_SIZE);
    if (!formattedNode)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to allocate memory for formatted node");
        return NULL;
    }

    // Create indentation string
    char indent[1024];
    memset(indent, 0, sizeof(indent));
    for (int i = 0; i < indentLevel; i++)
    {
        if (console)
        {
            char *coloredBar = (char *)malloc(sizeof(char) * BUFFER_CHAR_SIZE);
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
    else if (strcmp(nodeType, "StructConstructor") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatStructConstructor(node, output);
        }
        else
        {
            formattedNode = formatStructConstructor(node, output);
        }
    }
    else if (strcmp(nodeType, "This") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatThisNode(node, output);
        }
        else
        {
            formattedNode = formatThisNode(node, output);
        }
    }
    else if (strcmp(nodeType, "ThisAssign") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatThisAssignmentNode(node, output);
        }
        else
        {
            formattedNode = formatThisAssignmentNode(node, output);
        }
    }
    else if (strcmp(nodeType, "PropertyReassign") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatPropertyAssignmentNode(node, output);
        }
        else
        {
            formattedNode = formatPropertyAssignmentNode(node, output);
        }
    }
    else if (strcmp(nodeType, "Method") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatMethodNode(node, output);
        }
        else
        {
            formattedNode = formatMethodNode(node, output);
        }
    }
    else if (strcmp(nodeType, "IntLiteral") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatIntLiteralNode(node, output);
        }
        else
        {
            formattedNode = formatIntLiteralNode(node, output);
        }
    }
    else if (strcmp(nodeType, "StringLiteral") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatStringLiteralNode(node, output);
        }
        else
        {
            formattedNode = formatStringLiteralNode(node, output);
        }
    }
    else if (strcmp(nodeType, "BooleanLiteral") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatBooleanLiteralNode(node, output);
        }
        else
        {
            formattedNode = formatBooleanLiteralNode(node, output);
        }
    }
    else if (strcmp(nodeType, "MethodCall") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatMethodCallNode(node, output);
        }
        else
        {
            formattedNode = formatMethodCallNode(node, output);
        }
    }
    else if (strcmp(nodeType, "BinOp") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatBinOpNode(node, output);
        }
        else
        {
            formattedNode = formatBinOpNode(node, output);
        }
    }
    else if (strcmp(nodeType, "Class") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatClassNode(node, output);
        }
        else
        {
            formattedNode = formatClassNode(node, output);
        }
    }
    else if (strcmp(nodeType, "AccessControl") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatAccessControlNode(node, output);
        }
        else
        {
            formattedNode = formatAccessControlNode(node, output);
        }
    }
    else if (strcmp(nodeType, "ArgList") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatArgListNode(node, output);
        }
        else
        {
            formattedNode = formatArgListNode(node, output);
        }
    }
    else if (strcmp(nodeType, "ObjectInst") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatObjectInstNode(node, output);
        }
        else
        {
            formattedNode = formatObjectInstNode(node, output);
        }
    }
    else if (strcmp(nodeType, "UnaryOp") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatUnaryOpNode(node, output);
        }
        else
        {
            formattedNode = formatUnaryOpNode(node, output);
        }
    }
    else if (strcmp(nodeType, "Type") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatTypeNode(node, output);
        }
        else
        {
            formattedNode = formatTypeNode(node, output);
        }
    }
    else if (strcmp(nodeType, "Module") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatModuleNode(node, output);
        }
        else
        {
            formattedNode = formatModuleNode(node, output);
        }
    }
    else if (strcmp(nodeType, "Implementation") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatImplementationNode(node, output);
        }
        else
        {
            formattedNode = formatImplementationNode(node, output);
        }
    }
    else if (strcmp(nodeType, "PropertyAccess") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatPropertyAccessNode(node, output);
        }
        else
        {
            formattedNode = formatPropertyAccessNode(node, output);
        }
    }
    else if (strcmp(nodeType, "IfStatement") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatIfStatementNode(node, output);
        }
        else
        {
            formattedNode = formatIfStatementNode(node, output);
        }
    }
    else if (strcmp(nodeType, "NullLiteral") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatNullLiteralNode(node, output);
        }
        else
        {
            formattedNode = formatNullLiteralNode(node, output);
        }
    }
    else if (strcmp(nodeType, "Operator") == 0)
    {
        if (console)
        {
            formattedNode = CONSOLE_formatOperatorNode(node, output);
        }
        else
        {
            formattedNode = formatOperatorNode(node, output);
        }
    }
    else if (strcmp(nodeType, "Namespace") == 0)
    {
        // Skip namespace nodes
        return NULL;
    }

    else
    {
        logMessage(LMI, "ERROR", "AST", "Unhandled node type: %s", nodeType);
        return NULL;
    }

    // Add indentation to the formatted node
    char *indentedNode = MALLOC_BUFFER;
    if (!indentedNode)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to allocate memory for indented node");
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
    sprintf(buffer, "<FunctionDecl> [%s]: %s <%i:%i>",
            node->nodeName,
            DTM->debug->dataTypeToString(node->dataType),
            node->line, node->column);
    return buffer;
}
char *CONSOLE_formatFunctionDeclNode(ASTDebugNode *node, DebugASTOutput *output)
{
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<FunctionDecl>%s %s[%s]:%s %s%s%s%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET, YELLOW, node->nodeName, COLOR_RESET,
            BOLD, LIGHT_CYAN, DTM->debug->dataTypeToString(node->dataType), COLOR_RESET,
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
    sprintf(buffer, "<VarDecl> [%s] { Type: %s } <%i:%i>",
            node->nodeName,
            DTM->debug->dataTypeToString(node->dataType),
            node->line, node->column);

    if (node->value)
    {
        char *valueBuffer = ASTNodeValueBuffer(node->value);
        if (valueBuffer)
        {
            sprintf(buffer, "%s Value: %s", buffer, valueBuffer);
            free(valueBuffer);
        }
    }
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
            BOLD, LIGHT_CYAN, DTM->debug->dataTypeToString(node->dataType), COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);

    if (node->value)
    {
        char *valueBuffer = ASTNodeValueBuffer(node->value);
        if (valueBuffer)
        {
            sprintf(buffer, "%s %s%sValue:%s %s%s%s %s",
                    buffer,
                    DARK_GRAY, ITALIC, COLOR_RESET, valueBuffer, DARK_GRAY, ITALIC,
                    COLOR_RESET);
            free(valueBuffer);
        }
    }
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

    if (node->value)
    {
        char *valueBuffer = ASTNodeValueBuffer(node->value);
        if (valueBuffer)
        {
            sprintf(buffer, "%s %s%sValue:%s %s%s%s %s",
                    buffer,
                    DARK_GRAY, ITALIC, COLOR_RESET, valueBuffer, DARK_GRAY, ITALIC,
                    COLOR_RESET);
            free(valueBuffer);
        }
    }
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
    // <FunctionCall> [NAME] { RETURN_TYPE } <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<FunctionCall> [%s] →  %s <%i:%i>",
            node->nodeName,
            DTM->debug->dataTypeToString(node->dataType),
            node->line, node->column);
    return buffer;
}
char *CONSOLE_formatFunctionCallNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <FunctionCall> [NAME] { RETURN_TYPE } <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<FunctionCall>%s %s[%s] → %s%s%s %s %s%s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            YELLOW, node->nodeName, COLOR_RESET,
            BOLD, LIGHT_CYAN, DTM->debug->dataTypeToString(node->dataType), COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);

    if (node->args)
    {
        for (int i = 0; i < node->argCount; i++)
        {
            char *argBuffer = ASTNodeValueBuffer(node->args[i]);
            if (argBuffer)
            {
                sprintf(buffer, "%s %s%s%i:%s %s%s%s %s",
                        buffer, DARK_GRAY, ITALIC, i, COLOR_RESET, argBuffer, DARK_GRAY, ITALIC,
                        COLOR_RESET);
            }
        }
    }
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
    sprintf(buffer, "<Param> { Type: %s } <%i:%i>", DTM->debug->dataTypeToString(node->dataType), node->line, node->column);
    return buffer;
}
char *CONSOLE_formatParamNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <Param> <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<Param>%s %s%s{ %s }%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            BOLD, LIGHT_CYAN, DTM->debug->dataTypeToString(node->dataType), COLOR_RESET,
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
    sprintf(buffer, "<Property> [%s] { Type: %s } <0:0>", node->nodeName, DTM->debug->dataTypeToString(node->dataType));
    return buffer;
}
char *CONSOLE_formatPropertyNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <Property> <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH

    sprintf(buffer, "%s%s<Property>%s %s[%s]%s %s%s{ %s }%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET, YELLOW, node->nodeName, COLOR_RESET,
            BOLD, LIGHT_CYAN, DTM->debug->dataTypeToString(node->dataType), COLOR_RESET,
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
    sprintf(buffer, "<ExternFunction> [%s] →   %s <%i:%i>", node->nodeName, DTM->debug->dataTypeToString(node->dataType), node->line, node->column);
    return buffer;
}
char *CONSOLE_formatExternFunctionNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <ExternFunction> [NAME] [RETURN_TYPE] <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<ExternFunction>%s %s[%s] →%s  %s%s%s%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            YELLOW, node->nodeName, COLOR_RESET,
            BOLD, CYAN, DTM->debug->dataTypeToString(node->dataType), COLOR_RESET,
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
// ============================================================
// <StructConstructor>
char *formatStructConstructor(ASTDebugNode *node, DebugASTOutput *output)
{
    // <StructConstructor> <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<Constructor> <%i:%i>", node->line, node->column);
    return buffer;
}
char *CONSOLE_formatStructConstructor(ASTDebugNode *node, DebugASTOutput *output)
{
    // <StructConstructor> <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<Constructor>%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </StructConstructor>
// ============================================================
// ============================================================
// <This>
char *formatThisNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <This> <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<This> <%i:%i>", node->line, node->column);
    return buffer;
}
char *CONSOLE_formatThisNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <This> <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<This>%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </This>
// ============================================================
// ============================================================
// <ThisAssignment>
char *formatThisAssignmentNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <ThisAssignment> <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<ThisAssignment> <%i:%i>", node->line, node->column);
    return buffer;
}
char *CONSOLE_formatThisAssignmentNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <ThisAssignment> <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<ThisAssignment>%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </ThisAssignment>
// ============================================================
// ============================================================
// <PropertyAssignment>
char *formatPropertyAssignmentNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <PropertyAssignment> <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<PropertyAssignment> <%i:%i>", node->line, node->column);
    return buffer;
}
char *CONSOLE_formatPropertyAssignmentNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <PropertyAssignment> <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<PropertyAssignment>%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </PropertyAssignment>
// ============================================================
// ============================================================
// <Method>
char *formatMethodNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <Method> [NAME] [RETURN_TYPE] <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<Method> [%s]:  %s <0:0>",
            node->nodeName,
            DTM->debug->dataTypeToString(node->dataType));
    return buffer;
}
char *CONSOLE_formatMethodNode(ASTDebugNode *node, DebugASTOutput *output)
{
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH

    // Safe defaults
    const char *nodeName = node->nodeName ? node->nodeName : "NULL";
    const char *dataTypeStr = node->dataType ? DTM->debug->dataTypeToString(node->dataType) : "NULL";

    sprintf(buffer, "%s%s<Method>%s %s[%s]:%s %s%s%s%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            YELLOW, nodeName, COLOR_RESET,
            BOLD, LIGHT_CYAN, dataTypeStr, COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);

    return buffer;
}
// </Method>
// ============================================================
// ============================================================
// <IntLiteral>
char *formatIntLiteralNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <IntLiteral> [VALUE] <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<IntLiteral> [%s] <%i:%i>", node->nodeName, node->line, node->column);
    return buffer;
}
char *CONSOLE_formatIntLiteralNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <IntLiteral> [VALUE] <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<IntLiteral>%s %s[%s]%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            YELLOW, node->nodeName, COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </IntLiteral>c
// ============================================================
// ============================================================
// <StringLiteral>
char *formatStringLiteralNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <StringLiteral> [VALUE] <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<StringLiteral> [%s] <%i:%i>", node->nodeName, node->line, node->column);
    return buffer;
}
char *CONSOLE_formatStringLiteralNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <StringLiteral> [VALUE] <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<StringLiteral>%s %s[%s]%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            YELLOW, node->nodeName, COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </StringLiteral>
// ============================================================
// ============================================================
// <BooleanLiteral>
char *formatBooleanLiteralNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <BooleanLiteral> [VALUE] <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<BooleanLiteral> [%s] <%i:%i>", node->nodeName, node->line, node->column);
    return buffer;
}
char *CONSOLE_formatBooleanLiteralNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <BooleanLiteral> [VALUE] <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<BooleanLiteral>%s %s[%s]%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            YELLOW, node->nodeName, COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </BooleanLiteral>
// ============================================================
// ============================================================
// <MethodCall>
char *formatMethodCallNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <MethodCall> [NAME] { RETURN_TYPE } <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<MethodCall> [%s]: %s <0:0>",
            node->nodeName,
            DTM->debug->dataTypeToString(node->dataType));
    return buffer;
}
char *CONSOLE_formatMethodCallNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <MethodCall> [NAME] { RETURN_TYPE } <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    const char *nodeName = node->nodeName;
    const char *dataTypeStr = DTM->debug->dataTypeToString(node->dataType);

    sprintf(buffer, "%s%s<MethodCall>%s %s[%s]:%s%s%s %s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            YELLOW, nodeName, COLOR_RESET,
            BOLD, CYAN, dataTypeStr, COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </MethodCall>
// ============================================================
// ============================================================
// <BinOp>
char *formatBinOpNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <BinOp> [OPERATOR] <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<BinOp> [%s] <%i:%i>", node->nodeName, node->line, node->column);
    return buffer;
}
char *CONSOLE_formatBinOpNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <BinOp> [OPERATOR] <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<BinOp>%s %s[%s]%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            YELLOW, node->nodeName, COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </BinOp>
// ============================================================
// ============================================================
// <GenericDecl>
char *formatGenericDeclNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <GenericDecl> [NAME] <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<GenericDecl> [%s] <%i:%i>", node->nodeName, node->line, node->column);
    return buffer;
}
char *CONSOLE_formatGenericDeclNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <GenericDecl> [NAME] <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<GenericDecl>%s %s[%s]%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            YELLOW, node->nodeName, COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </GenericDecl>
// ============================================================
// ============================================================
// <GenericInst>
char *formatGenericInstNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <GenericInst> [NAME] <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<GenericInst> [%s] <%i:%i>", node->nodeName, node->line, node->column);
    return buffer;
}
char *CONSOLE_formatGenericInstNode(ASTDebugNode *node, DebugASTOutput *output)
{
    // <GenericInst> [NAME] <L:C>
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<GenericInst>%s %s[%s]%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            YELLOW, node->nodeName, COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </GenericInst>
// ============================================================
// ============================================================
// <Class>
char *formatClassNode(ASTDebugNode *node, DebugASTOutput *output)
{
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<Class> [%s] <0:0>",
            node->nodeName,
            DTM->debug->dataTypeToString(node->dataType));
    return buffer;
}

char *CONSOLE_formatClassNode(ASTDebugNode *node, DebugASTOutput *output)
{
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<Class>%s %s[%s]%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            YELLOW, node->nodeName, COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </Class>
// ============================================================
// ============================================================
// <AccessControl>
char *formatAccessControlNode(ASTDebugNode *node, DebugASTOutput *output)
{
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s:",
            node->nodeName);
    return buffer;
}
char *CONSOLE_formatAccessControlNode(ASTDebugNode *node, DebugASTOutput *output)
{
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s%s:%s",
            BOLD, LIGHT_BLUE, node->nodeName, COLOR_RESET);
    return buffer;
}
// </AccessControl>
// ============================================================
// ============================================================
// <ArgList>
char *formatArgListNode(ASTDebugNode *node, DebugASTOutput *output)
{
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<ArgList> <0:0>");
    return buffer;
}
char *CONSOLE_formatArgListNode(ASTDebugNode *node, DebugASTOutput *output)
{
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    char *argumentBuffer = (char *)malloc(sizeof(char) * BUFFER_CHAR_SIZE);
    if (!argumentBuffer)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to allocate memory for argument buffer");
        return NULL;
    }
    sprintf(argumentBuffer, "[");
    ASTNode **argsNode = node->args;
    for (int i = 0; i < node->argCount; i++)
    {
        ASTNode *argNode = argsNode[i];
        DataType *argType = DTM->astInterface->getTypeofASTNode(argNode);
        const char *argTypeString = DTM->debug->dataTypeToString(argType);

        if (i == node->argCount - 1)
        {
            sprintf(argumentBuffer, "%s%s%s", argumentBuffer, argTypeString, COLOR_RESET);
        }
        else
        {
            sprintf(argumentBuffer, "%s%s%s, ", argumentBuffer, argTypeString, COLOR_RESET);
        }
    }

    sprintf(argumentBuffer, "%s]", argumentBuffer);
    sprintf(buffer, "%s%s<ArgList>%s %s%s<0:0>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            argumentBuffer, DARK_GRAY, ITALIC, COLOR_RESET);

    return buffer;
}
// </ArgList>
// ============================================================
// ============================================================
// <ObjectInst>
char *formatObjectInstNode(ASTDebugNode *node, DebugASTOutput *output)
{
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<ObjectInst> [%s] <0:0>",
            node->nodeName,
            DTM->debug->dataTypeToString(node->dataType));
    return buffer;
}
char *CONSOLE_formatObjectInstNode(ASTDebugNode *node, DebugASTOutput *output)
{
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH

    ASTNode *objectNode = node->sourceNode;

    bool isNew = objectNode->data.objectNode->isNewInstance;
    int argCount = objectNode->data.objectNode->argCount;
    ASTNode **args = objectNode->data.objectNode->args;
    DataType **argumentTypes = (DataType **)malloc(sizeof(DataType *) * argCount);
    for (int i = 0; i < argCount; i++)
    {
        argumentTypes[i] = DTM->astInterface->getTypeofASTNode(args[i]);
        DTM->debug->printDataType(argumentTypes[i]);
    }
    char *argTypeArray = printFormattedDataTypeArray(argumentTypes, argCount);

    if (isNew)
    {
        char *newKeyword = formattedNewKeyword();
        sprintf(buffer, "%s%s%s<ObjectInst>%s %s %s[%s]%s %s %s%s<%i:%i>%s",
                COLOR_RESET, BOLD, LIGHT_MAGENTA, COLOR_RESET,
                newKeyword,
                YELLOW, node->nodeName, COLOR_RESET,
                argTypeArray,
                DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    }
    else
    {
        sprintf(buffer, "%s%s<ObjectInst>%s %s[%s]%s %s%s<%i:%i>%s",
                BOLD, LIGHT_MAGENTA, COLOR_RESET,
                YELLOW, node->nodeName, COLOR_RESET,
                DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    }
    return buffer;
}
// </ObjectInst>
// ============================================================
// ============================================================
// <UnaryOp>
char *formatUnaryOpNode(ASTDebugNode *node, DebugASTOutput *output)
{
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<UnaryOp> [%s] <0:0>",
            node->nodeName);
    return buffer;
}
char *CONSOLE_formatUnaryOpNode(ASTDebugNode *node, DebugASTOutput *output)
{
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<UnaryOp>%s %s[%s]%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            YELLOW, node->nodeName, COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </UnaryOp>
// ============================================================
// ============================================================
// <Type>
char *formatTypeNode(ASTDebugNode *node, DebugASTOutput *output)
{
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<Type> [%s] <0:0>",
            node->nodeName);
    return buffer;
}
char *CONSOLE_formatTypeNode(ASTDebugNode *node, DebugASTOutput *output)
{
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<Type>%s %s[%s]%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            YELLOW, node->nodeName, COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </Type>
// ============================================================
// ============================================================
// <Module>
char *formatModuleNode(ASTDebugNode *node, DebugASTOutput *output)
{
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<Module> [%s] <0:0>",
            node->nodeName);
    return buffer;
}
char *CONSOLE_formatModuleNode(ASTDebugNode *node, DebugASTOutput *output)
{
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<Module>%s %s[%s]%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            YELLOW, node->nodeName, COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </Module>
// ============================================================
// ============================================================
// <Implementation>
char *formatImplementationNode(ASTDebugNode *node, DebugASTOutput *output)
{
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<Implementation> [%s] <0:0>",
            node->nodeName);
    return buffer;
}
char *CONSOLE_formatImplementationNode(ASTDebugNode *node, DebugASTOutput *output)
{
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<Implementation>%s %s[%s]%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            YELLOW, node->nodeName, COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </Implementation>
// ============================================================
// ============================================================
// <PropertyAccess>
char *formatPropertyAccessNode(ASTDebugNode *node, DebugASTOutput *output)
{
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<PropertyAccess> [%s] <0:0>",
            node->nodeName);
    return buffer;
}
char *CONSOLE_formatPropertyAccessNode(ASTDebugNode *node, DebugASTOutput *output)
{
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    const char *propertyName = node->sourceNode->data.propertyAccess->propertyName ? node->sourceNode->data.propertyAccess->propertyName : "NULL";
    int propertyIndex = node->sourceNode->data.propertyAccess->propertyIndex;
    sprintf(buffer, "%s%s<PropertyAccess>%s %s[%s]%s %s%s<%i:%i>: %s%s%s%s %s%s[%i]%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            YELLOW, node->nodeName, COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column,
            BOLD, LIGHT_BLUE, propertyName, COLOR_RESET,
            DARK_GRAY, ITALIC, propertyIndex, COLOR_RESET);
    return buffer;
}
// </PropertyAccess>
// ============================================================
// ============================================================
// <IfStatement>
char *formatIfStatementNode(ASTDebugNode *node, DebugASTOutput *output)
{
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<IfStatement> <%i:%i>", node->line, node->column);
    return buffer;
}
char *CONSOLE_formatIfStatementNode(ASTDebugNode *node, DebugASTOutput *output)
{
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<IfStatement>%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </IfStatement>
// ============================================================
// ============================================================
// <NullLiteral>
char *formatNullLiteralNode(ASTDebugNode *node, DebugASTOutput *output)
{
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<NullLiteral> <%i:%i>", node->line, node->column);
    return buffer;
}
char *CONSOLE_formatNullLiteralNode(ASTDebugNode *node, DebugASTOutput *output)
{
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<NullLiteral>%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </NullLiteral>
// ============================================================
// ============================================================
// <Operator>
char *formatOperatorNode(ASTDebugNode *node, DebugASTOutput *output)
{
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "<Operator> [%s] <%i:%i>", node->nodeName, node->line, node->column);
    return buffer;
}
char *CONSOLE_formatOperatorNode(ASTDebugNode *node, DebugASTOutput *output)
{
    char *buffer = MALLOC_BUFFER;
    BUFFER_FAILED_ALLOCA_CATCH
    sprintf(buffer, "%s%s<Operator>%s %s[%s]%s %s%s<%i:%i>%s",
            BOLD, LIGHT_MAGENTA, COLOR_RESET,
            YELLOW, node->nodeName, COLOR_RESET,
            DARK_GRAY, ITALIC, node->line, node->column, COLOR_RESET);
    return buffer;
}
// </Operator>
// ============================================================
// ============================================================

// # ============================================================ #
// # AST Tree Traversal                                           #
// # ============================================================ #

///
void createASTDebugView(ASTNode *node, DebugASTOutput *output, int indentLevel)
{
    if (output->nodeCount >= AST_DEBUG_VIEW_NODE_COUNT)
    {
        logMessage(LMI, "ERROR", "AST", "Exceeded maximum node count for debug output");
        logMessage(LMI, "ERROR", "AST", "Node Count: %d", output->nodeCount);
        return;
    }

    CryoNodeType nodeType = node->metaData->type;

    switch (nodeType)
    {
    case NODE_PROGRAM:
    {
        __LINE_AND_COLUMN__
        logMessage(LMI, "DEBUG", "ASTDBG", "Program");
        ASTDebugNode *programNode = createASTDebugNode("Program", "Program", DTM->primitives->createVoid(), line, column, indentLevel, node);
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

    case NODE_MODULE:
    {
        __LINE_AND_COLUMN__
        logMessage(LMI, "DEBUG", "ASTDBG", "Module");
        ASTDebugNode *moduleNode = createASTDebugNode("Module", "Module", DTM->primitives->createVoid(), line, column, indentLevel, node);
        output->nodes[output->nodeCount] = *moduleNode;
        output->nodeCount++;
        indentLevel++;
        for (int i = 0; i < node->data.module->statementCount; i++)
        {
            createASTDebugView(node->data.module->statements[i], output, indentLevel);
        }
        indentLevel--;
        break;
    }

    case NODE_FUNCTION_DECLARATION:
    {
        __LINE_AND_COLUMN__
        logMessage(LMI, "DEBUG", "ASTDBG", "FunctionDecl");
        const char *funcName = strdup(node->data.functionDecl->name);
        DataType *returnType = node->data.functionDecl->type->container->type.functionType->returnType;
        ASTDebugNode *functionNode = createASTDebugNode("FunctionDecl", funcName, returnType, line, column, indentLevel, node);
        output->nodes[output->nodeCount] = *functionNode;
        output->nodeCount++;
        for (int i = 0; i < node->data.functionDecl->paramCount; i++)
        {
            if (node->data.functionDecl->paramCount == 0)
                break;

            indentLevel++;
            createASTDebugView(node->data.functionDecl->params[i], output, indentLevel);
            indentLevel--;
        }
        indentLevel++;
        createASTDebugView(node->data.functionDecl->body, output, indentLevel);
        break;
    }

    case NODE_EXTERN_FUNCTION:
    {
        __LINE_AND_COLUMN__
        logMessage(LMI, "DEBUG", "ASTDBG", "ExternFunction");
        char *funcName = strdup(node->data.externFunction->name);
        DataType *returnType = node->data.externFunction->type;

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
        logMessage(LMI, "DEBUG", "ASTDBG", "Block");
        int statementCount = node->data.block->statementCount;
        char *blockName = (char *)malloc(sizeof(char) * BUFFER_CHAR_SIZE);
        if (statementCount == 0)
        {
            sprintf(blockName, "EmptyBlock");
        }
        else
        {
            sprintf(blockName, "Block");
        }
        ASTDebugNode *blockNode = createASTDebugNode("Block", blockName, DTM->primitives->createVoid(), line, column, indentLevel, node);
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
        logMessage(LMI, "DEBUG", "ASTDBG", "Function Block");
        ASTDebugNode *functionBlockNode = createASTDebugNode("FunctionBlock", "FunctionBlock", DTM->primitives->createVoid(), line, column, indentLevel, node);
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
        logMessage(LMI, "DEBUG", "ASTDBG", "Struct Declaration");
        char *structName = strdup(node->data.structNode->name);
        ASTDebugNode *structNode = createASTDebugNode("StructDecl", structName, DTM->primitives->createVoid(), line, column, indentLevel, node);
        output->nodes[output->nodeCount] = *structNode;
        output->nodeCount++;
        for (int i = 0; i < node->data.structNode->propertyCount; i++)
        {
            indentLevel++;
            createASTDebugView(node->data.structNode->properties[i], output, indentLevel);
            indentLevel--;
        }

        if (node->data.structNode->constructor != NULL)
        {
            createASTDebugView(node->data.structNode->constructor, output, indentLevel + 1);
        }

        for (int i = 0; i < node->data.structNode->methodCount; i++)
        {
            indentLevel++;
            createASTDebugView(node->data.structNode->methods[i], output, indentLevel);
            indentLevel--;
        }
        break;
    }

    case NODE_PARAM_LIST:
    {
        __LINE_AND_COLUMN__
        logMessage(LMI, "DEBUG", "ASTDBG", "ParamList");
        ASTDebugNode *paramListNode = createASTDebugNode("ParamList", "ParamList", DTM->primitives->createVoid(), line, column, indentLevel, node);
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

    case NODE_STRUCT_CONSTRUCTOR:
    {
        __LINE_AND_COLUMN__
        logMessage(LMI, "DEBUG", "ASTDBG", "StructConstructor");
        ASTDebugNode *structConstructorNode = createASTDebugNode("StructConstructor", "StructConstructor", DTM->primitives->createVoid(), line, column, indentLevel, node);
        output->nodes[output->nodeCount] = *structConstructorNode;
        output->nodeCount++;
        for (int i = 0; i < node->data.structConstructor->argCount; i++)
        {
            indentLevel++;
            createASTDebugView(node->data.structConstructor->args[i], output, indentLevel);
            indentLevel--;
        }

        createASTDebugView(node->data.structConstructor->constructorBody, output, indentLevel + 1);

        break;
    }

    case NODE_METHOD:
    {
        __LINE_AND_COLUMN__
        logMessage(LMI, "DEBUG", "ASTDBG", "Method");
        char *methodName = strdup(node->data.method->name);
        DataType *returnType = node->data.method->functionType;

        ASTDebugNode *methodNode = createASTDebugNode("Method", methodName, returnType, line, column, indentLevel, node);
        output->nodes[output->nodeCount] = *methodNode;
        output->nodeCount++;
        for (int i = 0; i < node->data.method->paramCount; i++)
        {
            indentLevel++;
            createASTDebugView(node->data.method->params[i], output, indentLevel);
            indentLevel--;
        }

        createASTDebugView(node->data.method->body, output, indentLevel + 1);
        break;
    }

    case NODE_NAMESPACE:
    {
        __LINE_AND_COLUMN__
        logMessage(LMI, "DEBUG", "ASTDBG", "Namespace");
        char *namespaceName = strdup(node->data.cryoNamespace->name);
        ASTDebugNode *namespaceNode = createASTDebugNode("Namespace", namespaceName, DTM->primitives->createVoid(), line, column, indentLevel, node);
        output->nodes[output->nodeCount] = *namespaceNode;
        output->nodeCount++;
        namespaceNode->indent = indentLevel;
        break;
    }

    case NODE_VAR_DECLARATION:
    {
        __LINE_AND_COLUMN__
        logMessage(LMI, "DEBUG", "ASTDBG", "VarDecl");
        char *varName = strdup(node->data.varDecl->name);
        DataType *dataType = node->data.varDecl->type;
        ASTDebugNode *varDeclNode = createASTDebugNode("VarDecl", varName, dataType, line, column, indentLevel, node);
        varDeclNode->value = node->data.varDecl->initializer;
        output->nodes[output->nodeCount] = *varDeclNode;
        output->nodeCount++;
        break;
    }

    case NODE_EXPRESSION:
    {
        __LINE_AND_COLUMN__
        logMessage(LMI, "DEBUG", "ASTDBG", "Expression");
        ASTDebugNode *expressionNode = createASTDebugNode("Expression", "Expression", DTM->primitives->createVoid(), line, column, indentLevel, node);
        output->nodes[output->nodeCount] = *expressionNode;
        output->nodeCount++;
        break;
    }

    case NODE_LITERAL_EXPR:
    {
        __LINE_AND_COLUMN__
        logMessage(LMI, "DEBUG", "ASTDBG", "LiteralExpr");
        DataType *dataType = node->data.literal->type;
        switch (dataType->container->primitive)
        {
        case PRIM_INT:
        {
            int intValue = node->data.literal->value.intValue;
            // Allocate memory for the safe copy of int
            int *safeIntCpy = (int *)malloc(sizeof(int));
            if (safeIntCpy == NULL)
            {
                logMessage(LMI, "ERROR", "AST", "Failed to allocate memory for safe int copy");
                return;
            }
            *safeIntCpy = intValue;

            // Allocate memory for the literal value string
            char literalValue[BUFFER_CHAR_SIZE];
            snprintf(literalValue, BUFFER_CHAR_SIZE, "%i", intValue);

            ASTDebugNode *intLiteralNode = createASTDebugNode("IntLiteral", literalValue, dataType, line, column, indentLevel, node);
            if (intLiteralNode == NULL)
            {
                logMessage(LMI, "ERROR", "AST", "Failed to create AST debug node");
                free(safeIntCpy);
                return;
            }

            output->nodes[output->nodeCount] = *intLiteralNode;
            output->nodeCount++;

            // Free allocated memory
            free(safeIntCpy);
            break;
        }

        case PRIM_I128:
        case PRIM_I64:
        case PRIM_I32:
        case PRIM_I16:
        case PRIM_I8:
        {
            int intValue = node->data.literal->value.intValue;
            char *literalValue = intToSafeString(intValue);
            if (literalValue == NULL)
            {
                logMessage(LMI, "ERROR", "AST", "Failed to convert int to string");
                return;
            }
            ASTDebugNode *intLiteralNode = createASTDebugNode("IntLiteral", literalValue, dataType, line, column, indentLevel, node);
            output->nodes[output->nodeCount] = *intLiteralNode;
            output->nodeCount++;
            free(literalValue);
            break;
        }

        case PRIM_FLOAT:
        {
            char *literalValue = (char *)malloc(sizeof(char) * BUFFER_CHAR_SIZE);
            sprintf(literalValue, "%f", node->data.literal->value.floatValue);
            ASTDebugNode *floatLiteralNode = createASTDebugNode("FloatLiteral", literalValue, dataType, line, column, indentLevel, node);
            output->nodes[output->nodeCount] = *floatLiteralNode;
            output->nodeCount++;
            free(literalValue);
            break;
        }
        case PRIM_STRING:
        {
            char *literalValue = strdup(node->data.literal->value.stringValue);
            char *strippedStr = stringToUFString(literalValue);
            ASTDebugNode *stringLiteralNode = createASTDebugNode("StringLiteral", strippedStr, dataType, line, column, indentLevel, node);
            output->nodes[output->nodeCount] = *stringLiteralNode;
            output->nodeCount++;
            free(literalValue);
            break;
        }
        case PRIM_BOOLEAN:
        {
            char *literalValue = (char *)malloc(sizeof(char) * BUFFER_CHAR_SIZE);
            sprintf(literalValue, "%s", node->data.literal->value.booleanValue ? "true" : "false");
            ASTDebugNode *boolLiteralNode = createASTDebugNode("BoolLiteral", literalValue, dataType, line, column, indentLevel, node);
            output->nodes[output->nodeCount] = *boolLiteralNode;
            output->nodeCount++;
            free(literalValue);
            break;
        }

        case PRIM_OBJECT:
        {
            switch (node->data.literal->literalType)
            {
            case LITERAL_INT:
            {
                int intValue = node->data.literal->value.intValue;
                // Allocate memory for the literal value string
                char *literalValue = (char *)malloc(sizeof(char) * BUFFER_CHAR_SIZE);
                if (literalValue == NULL)
                {
                    logMessage(LMI, "ERROR", "AST", "Failed to allocate memory for literal value");
                    return;
                }
                snprintf(literalValue, BUFFER_CHAR_SIZE, "%i", intValue);

                ASTDebugNode *intLiteralNode = createASTDebugNode("IntLiteral", literalValue, dataType, line, column, indentLevel, node);
                output->nodes[output->nodeCount] = *intLiteralNode;
                output->nodeCount++;
                break;
            }
            case LITERAL_FLOAT:
            {
                char *literalValue = (char *)malloc(sizeof(char) * BUFFER_CHAR_SIZE);
                sprintf(literalValue, "%f", node->data.literal->value.floatValue);
                ASTDebugNode *floatLiteralNode = createASTDebugNode("FloatLiteral", literalValue, dataType, line, column, indentLevel, node);
                output->nodes[output->nodeCount] = *floatLiteralNode;
                output->nodeCount++;
                free(literalValue);
                break;
            }
            case LITERAL_STRING:
            {
                char *literalValue = strdup(node->data.literal->value.stringValue);
                char *strippedStr = stringToUFString(literalValue);
                ASTDebugNode *stringLiteralNode = createASTDebugNode("StringLiteral", strippedStr, dataType, line, column, indentLevel, node);
                output->nodes[output->nodeCount] = *stringLiteralNode;
                output->nodeCount++;
                free(literalValue);
                break;
            }
            case LITERAL_BOOLEAN:
            {
                char *literalValue = (char *)malloc(sizeof(char) * BUFFER_CHAR_SIZE);
                sprintf(literalValue, "%s", node->data.literal->value.booleanValue ? "true" : "false");
                ASTDebugNode *boolLiteralNode = createASTDebugNode("BoolLiteral", literalValue, dataType, line, column, indentLevel, node);
                output->nodes[output->nodeCount] = *boolLiteralNode;
                output->nodeCount++;
                free(literalValue);
                break;
            }
            default:
            {
                char *literalValue = strdup("Unknown");
                ASTDebugNode *unknownLiteralNode = createASTDebugNode("UnknownLiteral", literalValue, dataType, line, column, indentLevel, node);
                output->nodes[output->nodeCount] = *unknownLiteralNode;
                output->nodeCount++;
                free(literalValue);
                break;
            }
            }
            break;
        }

        default:
        {
            char *literalValue = strdup("Unknown");
            ASTDebugNode *unknownLiteralNode = createASTDebugNode("UnknownLiteral", literalValue, dataType, line, column, indentLevel, node);
            output->nodes[output->nodeCount] = *unknownLiteralNode;
            output->nodeCount++;
            free(literalValue);
            break;
        }
        }
        break;
    }

    case NODE_RETURN_STATEMENT:
    {
        __LINE_AND_COLUMN__
        logMessage(LMI, "DEBUG", "ASTDBG", "ReturnStatement");
        DataType *returnType = node->data.returnStatement->type;
        ASTDebugNode *returnNode = createASTDebugNode("ReturnStatement", "ReturnStatement", returnType, line, column, indentLevel, node);
        output->nodes[output->nodeCount] = *returnNode;
        output->nodeCount++;

        if (node->data.returnStatement->returnValue != NULL)
        {
            ASTNode *expr = node->data.returnStatement->expression;
            createASTDebugView(expr, output, indentLevel + 1);
        }

        break;
    }

    case NODE_FUNCTION_CALL:
    {
        __LINE_AND_COLUMN__
        logMessage(LMI, "DEBUG", "ASTDBG", "FunctionCall");
        char *funcName = strdup(node->data.functionCall->name);
        DataType *returnType = node->data.functionCall->returnType;
        ASTDebugNode *functionCallNode = createASTDebugNode("FunctionCall", funcName, returnType, line, column, indentLevel, node);
        functionCallNode->args = node->data.functionCall->args;
        functionCallNode->argCount = node->data.functionCall->argCount;
        output->nodes[output->nodeCount] = *functionCallNode;
        output->nodeCount++;
        break;
    }

    case NODE_PARAM:
    {
        __LINE_AND_COLUMN__
        logMessage(LMI, "DEBUG", "ASTDBG", "Param");
        char *paramName = strdup(node->data.param->name);
        DataType *paramType = node->data.param->type;

        ASTDebugNode *paramNode = createASTDebugNode("Param", paramName, paramType, line, column, indentLevel, node);
        paramNode->sourceNode = node;
        output->nodes[output->nodeCount] = *paramNode;
        output->nodeCount++;
        break;
    }

    case NODE_PROPERTY:
    {
        __LINE_AND_COLUMN__
        logMessage(LMI, "DEBUG", "ASTDBG", "Property");
        char *propertyName = strdup(node->data.property->name);
        DataType *propertyType = node->data.property->type;

        ASTDebugNode *propertyNode = createASTDebugNode("Property", propertyName, propertyType, line, column, indentLevel, node);
        output->nodes[output->nodeCount] = *propertyNode;
        output->nodeCount++;
        break;
    }

    case NODE_VAR_NAME:
    {
        __LINE_AND_COLUMN__
        logMessage(LMI, "DEBUG", "ASTDBG", "VarName");
        char *varName = strdup(node->data.varName->varName);
        ASTDebugNode *varNameNode = createASTDebugNode("VarName", varName, DTM->primitives->createVoid(), line, column, indentLevel, node);
        output->nodes[output->nodeCount] = *varNameNode;
        output->nodeCount++;
        break;
    }

    case NODE_ARG_LIST:
    {
        __LINE_AND_COLUMN__
        logMessage(LMI, "DEBUG", "ASTDBG", "ArgList");
        ASTDebugNode *argListNode = createASTDebugNode("ArgList", "ArgList", DTM->primitives->createVoid(), line, column, indentLevel, node);
        ASTNode **args = node->data.argList->args;
        argListNode->args = args;
        argListNode->argCount = node->data.argList->argCount;

        output->nodes[output->nodeCount] = *argListNode;
        output->nodeCount++;
        break;
    }

    case NODE_THIS:
    {
        __LINE_AND_COLUMN__
        logMessage(LMI, "DEBUG", "ASTDBG", "This");
        ASTDebugNode *thisNode = createASTDebugNode("This", "This", DTM->primitives->createVoid(), line, column, indentLevel, node);
        output->nodes[output->nodeCount] = *thisNode;
        output->nodeCount++;
        break;
    }

    case NODE_THIS_ASSIGNMENT:
    {
        __LINE_AND_COLUMN__
        logMessage(LMI, "DEBUG", "ASTDBG", "ThisAssign");
        ASTDebugNode *thisReassignNode = createASTDebugNode("ThisAssign", "ThisAssign", DTM->primitives->createVoid(), line, column, indentLevel, node);
        output->nodes[output->nodeCount] = *thisReassignNode;
        output->nodeCount++;
        break;
    }

    case NODE_PROPERTY_REASSIGN:
    {
        __LINE_AND_COLUMN__
        logMessage(LMI, "DEBUG", "ASTDBG", "PropertyReassign");
        ASTDebugNode *propertyReassignNode = createASTDebugNode("PropertyReassign", "PropertyReassign", DTM->primitives->createVoid(), line, column, indentLevel, node);
        output->nodes[output->nodeCount] = *propertyReassignNode;
        output->nodeCount++;
        break;
    }

    case NODE_METHOD_CALL:
    {
        __LINE_AND_COLUMN__
        logMessage(LMI, "DEBUG", "ASTDBG", "MethodCall");
        char *methodName = strdup(node->data.methodCall->name);
        DataType *returnType = node->data.methodCall->returnType;
        ASTDebugNode *methodCallNode = createASTDebugNode("MethodCall", methodName, returnType, line, column, indentLevel, node);
        methodCallNode->args = node->data.methodCall->args;
        methodCallNode->argCount = node->data.methodCall->argCount;
        output->nodes[output->nodeCount] = *methodCallNode;
        output->nodeCount++;
        break;
    }

    case NODE_BINARY_EXPR:
    {
        __LINE_AND_COLUMN__
        logMessage(LMI, "DEBUG", "ASTDBG", "BinaryExpr");
        ASTDebugNode *binaryExprNode = createASTDebugNode("BinOp", "BinOp", DTM->primitives->createVoid(), line, column, indentLevel, node);
        output->nodes[output->nodeCount] = *binaryExprNode;
        output->nodeCount++;

        // lhs
        createASTDebugView(node->data.bin_op->left, output, indentLevel + 1);

        // operator
        char *opName = CryoOperatorTypeToString(node->data.bin_op->op);
        ASTDebugNode *opNode = createASTDebugNode("Operator", opName, DTM->primitives->createVoid(), line, column, indentLevel + 1, node);
        output->nodes[output->nodeCount] = *opNode;
        output->nodeCount++;

        // rhs
        createASTDebugView(node->data.bin_op->right, output, indentLevel + 1);
        break;
    }

    case NODE_CLASS:
    {
        __LINE_AND_COLUMN__
        logMessage(LMI, "DEBUG", "ASTDBG", "Class");
        const char *className = node->data.classNode->name;
        logMessage(LMI, "DEBUG", "ASTDBG", "ClassName: %s", className);

        DataType *classType = node->data.classNode->type;
        classType->debug->printType(classType);
        classType->debug->printVerbosType(classType);

        ASTDebugNode *classNode = createASTDebugNode("Class", className, classType, line, column, indentLevel, node);
        output->nodes[output->nodeCount] = *classNode;
        output->nodeCount++;

        // Handle constructor if present
        if (node->data.classNode->constructorCount < 0)
        {
            indentLevel++;
            logMessage(LMI, "DEBUG", "ASTDBG", "Creating Debug View for Constructor");
            for (int i = 0; i < node->data.classNode->constructorCount; i++)
            {
                createASTDebugView(node->data.classNode->constructors[i], output, indentLevel);
            }
            indentLevel--;
        }

        // Handle public members
        if (node->data.classNode->publicMembers)
        {
            logMessage(LMI, "DEBUG", "ASTDBG", "Creating Debug View for Public Members");
            ASTDebugNode *publicNode = createASTDebugNode("AccessControl", "Public", NULL, 0, 0, indentLevel + 1, NULL);
            printf("Created Debug Node for Public Members\n");
            output->nodes[output->nodeCount] = *publicNode;
            output->nodeCount++;

            logMessage(LMI, "DEBUG", "ASTDBG", "Public Members Count: %d", node->data.classNode->publicMembers->propertyCount);
            // Log public properties
            for (int i = 0; i < node->data.classNode->publicMembers->propertyCount; i++)
            {
                const char *propertyName = node->data.classNode->publicMembers->properties[i]->data.property->name;
                DataType *propertyType = node->data.classNode->publicMembers->properties[i]->data.property->type;
                ASTDebugNode *propertyNode = createASTDebugNode("Property", propertyName, propertyType, 0, 0, indentLevel + 2, node->data.classNode->publicMembers->properties[i]);
                output->nodes[output->nodeCount] = *propertyNode;
                output->nodeCount++;
            }

            logMessage(LMI, "DEBUG", "ASTDBG", "Public Methods Count: %d", node->data.classNode->publicMembers->methodCount);
            // Log public methods
            for (int i = 0; i < node->data.classNode->publicMembers->methodCount; i++)
            {
                const char *methodName = node->data.classNode->publicMembers->methods[i]->data.method->name;
                DataType *methodType = node->data.classNode->publicMembers->methods[i]->data.method->type;
                ASTDebugNode *methodNode = createASTDebugNode("Method", methodName, methodType, 0, 0, indentLevel + 2, node->data.classNode->publicMembers->methods[i]);
                output->nodes[output->nodeCount] = *methodNode;
                output->nodeCount++;

                // Log method body
                logMessage(LMI, "DEBUG", "ASTDBG", "Creating Debug View for Method Body");
                createASTDebugView(node->data.classNode->publicMembers->methods[i]->data.method->body, output, indentLevel + 3);
            }
        }

        // Handle private members
        logMessage(LMI, "DEBUG", "ASTDBG", "Creating Debug View for Private Members");
        if (node->data.classNode->privateMembers)
        {
            ASTDebugNode *privateNode = createASTDebugNode("AccessControl", "Private", DTM->primitives->createVoid(), 0, 0, indentLevel + 1, NULL);
            output->nodes[output->nodeCount] = *privateNode;
            output->nodeCount++;

            // Log private properties
            for (int i = 0; i < node->data.classNode->privateMembers->propertyCount; i++)
            {
                const char *propertyName = node->data.classNode->privateMembers->properties[i]->data.property->name;
                DataType *propertyType = node->data.classNode->privateMembers->properties[i]->data.property->type;
                ASTDebugNode *propertyNode = createASTDebugNode("Property", propertyName, propertyType, 0, 0, indentLevel + 2, node->data.classNode->privateMembers->properties[i]);
                output->nodes[output->nodeCount] = *propertyNode;
                output->nodeCount++;
            }

            // Log private methods
            for (int i = 0; i < node->data.classNode->privateMembers->methodCount; i++)
            {
                const char *methodName = node->data.classNode->privateMembers->methods[i]->data.method->name;
                DataType *methodType = node->data.classNode->privateMembers->methods[i]->data.method->type;
                ASTDebugNode *methodNode = createASTDebugNode("Method", methodName, methodType, 0, 0, indentLevel + 2, node->data.classNode->privateMembers->methods[i]);
                output->nodes[output->nodeCount] = *methodNode;
                output->nodeCount++;

                // Log method body
                createASTDebugView(node->data.classNode->privateMembers->methods[i]->data.method->body, output, indentLevel + 3);
            }
        }

        // Handle protected members
        if (node->data.classNode->protectedMembers)
        {
            ASTDebugNode *protectedNode = createASTDebugNode("AccessControl", "Protected", DTM->primitives->createVoid(), 0, 0, indentLevel + 1, NULL);
            output->nodes[output->nodeCount] = *protectedNode;
            output->nodeCount++;

            // Log protected properties
            for (int i = 0; i < node->data.classNode->protectedMembers->propertyCount; i++)
            {
                const char *propertyName = node->data.classNode->protectedMembers->properties[i]->data.property->name;
                DataType *propertyType = node->data.classNode->protectedMembers->properties[i]->data.property->type;
                ASTDebugNode *propertyNode = createASTDebugNode("Property", propertyName, propertyType, 0, 0, indentLevel + 2, node->data.classNode->protectedMembers->properties[i]);
                output->nodes[output->nodeCount] = *propertyNode;
                output->nodeCount++;
            }

            // Log protected methods
            for (int i = 0; i < node->data.classNode->protectedMembers->methodCount; i++)
            {
                const char *methodName = node->data.classNode->protectedMembers->methods[i]->data.method->name;
                DataType *methodType = node->data.classNode->protectedMembers->methods[i]->data.method->type;
                ASTDebugNode *methodNode = createASTDebugNode("Method", methodName, methodType, 0, 0, indentLevel + 2, node->data.classNode->protectedMembers->methods[i]);
                output->nodes[output->nodeCount] = *methodNode;
                output->nodeCount++;

                // Log method body
                createASTDebugView(node->data.classNode->protectedMembers->methods[i]->data.method->body, output, indentLevel + 3);
            }
        }
        break;
    }

    case NODE_OBJECT_INST:
    {
        __LINE_AND_COLUMN__
        logMessage(LMI, "DEBUG", "ASTDBG", "ObjectInst");
        char *objectName = strdup(node->data.objectNode->name);
        DataType *objectType = node->data.objectNode->objType;
        ASTDebugNode *objectNode = createASTDebugNode("ObjectInst", objectName, objectType, line, column, indentLevel, node);
        output->nodes[output->nodeCount] = *objectNode;
        output->nodeCount++;
        break;
    }

    case NODE_UNARY_EXPR:
    {
        __LINE_AND_COLUMN__
        logMessage(LMI, "DEBUG", "ASTDBG", "UnaryExpr");
        ASTDebugNode *unaryExprNode = createASTDebugNode("UnaryOp", "UnaryOp", DTM->primitives->createVoid(), line, column, indentLevel, node);
        output->nodes[output->nodeCount] = *unaryExprNode;
        output->nodeCount++;
        createASTDebugView(node->data.unary_op->expression, output, indentLevel + 1);
        break;
    }

    case NODE_TYPE:
    {
        __LINE_AND_COLUMN__
        logMessage(LMI, "DEBUG", "ASTDBG", "Type");
        char *typeName = strdup(node->data.typeDecl->name);
        DataType *type = node->data.typeDecl->type;
        ASTDebugNode *typeNode = createASTDebugNode("Type", typeName, type, line, column, indentLevel, node);
        output->nodes[output->nodeCount] = *typeNode;
        output->nodeCount++;
        break;
    }

    case NODE_IMPLEMENTATION:
    {
        __LINE_AND_COLUMN__
        logMessage(LMI, "DEBUG", "ASTDBG", "Implementation");
        const char *implName = node->data.implementation->interfaceName;

        DataType *implType = node->data.implementation->interfaceType;
        ASTDebugNode *implNode = createASTDebugNode("Implementation", (char *)implName, implType, line, column, indentLevel, node);
        output->nodes[output->nodeCount] = *implNode;
        output->nodeCount++;

        // Handle implemented properties
        for (int i = 0; i < node->data.implementation->propertyCount; i++)
        {
            const char *propertyName = node->data.implementation->properties[i]->data.property->name;
            if (propertyName == NULL)
            {
                logMessage(LMI, "ERROR", "AST", "Property name is NULL");
                return;
            }
            DataType *propertyType = node->data.implementation->properties[i]->data.property->type;
            ASTDebugNode *propertyNode = createASTDebugNode("Property", propertyName, propertyType, 0, 0, indentLevel + 1, node->data.implementation->properties[i]);
            output->nodes[output->nodeCount] = *propertyNode;
            output->nodeCount++;
        }
        // Handle implemented methods
        for (int i = 0; i < node->data.implementation->methodCount; i++)
        {
            const char *methodName = node->data.implementation->methods[i]->data.method->name;
            if (methodName == NULL)
            {
                logMessage(LMI, "ERROR", "AST", "Method name is NULL");
                return;
            }
            DataType *methodType = node->data.implementation->methods[i]->data.method->type;
            ASTDebugNode *methodNode = createASTDebugNode("Method", methodName, methodType, 0, 0, indentLevel + 1, node->data.implementation->methods[i]);
            output->nodes[output->nodeCount] = *methodNode;
            output->nodeCount++;

            // Check if the method has a body
            if (node->data.implementation->methods[i]->data.method->body != NULL)
            {
                createASTDebugView(node->data.implementation->methods[i]->data.method->body, output, indentLevel + 2);
            }
        }

        break;
    }

    case NODE_PROPERTY_ACCESS:
    {
        __LINE_AND_COLUMN__
        logMessage(LMI, "DEBUG", "ASTDBG", "PropertyAccess");
        const char *propertyName = node->data.propertyAccess->propertyName;
        DataType *voidType = DTM->primitives->createVoid();
        ASTDebugNode *propertyAccessNode = createASTDebugNode("PropertyAccess", propertyName, voidType, line, column, indentLevel, node);
        output->nodes[output->nodeCount] = *propertyAccessNode;
        output->nodeCount++;
        break;
    }

    case NODE_SCOPED_FUNCTION_CALL:
    {
        __LINE_AND_COLUMN__
        logMessage(LMI, "DEBUG", "ASTDBG", "ScopedFunctionCall");
        char *funcName = strdup(node->data.scopedFunctionCall->functionName);
        DataType *type = node->data.scopedFunctionCall->type;
        ASTDebugNode *scopedFunctionCallNode = createASTDebugNode("ScopedFunctionCall", funcName, type, line, column, indentLevel, node);
        scopedFunctionCallNode->args = node->data.scopedFunctionCall->args;
        scopedFunctionCallNode->argCount = node->data.scopedFunctionCall->argCount;
        output->nodes[output->nodeCount] = *scopedFunctionCallNode;
        output->nodeCount++;
        break;
    }

    case NODE_IF_STATEMENT:
    {
        __LINE_AND_COLUMN__
        logMessage(LMI, "DEBUG", "ASTDBG", "IfStatement");
        ASTDebugNode *ifNode = createASTDebugNode("IfStatement", "IfStatement", DTM->primitives->createVoid(), line, column, indentLevel, node);
        output->nodes[output->nodeCount] = *ifNode;
        output->nodeCount++;
        if (node->data.ifStatement->condition != NULL)
        {
            createASTDebugView(node->data.ifStatement->condition, output, indentLevel + 1);
        }
        if (node->data.ifStatement->elseBranch != NULL)
        {
            createASTDebugView(node->data.ifStatement->elseBranch, output, indentLevel + 1);
        }
        if (node->data.ifStatement->thenBranch != NULL)
        {
            createASTDebugView(node->data.ifStatement->thenBranch, output, indentLevel + 1);
        }
        break;
    }

    case NODE_NULL_LITERAL:
    {
        __LINE_AND_COLUMN__
        logMessage(LMI, "DEBUG", "ASTDBG", "NullLiteral");
        ASTDebugNode *nullLiteralNode = createASTDebugNode("NullLiteral", "null", DTM->primitives->createVoid(), line, column, indentLevel, node);
        output->nodes[output->nodeCount] = *nullLiteralNode;
        output->nodeCount++;
        break;
    }

    case NODE_ANNOTATION:
    {
        // We don't need to log annotations
        logMessage(LMI, "DEBUG", "ASTDBG", "Skipping Annotation...");
        break;
    }

    case NODE_DISCARD:
    {
        // We don't need to log discard nodes
        logMessage(LMI, "DEBUG", "ASTDBG", "Skipping Discard...");
        break;
    }

    default:
    {
        logMessage(LMI, "ERROR", "AST", "Unknown node type encountered: %s", CryoNodeTypeToString(nodeType));
        break;
    }
    }

    return;
}

char *ASTNodeValueBuffer(ASTNode *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    switch (node->metaData->type)
    {

    // Literals
    case NODE_LITERAL_EXPR:
    {
        DataType *type = node->data.literal->type;
        switch (type->container->primitive)
        {
        case PRIM_INT:
        {
            char *buffer = (char *)malloc(sizeof(char) * BUFFER_CHAR_SIZE);
            if (!buffer)
            {
                logMessage(LMI, "ERROR", "AST", "Failed to allocate memory for buffer");
                return NULL;
            }
            int intValue = node->data.literal->value.intValue;
            printf("int value: %i\n", intValue);
            strcat(buffer, intToSafeString(intValue));

            char *copyBuffer = strdup(buffer);
            free(buffer);
            return copyBuffer;
        }
        case PRIM_FLOAT:
        {
            char *buffer = (char *)malloc(sizeof(char) * BUFFER_CHAR_SIZE);
            sprintf(buffer, "%f", node->data.literal->value.floatValue);
            return buffer;
        }
        case PRIM_STRING:
        {
            char *buffer = (char *)malloc(sizeof(char) * BUFFER_CHAR_SIZE);
            if (!node->data.literal->value.stringValue)
            {
                logMessage(LMI, "ERROR", "AST", "Failed to convert string to buffer");
                return NULL;
            }
            sprintf(buffer, "%s", node->data.literal->value.stringValue);
            if (buffer == NULL)
            {
                logMessage(LMI, "ERROR", "AST", "Failed to convert string to buffer");
                return NULL;
            }
            char *strippedStr = stringToUFString(buffer);
            return strippedStr;
        }
        case PRIM_BOOLEAN:
        {
            char *buffer = (char *)malloc(sizeof(char) * BUFFER_CHAR_SIZE);
            sprintf(buffer, "%s", node->data.literal->value.booleanValue ? "true" : "false");
            return buffer;
        }
        default:
        {
            char *buffer = strdup("Unknown");
            return buffer;
        }
        }
    }
    // VarNames
    case NODE_VAR_NAME:
    {
        char *buffer = strdup(node->data.varName->varName);
        return buffer;
    }

    default:
    {
        return NULL;
    }
    }

    return NULL;
}

char *formattedNewKeyword(void)
{
    char *buffer = (char *)malloc(sizeof(char) * BUFFER_CHAR_SIZE);
    sprintf(buffer, BOLD CYAN "new" COLOR_RESET);
    // Another color reset on the buffer to be safe
    strcat(buffer, COLOR_RESET);
    return buffer;
}

char *printFormattedDataTypeArray(DataType **typeArray, int typeCount)
{
    // Yellow `[]` brackets and Light Cyan for the type
    char *buffer = (char *)malloc(sizeof(char) * BUFFER_CHAR_SIZE);
    if (!buffer)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to allocate memory for type buffer");
        return NULL;
    }

    sprintf(buffer, COLOR_RESET YELLOW "[" COLOR_RESET);
    for (int i = 0; i < typeCount; i++)
    {
        DataType *type = typeArray[i];
        const char *typeString = DTM->debug->dataTypeToString(type);
        if (i == typeCount - 1)
        {
            sprintf(buffer, "%s%s%s", buffer, typeString, COLOR_RESET);
        }
        else
        {
            sprintf(buffer, "%s%s%s, ", buffer, typeString, COLOR_RESET);
        }
    }

    sprintf(buffer, "%s" COLOR_RESET YELLOW "]" COLOR_RESET, buffer);
    return buffer;
}
