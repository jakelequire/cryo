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
#include "include/utils.h"

/*
Note to self before I forget,
printf the size of the Cryo program in the debugging process.
*/

// <verbose_log>
void verbose_log(const char* msg, const char* token) {
    printf("VERBOSE: %s | Token: %s\n", msg, token);
}
// </verbose_log>


// <critical_log>
void critical_log(const char* msg, const char* token, const char* etc) {
    printf("VERBOSE: %s | Token: %s | Etc: %s\n", msg, token, etc);
}
// </critical_log>


/*----------------------------------*/
/*### VERBOSE LOGGING FUNCTIONS  ###*/
/*----------------------------------*/

// <verbose_cryo_token_log>
void verbose_cryo_token_log(CryoTokenType* token) {
    // *implement later with lsp*
}
// <verbose_cryo_token_log>


// <verbose_cryo_node_log>
void verbose_cryo_node_log(CryoNodeType* node) {
    // *implement later with lsp*
}
// </verbose_cryo_node_log>


// <verbose_cryo_astnode_log>
void verbose_cryo_astnode_log(ASTNode* astNode) {
    // *implement later with lsp*
}
// <verbose_cryo_astnode_log>



/*-----------------------------------*/
/*### CRITICAL LOGGING FUNCTIONS  ###*/
/*-----------------------------------*/

// <critical_cryo_token_log>
void critical_cryo_token_log(CryoTokenType* token) {
    // *implement later with lsp*
}
// <critical_cryo_token_log>


// <critical_cryo_node_log>
void critical_cryo_node_log(CryoNodeType* node) {
    // *implement later with lsp*
}
// </critical_cryo_node_log>


// <critical_cryo_astnode_log>
void critical_cryo_astnode_log(ASTNode* astNode) {
    // *implement later with lsp*
}
// </critical_cryo_astnode_log>
