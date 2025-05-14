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

// Only include these definitions for C, not for C++
#ifndef __cplusplus
// ANSI Color codes
#define COLOR_RESET "\033[0m"
#define BOLD "\033[1m"

#define GREEN "\033[32m"
#define CYAN "\033[36m"
#define BRIGHT_CYAN "\033[96m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RED "\033[31m"
#define BRIGHT_RED "\033[91m"
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
#define UNDERLINE "\033[4m"

// Makes text 2x larger
#define FONT_LARGE "\x1b#3"
// Makes text 1x (normal size)
#define FONT_NORMAL "\x1b#5"

#define TOP_LEFT "┌"
#define TOP_RIGHT "┐"
#define BOTTOM_LEFT "└"
#define BOTTOM_RIGHT "┘"
#define HORIZONTAL "─"
#define VERTICAL "│"
#define T_DOWN "┬"
#define T_UP "┴"
#define T_RIGHT "├"
#define T_LEFT "┤"

// Box drawing characters
#define BOX_TOP_LEFT "╔"
#define BOX_TOP_RIGHT "╗"
#define BOX_BOTTOM_LEFT "╚"
#define BOX_BOTTOM_RIGHT "╝"
#define BOX_HORIZONTAL "═"
#define BOX_VERTICAL "║"
#define BOX_LEFT_T "╠"
#define BOX_RIGHT_T "╣"
#define BOX_VERTICAL_SEP "│"
#else
// When compiling with C++, this header does nothing
// Add a warning message to help developers understand why the header is empty
#ifdef __GNUC__
#warning "consoleColors.h is meant for C only, not C++"
#else
// For non-GCC compilers that might not support #warning
// Static_assert would work in C++11 and above
#if __cplusplus >= 201103L
static_assert(false, "consoleColors.h is meant for C only, not C++");
#endif
#endif
#endif // __cplusplus
/*
    Box Drawing Characters Reference Table
    ====================================

    Basic Box Elements        Heavy/Double Elements
    ------------------       -------------------
    │  Single vertical      ║  Double vertical
    ─  Single horizontal    ═  Double horizontal

    Corner Pieces           Double Corners
    -------------          --------------
    ┌  Top left            ╔  Top left
    ┐  Top right           ╗  Top right
    └  Bottom left         ╚  Bottom left
    ┘  Bottom right        ╝  Bottom right

    T-Junctions            Double T-Junctions
    ------------           -----------------
    ├  Left T              ╠  Left T
    ┤  Right T             ╣  Right T
    ┬  Top T               ╦  Top T
    ┴  Bottom T            ╩  Bottom T

    Mixed Junctions        Special Junctions
    ---------------        -----------------
    ╞  Single-Double L     ╟  Heavy-Light L
    ╡  Single-Double R     ╢  Heavy-Light R
    ╥  Double-Single Up    ╫  Heavy vertical
    ╨  Double-Single Down  ╪  Heavy horizontal
    ╒  Single-Double TL    ╓  Heavy-Light TL
    ╕  Single-Double TR    ╖  Heavy-Light TR
    ╘  Single-Double BL    ╙  Heavy-Light BL
    ╛  Single-Double BR    ╜  Heavy-Light BR

    Intersections
    ------------
    ┼  Single cross
    ╬  Double cross

↖ ↑ ↗
← · →
↙ ↓ ↘
*/

#endif // CONSOLE_COLORS_H