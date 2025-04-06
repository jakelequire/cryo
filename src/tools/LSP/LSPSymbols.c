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
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include <signal.h>
#include <netinet/tcp.h>
#include "diagnostics/diagnostics.h"

// Global array to store symbols
LSPSymbol *symbolTable[MAX_SYMBOLS];
int symbolCount = 0;

// Add global variable for graceful shutdown
volatile sig_atomic_t keep_running = 1;

// Add signal handler
void handle_shutdown(int signum)
{
    __STACK_FRAME__
    keep_running = 0;
}

void check_port_status(int port)
{
    __STACK_FRAME__
    int test_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (test_socket < 0)
    {
        fprintf(stderr, "<Compiler> ERROR: Cannot create test socket: %s\n", strerror(errno));
        return;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    int result = bind(test_socket, (struct sockaddr *)&addr, sizeof(addr));
    if (result < 0)
    {
        if (errno == EADDRINUSE)
        {
            fprintf(stderr, "<Compiler> DEBUG: Port %d is in use\n", port);

            // Kill the process using the port
            char command[256];
            snprintf(command, sizeof(command), "fuser -k %d/tcp", port);
            system(command);
        }
        else
        {
            fprintf(stderr, "<Compiler> DEBUG: Port %d bind test failed: %s\n",
                    port, strerror(errno));
        }
    }
    else
    {
        fprintf(stderr, "<Compiler> DEBUG: Port %d is available\n", port);
    }

    close(test_socket);
}

bool check_proxy_running(void)
{
    __STACK_FRAME__
    // Try to connect to the debug proxy port
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        return false;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(DEBUG_PORT);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000; // 100ms timeout

    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    int result = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    close(sock);

    return result == 0;
}

int get_server_port(void)
{
    __STACK_FRAME__
    // First check if debug proxy is running
    if (check_proxy_running())
    {
        fprintf(stderr, "<Compiler> INFO: Debug proxy detected, using port %d\n", LSP_PORT);
        return LSP_PORT;
    }

    // If no proxy, use direct port
    fprintf(stderr, "<Compiler> INFO: No debug proxy detected, using direct port %d\n", LSP_PORT);
    return LSP_PORT;
}

void runLSPSymbols(ASTNode *programNode)
{
    __STACK_FRAME__
    // Process AST and collect symbols
    processNode(programNode);

    // Print symbols
    for (int i = 0; i < symbolCount; i++)
    {
        LSPSymbol *symbol = symbolTable[i];
        printf("Symbol: %s\n", symbol->name);
    }

    // Start server and send symbols
    startLSPServer();
}

LSPSymbol *createLSPSymbol(void)
{
    __STACK_FRAME__
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
bool sendSymbol(int client_fd, LSPSymbol *symbol)
{
    __STACK_FRAME__
    if (!symbol)
    {
        fprintf(stderr, "<Compiler> ERROR: Attempted to send NULL symbol\n");
        return false;
    }

    char *symbolStr = formatSymbol(symbol);
    if (!symbolStr)
    {
        fprintf(stderr, "<Compiler> ERROR: Failed to format symbol %s\n",
                symbol->name ? symbol->name : "unknown");
        return false;
    }

    // Log the JSON we're about to send for debugging
    fprintf(stderr, "<Compiler> DEBUG: Sending symbol JSON: %s\n", symbolStr);

    // Send the JSON string
    size_t len = strlen(symbolStr);
    ssize_t sent = send(client_fd, symbolStr, len, MSG_NOSIGNAL);
    if (sent < 0)
    {
        fprintf(stderr, "<Compiler> ERROR: Failed to send symbol data: %s\n",
                strerror(errno));
        free(symbolStr);
        return false;
    }

    // Send newline separator
    if (send(client_fd, "\n", 1, MSG_NOSIGNAL) < 0)
    {
        fprintf(stderr, "<Compiler> ERROR: Failed to send newline: %s\n",
                strerror(errno));
        free(symbolStr);
        return false;
    }

    free(symbolStr);
    return true;
}

// Use this function in startLSPServer when sending symbols:
void sendAllSymbols(int client_fd)
{
    __STACK_FRAME__
    int successful = 0;
    int failed = 0;

    fprintf(stderr, "<Compiler> INFO: Sending %d symbols\n", symbolCount);

    for (int i = 0; i < symbolCount; i++)
    {
        LSPSymbol *symbol = symbolTable[i];
        if (sendSymbol(client_fd, symbol))
        {
            successful++;
        }
        else
        {
            failed++;
        }
    }

    fprintf(stderr, "<Compiler> INFO: Sent %d symbols successfully, %d failed\n",
            successful, failed);
}

// Start LSP server and send symbols
void startLSPServer(void)
{
    __STACK_FRAME__
    fprintf(stderr, "<Compiler> INFO: Starting LSP server...\n");

    // Setup signal handling
    signal(SIGINT, handle_shutdown);
    signal(SIGTERM, handle_shutdown);
    signal(SIGPIPE, SIG_IGN); // Ignore SIGPIPE

    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        fprintf(stderr, "<Compiler> ERROR: Socket creation failed: %s\n", strerror(errno));
        return;
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        fprintf(stderr, "<Compiler> ERROR: setsockopt failed: %s\n", strerror(errno));
        close(server_fd);
        return;
    }

    // Set keep-alive options
    int keepalive = 1;
    int keepcnt = 3;
    int keepidle = 10;
    int keepintvl = 5;
    setsockopt(server_fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));
    setsockopt(server_fd, IPPROTO_TCP, TCP_KEEPCNT, &keepcnt, sizeof(keepcnt));
    setsockopt(server_fd, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle));
    setsockopt(server_fd, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(keepintvl));

    // Setup address structure
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(LSP_PORT);

    // Bind with retry
    int bind_attempts = 0;
    const int max_bind_attempts = 10;

    while (bind_attempts < max_bind_attempts && keep_running)
    {
        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
        {
            if (errno == EADDRINUSE)
            {
                fprintf(stderr, "<Compiler> WARN: Port %d in use, waiting (attempt %d/%d)...\n",
                        LSP_PORT, bind_attempts + 1, max_bind_attempts);
                sleep(1);
                bind_attempts++;
                continue;
            }
            fprintf(stderr, "<Compiler> ERROR: Bind failed: %s\n", strerror(errno));
            close(server_fd);
            return;
        }
        break;
    }

    if (listen(server_fd, 5) < 0)
    {
        fprintf(stderr, "<Compiler> ERROR: Listen failed: %s\n", strerror(errno));
        close(server_fd);
        return;
    }

    fprintf(stderr, "<Compiler> INFO: LSP server started on port %d\n", LSP_PORT);

    // Main server loop
    while (keep_running)
    {
        // Setup for select()
        fd_set readfds;
        struct timeval tv;
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);

        tv.tv_sec = 1; // 1 second timeout
        tv.tv_usec = 0;

        int activity = select(server_fd + 1, &readfds, NULL, NULL, &tv);

        if (activity < 0 && errno != EINTR)
        {
            fprintf(stderr, "<Compiler> ERROR: Select failed: %s\n", strerror(errno));
            continue;
        }

        if (activity == 0)
            continue; // Timeout, check keep_running and continue

        // Accept connection
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);

        if (client_fd < 0)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                fprintf(stderr, "<Compiler> ERROR: Accept failed: %s\n", strerror(errno));
            }
            continue;
        }

        // Log connection
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        fprintf(stderr, "<Compiler> INFO: New connection from %s:%d\n",
                client_ip, ntohs(client_addr.sin_port));

        // Send initial symbols
        sendAllSymbols(client_fd);

        // Client message handling loop
        while (keep_running)
        {
            char buffer[8192] = {0};

            // Setup select for client socket
            fd_set client_fds;
            struct timeval client_tv;
            FD_ZERO(&client_fds);
            FD_SET(client_fd, &client_fds);
            client_tv.tv_sec = 1;
            client_tv.tv_usec = 0;

            int client_activity = select(client_fd + 1, &client_fds, NULL, NULL, &client_tv);

            if (client_activity < 0)
            {
                if (errno != EINTR)
                {
                    fprintf(stderr, "<Compiler> ERROR: Client select failed: %s\n", strerror(errno));
                    break;
                }
                continue;
            }

            if (client_activity == 0)
                continue;

            ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

            if (bytes_read < 0)
            {
                if (errno != EAGAIN && errno != EWOULDBLOCK)
                {
                    fprintf(stderr, "<Compiler> ERROR: Recv failed: %s\n", strerror(errno));
                    break;
                }
                continue;
            }

            if (bytes_read == 0)
            {
                fprintf(stderr, "<Compiler> INFO: Client disconnected\n");
                break;
            }

            buffer[bytes_read] = '\0';

            if (strstr(buffer, "update_symbols") != NULL)
            {
                fprintf(stderr, "<Compiler> INFO: Received symbol update request\n");
                sendAllSymbols(client_fd);
            }
        }

        close(client_fd);
    }

    fprintf(stderr, "<Compiler> INFO: LSP server shutting down...\n");
    close(server_fd);
}

