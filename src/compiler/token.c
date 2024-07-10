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
#include "compiler/token.h"


char* operatorToString(CryoOperatorType op) {
    switch (op) {
        case OPERATOR_ADD:                  return "+";
        case OPERATOR_SUB:                  return "-";
        case OPERATOR_MUL:                  return "*";
        case OPERATOR_DIV:                  return "/";
        case OPERATOR_MOD:                  return "%";
        case OPERATOR_AND:                  return "&";
        case OPERATOR_OR:                   return "|";
        case OPERATOR_XOR:                  return "^";
        case OPERATOR_NOT:                  return "!";
        case OPERATOR_LSHIFT:               return "<<";
        case OPERATOR_RSHIFT:               return ">>";
        case OPERATOR_LT:                   return "<";
        case OPERATOR_GT:                   return ">";
        case OPERATOR_LTE:                  return "<=";
        case OPERATOR_GTE:                  return ">=";
        case OPERATOR_EQ:                   return "==";
        case OPERATOR_NEQ:                  return "!=";
        case OPERATOR_ASSIGN:               return "=";
        case OPERATOR_ADD_ASSIGN:           return "+=";
        case OPERATOR_SUB_ASSIGN:           return "-=";
        case OPERATOR_MUL_ASSIGN:           return "*=";
        case OPERATOR_DIV_ASSIGN:           return "/=";
        case OPERATOR_MOD_ASSIGN:           return "%=";
        case OPERATOR_AND_ASSIGN:           return "&=";
        case OPERATOR_OR_ASSIGN:            return "|=";
        case OPERATOR_XOR_ASSIGN:           return "^=";
        case OPERATOR_NA:                   return "<N/A>";
        default:                            return "<OPERATOR UNKNOWN>";
    }
}

char* CryoNodeTypeToString(CryoNodeType node) {
    switch(node) {
        case NODE_PROGRAM:                  return "NODE_PROGRAM";
        case NODE_FUNCTION_DECLARATION:     return "NODE_FUNCTION_DECLARATION";
        case NODE_VAR_DECLARATION:          return "NODE_VAR_DECLARATION";      
        case NODE_STATEMENT:                return "NODE_STATEMENT";
        case NODE_EXPRESSION:               return "NODE_EXPRESSION";
        case NODE_BINARY_EXPR:              return "NODE_BINARY_EXPR";
        case NODE_UNARY_EXPR:               return "NODE_UNARY_EXPR";
        case NODE_LITERAL_EXPR:             return "NODE_LITERAL_EXPR";
        case NODE_VAR_NAME:                 return "NODE_VAR_NAME";
        case NODE_FUNCTION_CALL:            return "NODE_FUNCTION_CALL";
        case NODE_IF_STATEMENT:             return "NODE_IF_STATEMENT";
        case NODE_WHILE_STATEMENT:          return "NODE_WHILE_STATEMENT";
        case NODE_FOR_STATEMENT:            return "NODE_FOR_STATEMENT";
        case NODE_RETURN_STATEMENT:         return "NODE_RETURN_STATEMENT";
        case NODE_BLOCK:                    return "NODE_BLOCK";
        case NODE_FUNCTION_BLOCK:           return "NODE_FUNCTION_BLOCK";
        case NODE_EXPRESSION_STATEMENT:     return "NODE_EXPRESSION_STATEMENT";
        case NODE_ASSIGN:                   return "NODE_ASSIGN";
        case NODE_PARAM_LIST:               return "NODE_PARAM_LIST";
        case NODE_TYPE:                     return "NODE_TYPE";
        case NODE_STRING_LITERAL:           return "NODE_STRING_LITERAL";
        case NODE_BOOLEAN_LITERAL:          return "NODE_BOOLEAN_LITERAL";
        case NODE_ARRAY_LITERAL:            return "NODE_ARRAY_LITERAL";
        case NODE_IMPORT_STATEMENT:         return "NODE_IMPORT_STATEMENT";
        case NODE_EXTERN_STATEMENT:         return "NODE_EXTERN_STATEMENT";
        case NODE_EXTERN_FUNCTION:          return "NODE_EXTERN_FUNCTION";
        case NODE_ARG_LIST:                 return "NODE_ARG_LIST";
        case NODE_UNKNOWN:                  return "NODE_UNKNOWN";
        default:                            return "<NODE TYPE DEFAULTED>";
    }
}


