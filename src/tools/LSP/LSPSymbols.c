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
#include "tools/LSP/LSPSymbols.h"

// Global array to store symbols
LSPSymbol *symbolTable[MAX_SYMBOLS];
int symbolCount = 0;

void runLSPSymbols(ASTNode *programNode)
{
    // Process AST and collect symbols
    processNode(programNode);

    // Print symbols
    for (int i = 0; i < symbolCount; i++)
    {
        LSPSymbol *symbol = symbolTable[i];
        logMessage(LMI, "INFO", "LSPSymbols", "Symbol: %s", symbol->name);
    }

    // Start server and send symbols
    startLSPServer();
}

LSPSymbol *createLSPSymbol(void)
{
    LSPSymbol *symbol = (LSPSymbol *)malloc(sizeof(LSPSymbol));
    if (!symbol)
    {
        return NULL;
    }

    symbol->name = "";
    symbol->signature = "";
    symbol->documentation = "";
    symbol->kind = "";
    symbol->type = "";
    symbol->parent = "";
    symbol->file = "";
    symbol->line = "";
    symbol->column = "";

    return symbol;
}

// Start LSP server and send symbols
void startLSPServer(void)
{
    logMessage(LMI, "INFO", "LSPSymbols", "Starting LSP server...");
    int server_fd, client_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Socket creation and setup (your existing code)...

    while (1)
    { // Keep the server running
        // Accept incoming connections
        if ((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            logMessage(LMI, "ERROR", "LSPSymbols", "accept failed");
            continue; // Continue listening for next connection
        }

        // Send initial symbols batch
        for (int i = 0; i < symbolCount; i++)
        {
            LSPSymbol *symbol = symbolTable[i];
            char *symbolStr = formatSymbol(symbol);
            send(client_fd, symbolStr, strlen(symbolStr), 0);
            send(client_fd, "\n", 1, 0); // Add newline delimiter
            free(symbolStr);
        }

        // Keep connection open for updates
        char buffer[1024];
        while (1)
        {
            int bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
            if (bytes_read <= 0)
            {
                break; // Connection closed or error
            }
            buffer[bytes_read] = '\0';

            // Handle incoming messages (e.g., requests for symbol updates)
            if (strstr(buffer, "update_symbols") != NULL)
            {
                // Resend symbols if requested
                for (int i = 0; i < symbolCount; i++)
                {
                    LSPSymbol *symbol = symbolTable[i];
                    char *symbolStr = formatSymbol(symbol);
                    send(client_fd, symbolStr, strlen(symbolStr), 0);
                    send(client_fd, "\n", 1, 0);
                    free(symbolStr);
                }
            }

            // Break if there is an error
            if (bytes_read < 0)
            {
                break;
            }
        }

        close(client_fd); // Close this client connection
    }

    close(server_fd); // This will only happen if the while(1) is broken
}

// Format symbol as JSON string
char *formatSymbol(LSPSymbol *symbol)
{
    char *buffer = (char *)malloc(sizeof(char) * 1024);
    if (!buffer)
    {
        return NULL;
    }

    snprintf(buffer, 1024, "{\n"
                           "  \"name\": \"%s\",\n"
                           "  \"signature\": \"%s\",\n"
                           "  \"documentation\": \"%s\",\n"
                           "  \"kind\": \"%s\",\n"
                           "  \"type\": \"%s\",\n"
                           "  \"parent\": \"%s\",\n"
                           "  \"file\": \"%s\",\n"
                           "  \"line\": \"%s\",\n"
                           "  \"column\": \"%s\"\n"
                           "}\n",
             symbol->name, symbol->signature, symbol->documentation, symbol->kind, symbol->type, symbol->parent, symbol->file, symbol->line, symbol->column);

    return buffer;
}

// Process AST node and create symbol
void processNode(ASTNode *node)
{
    if (!node || symbolCount >= MAX_SYMBOLS)
        return;

    // Create symbol based on node type
    LSPSymbol *symbol = createLSPSymbol();

    // Fill in symbol details based on AST node type
    switch (node->metaData->type)
    {
    case NODE_PROGRAM:
    {
        ASTNode **statements = node->data.program->statements;
        for (int i = 0; i < node->data.program->statementCount; i++)
        {
            processNode(statements[i]);
        }
        break;
    }
    case NODE_FUNCTION_DECLARATION:
    {
        symbol->kind = "function";
        symbol->name = (char *)node->data.functionDecl->name;
        symbol->signature = DataTypeToStringUnformatted(node->data.functionDecl->functionType);

        processNode(node->data.functionDecl->body);
        break;
    }
    case NODE_FUNCTION_BLOCK:
    {
        ASTNode **statements = node->data.functionBlock->statements;
        for (int i = 0; i < node->data.functionBlock->statementCount; i++)
        {
            processNode(statements[i]);
        }
        break;
    }
    case NODE_BLOCK:
    {
        ASTNode **statements = node->data.block->statements;
        for (int i = 0; i < node->data.block->statementCount; i++)
        {
            processNode(statements[i]);
        }
        break;
    }
    case NODE_VAR_DECLARATION:
        symbol->kind = "variable";
        symbol->name = node->data.varDecl->name;
        symbol->signature = DataTypeToStringUnformatted(node->data.varDecl->type);
        break;
    case NODE_VAR_NAME:
        symbol->kind = "variable";
        symbol->name = node->data.varName->varName;
        symbol->signature = DataTypeToStringUnformatted(node->data.varName->type);
        break;
    case NODE_METHOD:
        symbol->kind = "method";
        symbol->name = node->data.method->name;
        symbol->signature = DataTypeToStringUnformatted(node->data.method->functionType);
        processNode(node->data.method->body);
        break;
    case NODE_CLASS:
        symbol->kind = "class";
        symbol->name = (char *)node->data.classNode->name;
        break;
    case NODE_PROPERTY:
        symbol->kind = "property";
        symbol->name = (char *)node->data.property->name;
        symbol->signature = DataTypeToStringUnformatted(node->data.property->type);
        break;
    // Skip Nodes:
    case NODE_NAMESPACE:
    case NODE_USING:
    case NODE_RETURN_STATEMENT:
    {
        logMessage(LMI, "INFO", "LSPSymbols", "Skipping node type: %s", CryoNodeTypeToString(node->metaData->type));
        break;
    }
    default:
        logMessage(LMI, "WARN", "LSPSymbols", "Unhandled node type: %s", CryoNodeTypeToString(node->metaData->type));
        break;
    }

    if (symbol->name[0] != '\0' && symbol->name[0] != NULL)
    { // Only add if we got a valid symbol
        logMessage(LMI, "INFO", "LSPSymbols", "Adding symbol: %s", symbol->name);
        symbolTable[symbolCount++] = symbol;
    }
    else
    {
        logMessage(LMI, "WARN", "LSPSymbols", "Symbol name is empty");
        free(symbol);
    }
}
