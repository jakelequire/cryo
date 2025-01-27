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
#ifndef LSP_SYMBOLS_H
#define LSP_SYMBOLS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "frontend/AST.h"
#include "frontend/dataTypes.h"
#include "tools/utils/c_logger.h"

#define LSP_PORT 9000
#define DEBUG_PORT 9001
#define MAX_SYMBOLS 1024

typedef struct LSPSymbol
{
    char *name;
    char *signature;
    char *documentation;
    char *kind;
    char *type;
    char *parent;
    char *file;
    char *line;
    char *column;
} LSPSymbol;

// --------------------------------------------------

void runLSPSymbols(ASTNode *programNode);
void startLSPServer(void);
void processNode(ASTNode *node);
char *formatSymbol(LSPSymbol *symbol);

LSPSymbol *createLSPSymbol(void);

#endif // LSP_SYMBOLS_H
