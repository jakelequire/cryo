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
#ifndef CONSOLE_COLORS_H
#define CONSOLE_COLORS_H

// ANSI Color codes
#define COLOR_RESET "\033[0m"
#define BOLD "\033[1m"
#define GREEN "\033[32m"
#define CYAN "\033[36m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RED "\033[31m"
#define MAGENTA "\033[35m"
#define WHITE "\033[37m"
#define BLACK "\033[30m"
#define GRAY "\033[90m"
#define DARK_GRAY "\033[90m"

#define LIGHT_GRAY "\033[37m"
#define LIGHT_RED "\033[91m"
#define LIGHT_GREEN "\033[92m"
#define LIGHT_YELLOW "\033[93m"
#define LIGHT_BLUE "\033[94m"
#define LIGHT_MAGENTA "\033[95m"
#define LIGHT_CYAN "\033[96m"
#define LIGHT_WHITE "\033[97m"

#define BG_BLACK "\033[40m"
#define BG_RED "\033[41m"
#define BG_GREEN "\033[42m"
#define BG_YELLOW "\033[43m"
#define BG_BLUE "\033[44m"
#define BG_MAGENTA "\033[45m"
#define BG_CYAN "\033[46m"
#define BG_WHITE "\033[47m"

#define BG_LIGHT_GRAY "\033[100m"
#define BG_LIGHT_RED "\033[101m"
#define BG_LIGHT_GREEN "\033[102m"
#define BG_LIGHT_YELLOW "\033[103m"
#define BG_LIGHT_BLUE "\033[104m"
#define BG_LIGHT_MAGENTA "\033[105m"
#define BG_LIGHT_CYAN "\033[106m"
#define BG_LIGHT_WHITE "\033[107m"

#define BG_RESET "\033[49m"
#define ITALIC "\033[3m"

/*

│
┤
├
┼
┐
└
─
┬
┴
┌
┘

╡
╢
╖
╕
╣
║
╗
╝
╜
╛

╞
╟
╚
╔
╩
╦
╠
═
╬
╧
╨
╤
╥
╙
╘
╒
╓
╫
╪
*/

#endif // CONSOLE_COLORS_H