// Format symbol as JSON string
char *formatSymbol(LSPSymbol *symbol)
{
    __STACK_FRAME__
    if (!symbol)
    {
        return NULL;
    }

    // Ensure we have non-NULL values for all fields
    const char *name = symbol->name ? symbol->name : "";
    const char *signature = symbol->signature ? symbol->signature : "";
    const char *documentation = symbol->documentation ? symbol->documentation : "";
    const char *kind = symbol->kind ? symbol->kind : "";
    const char *type = symbol->type ? symbol->type : "";
    const char *parent = symbol->parent ? symbol->parent : "";
    const char *file = symbol->file ? symbol->file : "";
    const char *line = symbol->line ? symbol->line : "";
    const char *column = symbol->column ? symbol->column : "";

    // Allocate plenty of space for the JSON string
    char *buffer = (char *)malloc(sizeof(char) * 4096);
    if (!buffer)
    {
        fprintf(stderr, "<Compiler> ERROR: Failed to allocate memory for symbol JSON\n");
        return NULL;
    }

    // Format JSON with proper escaping
    int written = snprintf(buffer, 4096,
                           "{"
                           "\"name\":\"%s\","
                           "\"signature\":\"%s\","
                           "\"documentation\":\"%s\","
                           "\"kind\":\"%s\","
                           "\"type\":\"%s\","
                           "\"parent\":\"%s\","
                           "\"file\":\"%s\","
                           "\"line\":\"%s\","
                           "\"column\":\"%s\""
                           "}",
                           name, signature, documentation, kind, type, parent, file, line, column);

    if (written >= 4096)
    {
        fprintf(stderr, "<Compiler> ERROR: Symbol JSON truncated\n");
        free(buffer);
        return NULL;
    }

    return buffer;
}