char* CryoDataTypeToString(CryoDataType type) {
    switch(type) {
        case DATA_TYPE_INT:                 return "DATA_TYPE_INT";
        case NODE_LITERAL_INT:              return "NODE_LITERAL_INT";
        case DATA_TYPE_FLOAT:               return "DATA_TYPE_FLOAT";
        case DATA_TYPE_STRING:              return "DATA_TYPE_STRING";
        case DATA_TYPE_BOOLEAN:             return "DATA_TYPE_BOOLEAN";
        case DATA_TYPE_FUNCTION:            return "DATA_TYPE_FUNCTION";
        case DATA_TYPE_VOID:                return "DATA_TYPE_VOID";
        case DATA_TYPE_NULL:                return "DATA_TYPE_NULL";
        case DATA_TYPE_ARRAY:               return "DATA_TYPE_ARRAY";
        case DATA_TYPE_INT_ARRAY:           return "DATA_TYPE_INT_ARRAY";
        case DATA_TYPE_FLOAT_ARRAY:         return "DATA_TYPE_FLOAT_ARRAY";
        case DATA_TYPE_STRING_ARRAY:        return "DATA_TYPE_STRING_ARRAY";
        case DATA_TYPE_BOOLEAN_ARRAY:       return "DATA_TYPE_BOOLEAN_ARRAY";
        case DATA_TYPE_VOID_ARRAY:          return "DATA_TYPE_VOID_ARRAY";
        case DATA_TYPE_UNKNOWN:             return "DATA_TYPE_UNKNOWN";
        default:                            return "<DATA TYPE UNKNOWN>";
    }
}

char* CryoVisibilityTypeToString(CryoVisibilityType visibility) {
    switch(visibility) {
        case VISIBILITY_PUBLIC:             return "VISIBILITY_PUBLIC";  
        case VISIBILITY_PRIVATE:            return "VISIBILITY_PRIVATE";
        case VISIBILITY_PROTECTED:          return "VISIBILITY_PROTECTED";
        case VISIBILITY_UNKNOWN:            return "VISIBILITY_UNKNOWN";
        default:                            return "<VISIBILITY UNKNOWN>";
    }
}

