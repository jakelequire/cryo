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
#ifndef LOGGER_H
#define LOGGER_H
/*------ <includes> ------*/
#include <stdio.h>
#include "compiler/ast.h"
#include "compiler/token.h"
#include "compiler/lexer.h"
#include "parser.h"
/*---------<end>---------*/

#define LOG_LEVEL_NONE 0
#define LOG_LEVEL_VERBOSE 1
#define LOG_LEVEL_CRITICAL 2

#define CURRENT_LOG_LEVEL LOG_LEVEL_VERBOSE

typedef struct ASTNode ASTNode;

/*---------<MACROS>---------*/
// VERBOSE LOGGING
#if CURRENT_LOG_LEVEL >= LOG_LEVEL_VERBOSE
    #define VERBOSE_LOG(component, msg, token) verbose_log(component, msg, token)
    #define VERBOSE_CRYO_TOKEN_LOG(component, token) verbose_cryo_token_log(component, token)
    #define VERBOSE_CRYO_NODE_LOG(component, node) verbose_cryo_node_log(component, node)
    #define VERBOSE_CRYO_ASTNODE_LOG(component, astNode) verbose_cryo_astnode_log(component, astNode)
#else
    #define VERBOSE_LOG(component, msg, token)
    #define VERBOSE_CRYO_TOKEN_LOG(component, token)
    #define VERBOSE_CRYO_NODE_LOG(component, node)
    #define VERBOSE_CRYO_ASTNODE_LOG(component, astNode)
#endif
// CRITICAL LOGGING
#if CURRENT_LOG_LEVEL >= LOG_LEVEL_CRITICAL
    #define CRITICAL_LOG(component, msg, token, etc) critical_log(component, msg, token, etc)
#else
    #define CRITICAL_LOG(component, msg, token, etc)
#endif
/*-------<END_MACROS>-------*/


/*-----<function_prototypes>-----*/
// Logging function prototypes
void verbose_log(const char* component, const char* msg, const char* token);
void critical_log(const char* component, const char* msg, const char* token, const char* etc);

// VERBOSE LOGGING FUNCTION PROTOTYPES
void verbose_cryo_token_log(const char* component, CryoTokenType* token);
void verbose_cryo_node_log(const char* component, CryoNodeType* node);
void verbose_cryo_astnode_log(const char* component, ASTNode* astNode);
// CRITICAL LOGGING FUNCTION PROTOTYPES
void critical_cryo_token_log(const char* component, CryoTokenType* token);
void critical_cryo_node_log(const char* component, CryoNodeType* node);
void critical_cryo_astnode_log(const char* component, ASTNode* astNode);
/*-------<end_prototypes>-------*/

#endif // LOGGER_H