// Process AST node and create symbol
void processNode(ASTNode *node)
{
    __STACK_FRAME__
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
        DataType *functionType = node->data.functionDecl->functionType;
        symbol->signature = (char *)functionType->debug->toString(functionType);

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
    {
        symbol->kind = "variable";
        symbol->name = node->data.varDecl->name;
        DataType *varType = node->data.varDecl->type;
        symbol->signature = (char *)varType->debug->toString(varType);
        break;
    }
    case NODE_VAR_NAME:
    {
        symbol->kind = "variable";
        symbol->name = node->data.varName->varName;
        DataType *varNameType = node->data.varName->type;
        symbol->signature = (char *)varNameType->debug->toString(varNameType);
        break;
    }
    case NODE_METHOD:
    {
        symbol->kind = "method";
        symbol->name = (char *)node->data.method->name;
        DataType *methodType = node->data.method->functionType;
        symbol->signature = (char *)methodType->debug->toString(methodType);
        processNode(node->data.method->body);
        break;
    }
    case NODE_CLASS:
    {
        symbol->kind = "class";
        symbol->name = (char *)node->data.classNode->name;
        break;
    }
    case NODE_PROPERTY:
    {
        symbol->kind = "property";
        symbol->name = (char *)node->data.property->name;
        DataType *propertyType = node->data.property->type;
        symbol->signature = (char *)propertyType->debug->toString(propertyType);
        break;
    }
    // Skip Nodes:
    case NODE_NAMESPACE:
    case NODE_USING:
    case NODE_RETURN_STATEMENT:
    {
        fprintf(stderr, "<Compiler> INFO: Skipping node type: %s\n", CryoNodeTypeToString(node->metaData->type));
        break;
    }
    default:
        fprintf(stderr, "<Compiler> ERROR: Unhandled node type: %s\n", CryoNodeTypeToString(node->metaData->type));
        break;
    }

    if (symbol->name[0] != '\0' && symbol->name[0] != NULL)
    { // Only add if we got a valid symbol
        fprintf(stderr, "<Compiler> INFO: Adding symbol: %s\n", symbol->name);
        symbolTable[symbolCount++] = symbol;
    }
    else
    {
        fprintf(stderr, "<Compiler> ERROR: Invalid symbol name\n");
        free(symbol);
    }
}