char* CryoTokenToString(CryoTokenType node) {
    switch(node) {
        case TOKEN_EOF:                         return "TOKEN_EOF";
        case TOKEN_IDENTIFIER:                  return "TOKEN_IDENTIFIER";
        case TOKEN_ERROR:                       return "TOKEN_ERROR";
        case TOKEN_SINGLE_COMMENT:              return "TOKEN_SINGLE_COMMENT";
        case TOKEN_MULTI_COMMENT:               return "TOKEN_MULTI_COMMENT";
        case TOKEN_WHITESPACE:                  return "TOKEN_WHITESPACE";
        case TOKEN_ASSIGN:                      return "TOKEN_ASSIGN";
        case TOKEN_NEWLINE:                     return "TOKEN_NEWLINE";
        case TOKEN_TAB:                         return "TOKEN_TAB";
        case TOKEN_INDENT:                      return "TOKEN_INDENT";
        case TOKEN_DEDENT:                      return "TOKEN_DEDENT";

        case TOKEN_TYPE_INT:                    return "TOKEN_TYPE_INT";
        case TOKEN_TYPE_FLOAT:                  return "TOKEN_TYPE_FLOAT";
        case TOKEN_TYPE_STRING:                 return "TOKEN_TYPE_STRING";
        case TOKEN_TYPE_STRING_LITERAL:         return "TOKEN_TYPE_STRING_LITERAL";
        case TOKEN_TYPE_BOOLEAN:                return "TOKEN_TYPE_BOOLEAN";
        case TOKEN_TYPE_VOID:                   return "TOKEN_TYPE_VOID";
        case TOKEN_TYPE_INT_ARRAY:              return "TOKEN_TYPE_INT_ARRAY";
        case TOKEN_TYPE_FLOAT_ARRAY:            return "TOKEN_TYPE_FLOAT_ARRAY";
        case TOKEN_TYPE_STRING_ARRAY:           return "TOKEN_TYPE_STRING_ARRAY";
        case TOKEN_TYPE_BOOLEAN_ARRAY:          return "TOKEN_TYPE_BOOLEAN_ARRAY";
        case TOKEN_RESULT_ARROW:                return "TOKEN_RESULT_ARROW";

        case TOKEN_INT_LITERAL:                 return "TOKEN_INT_LITERAL";
        case TOKEN_FLOAT_LITERAL:               return "TOKEN_FLOAT_LITERAL";
        case TOKEN_STRING_LITERAL:              return "TOKEN_STRING_LITERAL";
        case TOKEN_BOOLEAN_LITERAL:             return "TOKEN_BOOLEAN_LITERAL";
        case TOKEN_VOID_LITERAL:                return "TOKEN_VOID_LITERAL";
        case TOKEN_NULL_LITERAL:                return "TOKEN_NULL_LITERAL";
        case TOKEN_BIN_OP_LITERAL:              return "TOKEN_BIN_OP_LITERAL";

        case TOKEN_OP_PLUS:                     return "TOKEN_OP_PLUS";
        case TOKEN_OP_MINUS:                    return "TOKEN_OP_MINUS";
        case TOKEN_OP_STAR:                     return "TOKEN_OP_STAR";
        case TOKEN_OP_SLASH:                    return "TOKEN_OP_SLASH";
        case TOKEN_OP_MOD:                      return "TOKEN_OP_MOD";
        case TOKEN_OP_AND:                      return "TOKEN_OP_AND";
        case TOKEN_OP_OR:                       return "TOKEN_OP_OR";
        case TOKEN_OP_XOR:                      return "TOKEN_OP_XOR";
        case TOKEN_OP_NOT:                      return "TOKEN_OP_NOT";
        case TOKEN_OP_LSHIFT:                   return "TOKEN_OP_LSHIFT";
        case TOKEN_OP_RSHIFT:                   return "TOKEN_OP_RSHIFT";
        case TOKEN_OP_LT:                       return "TOKEN_OP_LT";
        case TOKEN_OP_GT:                       return "TOKEN_OP_GT";
        case TOKEN_OP_LTE:                      return "TOKEN_OP_LTE";
        case TOKEN_OP_GTE:                      return "TOKEN_OP_GTE";
        case TOKEN_OP_EQ:                       return "TOKEN_OP_EQ";
        case TOKEN_OP_NEQ:                      return "TOKEN_OP_NEQ";
        case TOKEN_OP_ASSIGN:                   return "TOKEN_OP_ASSIGN";
        case TOKEN_OP_ADD_ASSIGN:               return "TOKEN_OP_ADD_ASSIGN";
        case TOKEN_OP_SUB_ASSIGN:               return "TOKEN_OP_SUB_ASSIGN";
        case TOKEN_OP_MUL_ASSIGN:               return "TOKEN_OP_MUL_ASSIGN";
        case TOKEN_OP_DIV_ASSIGN:               return "TOKEN_OP_DIV_ASSIGN";
        case TOKEN_OP_MOD_ASSIGN:               return "TOKEN_OP_MOD_ASSIGN";
        case TOKEN_OP_AND_ASSIGN:               return "TOKEN_OP_AND_ASSIGN";
        case TOKEN_OP_OR_ASSIGN:                return "TOKEN_OP_OR_ASSIGN";
        case TOKEN_OP_XOR_ASSIGN:               return "TOKEN_OP_XOR_ASSIGN";

        case TOKEN_KW_IF:                       return "TOKEN_KW_IF";
        case TOKEN_KW_ELSE:                     return "TOKEN_KW_ELSE";
        case TOKEN_KW_FOR:                      return "TOKEN_KW_FOR";
        case TOKEN_KW_WHILE:                    return "TOKEN_KW_WHILE";
        case TOKEN_KW_BREAK:                    return "TOKEN_KW_BREAK";
        case TOKEN_KW_CONTINUE:                 return "TOKEN_KW_CONTINUE";
        case TOKEN_KW_RETURN:                   return "TOKEN_KW_RETURN";
        case TOKEN_KW_FN:                       return "TOKEN_KW_FN";
        case TOKEN_KW_CONST:                    return "TOKEN_KW_CONST";
        case TOKEN_KW_PUBLIC:                   return "TOKEN_KW_PUBLIC";
        case TOKEN_KW_PRIVATE:                  return "TOKEN_KW_PRIVATE";
        case TOKEN_KW_STRUCT:                   return "TOKEN_KW_STRUCT";
        case TOKEN_KW_ENUM:                     return "TOKEN_KW_ENUM";
        case TOKEN_KW_TRAIT:                    return "TOKEN_KW_TRAIT";
        case TOKEN_KW_IMPL:                     return "TOKEN_KW_IMPL";
        case TOKEN_KW_USE:                      return "TOKEN_KW_USE";
        case TOKEN_KW_AS:                       return "TOKEN_KW_AS";
        case TOKEN_KW_MODULE:                   return "TOKEN_KW_MODULE";
        case TOKEN_KW_NAMESPACE:                return "TOKEN_KW_NAMESPACE";
        case TOKEN_KW_TYPE:                     return "TOKEN_KW_TYPE";
        case TOKEN_KW_CAST:                     return "TOKEN_KW_CAST";
        case TOKEN_KW_SIZE:                     return "TOKEN_KW_SIZE";
        case TOKEN_KW_CASE:                     return "TOKEN_KW_CLASS";
        case TOKEN_KW_CLASS:                    return "TOKEN_KW_DEFAULT";
        case TOKEN_KW_DEFAULT:                  return "TOKEN_KW_DEFAULT";
        case TOKEN_KW_EXTENDS:                  return "TOKEN_KW_EXTENDS";
        case TOKEN_KW_IMPORT:                   return "TOKEN_KW_IMPORT";
        case TOKEN_KW_IN:                       return "TOKEN_KW_IN";
        case TOKEN_KW_INSTANCEOF:               return "TOKEN_KW_INSTANCEOF";
        case TOKEN_KW_NEW:                      return "TOKEN_KW_NEW";
        case TOKEN_KW_NULL:                     return "TOKEN_KW_NULL";
        case TOKEN_KW_PROTECTED:                return "TOKEN_KW_PROTECTED";
        case TOKEN_KW_STATIC:                   return "TOKEN_KW_STATIC";
        case TOKEN_KW_RESULT:                   return "TOKEN_KW_RESULT";
        case TOKEN_KW_TRUE:                     return "TOKEN_KW_TRUE";
        case TOKEN_KW_FALSE:                    return "TOKEN_KW_FALSE";
        case TOKEN_KW_MUT:                      return "TOKEN_KW_MUT";
        case TOKEN_KW_BYTE:                     return "TOKEN_KW_BYTE";
        case TOKEN_KW_SHORT:                    return "TOKEN_KW_SHORT";
        case TOKEN_KW_INT:                      return "TOKEN_KW_INT";
        case TOKEN_KW_LONG:                     return "TOKEN_KW_LONG";
        case TOKEN_KW_FLOAT:                    return "TOKEN_KW_FLOAT";
        case TOKEN_KW_DOUBLE:                   return "TOKEN_KW_DOUBLE";
        case TOKEN_KW_CHAR:                     return "TOKEN_KW_CHAR";
        case TOKEN_KW_BOOL:                     return "TOKEN_KW_BOOL";
        case TOKEN_KW_VOID:                     return "TOKEN_KW_VOID";
        case TOKEN_KW_ANY:                      return "TOKEN_KW_ANY";
        case TOKEN_KW_STRING:                   return "TOKEN_KW_STRING";
        case TOKEN_KW_ARRAY:                    return "TOKEN_KW_ARRAY";
        case TOKEN_KW_MAP:                      return "TOKEN_KW_MAP";
        case TOKEN_KW_SET:                      return "TOKEN_KW_SET";
        case TOKEN_KW_NATIVE:                   return "TOKEN_KW_NATIVE";
        case TOKEN_KW_SYNCHRONIZED:             return "TOKEN_KW_SYNCHRONIZED";
        case TOKEN_KW_VOLATILE:                 return "TOKEN_KW_VOLATILE";
        case TOKEN_KW_INTERFACE:                return "TOKEN_KW_INTERFACE";
        case TOKEN_KW_BOOLEAN:                  return "TOKEN_KW_BOOLEAN";
        case TOKEN_KW_TYPEOF:                   return "TOKEN_KW_TYPEOF";
        case TOKEN_KW_AWAIT:                    return "TOKEN_KW_AWAIT";
        case TOKEN_KW_ASYNC:                    return "TOKEN_KW_ASYNC";
        case TOKEN_KW_EXPRESSION:               return "TOKEN_KW_EXPRESSION";
        case TOKEN_KW_EXTERN:                   return "TOKEN_KW_EXTERN";

        case TOKEN_BANG:                        return "TOKEN_BANG";
        case TOKEN_PLUS:                        return "TOKEN_PLUS";
        case TOKEN_MINUS:                       return "TOKEN_MINUS";
        case TOKEN_STAR:                        return "TOKEN_STAR";
        case TOKEN_SLASH:                       return "TOKEN_SLASH";
        case TOKEN_PIPE:                        return "TOKEN_PIPE";
        case TOKEN_EXCLAMATION:                 return "TOKEN_EXCLAMATION";
        case TOKEN_QUESTION:                    return "TOKEN_QUESTION";
        case TOKEN_COLON:                       return "TOKEN_COLON";
        case TOKEN_LESS:                        return "TOKEN_LESS";
        case TOKEN_GREATER:                     return "TOKEN_GREATER";
        case TOKEN_EQUAL:                       return "TOKEN_EQUAL";
        case TOKEN_DOT:                         return "TOKEN_DOT";
        case TOKEN_COMMA:                       return "TOKEN_COMMA";
        case TOKEN_HASH:                        return "TOKEN_HASH";
        case TOKEN_DOLLAR:                      return "TOKEN_DOLLAR";
        case TOKEN_AT:                          return "TOKEN_AT";
        case TOKEN_BACKSLASH:                   return "TOKEN_BACKSLASH";
        case TOKEN_UNDERSCORE:                  return "TOKEN_UNDERSCORE";
        case TOKEN_BACKTICK:                    return "TOKEN_BACKTICK";
        case TOKEN_QUOTE:                       return "TOKEN_QUOTE";
        case TOKEN_APOSTROPHE:                  return "TOKEN_APOSTROPHE";
        case TOKEN_SEMICOLON:                   return "TOKEN_SEMICOLON";
        case TOKEN_TILDE:                       return "TOKEN_TILDE";
        case TOKEN_CARET:                       return "TOKEN_CARET";
        case TOKEN_AMPERSAND:                   return "TOKEN_AMPERSAND";
        case TOKEN_PERCENT:                     return "TOKEN_PERCENT";
        case TOKEN_LPAREN:                      return "TOKEN_LPAREN";
        case TOKEN_RPAREN:                      return "TOKEN_RPAREN";
        case TOKEN_LBRACKET:                    return "TOKEN_LBRACKET";
        case TOKEN_RBRACKET:                    return "TOKEN_RBRACKET";
        case TOKEN_LBRACE:                      return "TOKEN_LBRACE";
        case TOKEN_RBRACE:                      return "TOKEN_RBRACE";
        case TOKEN_LANGLE:                      return "TOKEN_LANGLE";
        case TOKEN_RANGLE:                      return "TOKEN_RANGLE";
        case TOKEN_LSQUARE:                     return "TOKEN_LSQUARE";
        case TOKEN_RSQUARE:                     return "TOKEN_RSQUARE";
        case TOKEN_LANGLE_BRACKET:              return "TOKEN_LANGLE_BRACKET";
        case TOKEN_RANGLE_BRACKET:              return "TOKEN_RANGLE_BRACKET";
        case TOKEN_LCURLY_BRACKET:              return "TOKEN_LCURLY_BRACKET";
        case TOKEN_RCURLY_BRACKET:              return "TOKEN_RCURLY_BRACKET";
        case TOKEN_ELLIPSIS:                    return "TOKEN_ELLIPSIS";

        case TOKEN_ITER_VAR:                    return "TOKEN_ITER_VAR";
        case TOKEN_ITER_STEP:                   return "TOKEN_ITER_STEP";
        case TOKEN_EMPTY_ARRAY:                 return "TOKEN_EMPTY_ARRAY";

        default:                                return "<TOKEN UNKNOWN>";
    }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


CryoDataType CryoDataTypeStringToType(char* typeStr) {
    if(strcmp(typeStr, "int") == 0) {
        return DATA_TYPE_INT;
    } else if(strcmp(typeStr, "float") == 0) {
        return DATA_TYPE_FLOAT;
    } else if(strcmp(typeStr, "string") == 0) {
        return DATA_TYPE_STRING;
    } else if(strcmp(typeStr, "boolean") == 0) {
        return DATA_TYPE_BOOLEAN;
    } else if(strcmp(typeStr, "void") == 0) {
        return DATA_TYPE_VOID;
    } else if(strcmp(typeStr, "null") == 0) {
        return DATA_TYPE_NULL;
    } else if(strcmp(typeStr, "int[]") == 0) {
        return DATA_TYPE_INT_ARRAY;
    } else if(strcmp(typeStr, "float[]") == 0) {
        return DATA_TYPE_FLOAT_ARRAY;
    } else if(strcmp(typeStr, "string[]") == 0) {
        return DATA_TYPE_STRING_ARRAY;
    } else if(strcmp(typeStr, "boolean[]") == 0) {
        return DATA_TYPE_BOOLEAN_ARRAY;
    } else {
        fprintf(stderr, "<!> [Token] Error: Unknown data type: %s\n", typeStr);
        return DATA_TYPE_UNKNOWN;
    }
}