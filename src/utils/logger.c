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
#include "utils/logger.h"

// <verbose_log>
void verbose_log(const char* component, const char* msg, const char* token) {
    printf("VERBOSE [%s]: %s | Token: %s\n", component, msg, token);
}
// </verbose_log>

// <critical_log>
void critical_log(const char* component, const char* msg, const char* token, const char* etc) {
    printf("CRITICAL [%s]: %s | Token: %s | Etc: %s\n", component, msg, token, etc);
}
// </critical_log>

// <verbose_cryo_token_log>
void verbose_cryo_token_log(const char* component, CryoTokenType* token) {
    printf("### [VERBOSE] [%s] Cryo Token: %d\n", component, *token);
}
// </verbose_cryo_token_log>

// <verbose_cryo_node_log>
void verbose_cryo_node_log(const char* component, CryoNodeType* node) {
    printf("### [VERBOSE] [%s] Cryo Node: %d\n", component, *node);
}
// </verbose_cryo_node_log>

// <verbose_cryo_astnode_log>
void verbose_cryo_astnode_log(const char* component, ASTNode* astNode) {
    if (astNode) {
        printf("### [VERBOSE] [%s] Cryo ASTNode Type: %d\n", component, astNode->type);
        // Additional details can be printed here
    } else {
        printf("### [VERBOSE] [%s] Cryo ASTNode: NULL\n", component);
    }
}
// </verbose_cryo_astnode_log>

// <critical_cryo_token_log>
void critical_cryo_token_log(const char* component, CryoTokenType* token) {
    printf("### [CRITICAL] [%s] Cryo Token: %d\n", component, *token);
}
// </critical_cryo_token_log>

// <critical_cryo_node_log>
void critical_cryo_node_log(const char* component, CryoNodeType* node) {
    printf("### [CRITICAL] [%s] Cryo Node: %d\n", component, *node);
}
// </critical_cryo_node_log>

// <critical_cryo_astnode_log>
void critical_cryo_astnode_log(const char* component, ASTNode* astNode) {
    if (astNode) {
        printf("### [CRITICAL] [%s] Cryo ASTNode Type: %d\n", component, astNode->type);
        // Additional details can be printed here
    } else {
        printf("### [CRITICAL] [%s] Cryo ASTNode: NULL\n", component);
    }
}
// </critical_cryo_astnode_log>
