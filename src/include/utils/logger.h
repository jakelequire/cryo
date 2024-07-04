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




#endif // LOGGER_H
