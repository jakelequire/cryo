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
#ifndef CRYO_LIB_H
#define CRYO_LIB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Integers
void printIntPtr(int *value);
void printConstIntPtr(const int *value);
void printInt(int value);
// Floats
void printFloat(float value);
// Strings
void printStr(char *value);
void printConstStr(const char *value);
void printStrU(char value);

#endif // CRYO_LIB_H
