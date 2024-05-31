#ifndef UTILS_H
#define UTILS_H
/*------ <includes> ------*/
#include <stdio.h>
#include "ast.h"
#include "token.h"
#include "parser.h"
/*---------<end>---------*/

#define LOG_LEVEL_NONE 0                                                            
#define LOG_LEVEL_VERBOSE 1                                                         
#define LOG_LEVEL_CRITICAL 2                                                        

#define CURRENT_LOG_LEVEL LOG_LEVEL_VERBOSE                                         

/*---------<MACROS>---------*/                                                      
// VERBOSE LOGGING                                                                  
#if CURRENT_LOG_LEVEL >= LOG_LEVEL_VERBOSE                                          
    #define VERBOSE_LOG(msg, token) verbose_log(msg, token)                         
    #define VERBOSE_CRYO_TOKEN_LOG(token) verbose_cryo_token_log(token)             
    #define VERBOSE_CRYO_NODE_LOG(node) verbose_cryo_node_log(node)                 
    #define VERBOSE_CRYO_ASTNODE_LOG(astNode) verbose_cryo_astnode_log(astNode)     
#else                                                                               
    #define VERBOSE_LOG(msg, token)                                                 
    #define VERBOSE_CRYO_TOKEN_LOG(token)                                           
    #define VERBOSE_CRYO_NODE_LOG(node)                                             
    #define VERBOSE_CRYO_ASTNODE_LOG(astNode)                                       
#endif                                                                              
// CRITICAL LOGGING                                                                 
#if CURRENT_LOG_LEVEL >= LOG_LEVEL_CRITICAL                                         
    #define CRITICAL_LOG(msg, token, etc) critical_log(msg, token, etc)             
#else                                                                               
    #define CRITICAL_LOG(msg, token, etc)                                           
#endif                                                                              
/*-------<END_MACROS>-------*/


/*-----<function_prototypes>-----*/                                                 
/*-----<logger>-----*/                                                              
// Will replace token with Cryo Tokens / AST / Etc.                                 
// The `etc` is a filler. Will replace with other parameters later.                 
void verbose_log(const char* msg, const char* token);                               
void critical_log(const char* msg, const char* token, const char* etc);             

// Future production implementations *draft*:
// VERBOSE LOGGING FUNCTION PROTOTYPES
void verbose_cryo_token_log(CryoTokenType* token);      //<undefined>               
void verbose_cryo_node_log(CryoNodeType* node);         //<undefined>           
void verbose_cryo_astnode_log(ASTNode* astNode);        //<undefined>               
// CRITICAL LOGGING FUNCTION PROTOTYPES
void critical_cryo_token_log(CryoTokenType* token);     //<undefined>               
void critical_cryo_node_log(CryoNodeType* node);        //<undefined>           
void critical_cryo_astnode_log(ASTNode* astNode);       //<undefined>               
/*---<end_logger>---*/
/*-------<fs>-------*/
#define CUR_ROOT_DIR_ABS // TODO: Implement                                         
#define CUR_ROOT_DIR_REL // TODO: Implement                                         
char* readFile(const char* path);                                                   
char* currentDir(void);                                 //<undefined>               
//int mkdir(char* absPath, const char* relPath);          //<undefined>               
/*-----<end_fs>-----*/
/*-------<end_prototypes>-------*/

#endif // UTILS_H
