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
#include "support_lib.h"

__EXTERN_C__ void printInt(int value)
{
    printf("%d\n", value);
}

__EXTERN_C__ void printIntPtr(int *value)
{
    printf("%d\n", *value);
}

__EXTERN_C__ void printStr(char *value)
{
    printf("%s\n", value);
}

__EXTERN_C__ char *intToString(int value)
{
    char *buffer = (char *)malloc(sizeof(char) * 1024);
    sprintf(buffer, "%d", value);
    return buffer;
}

__EXTERN_C__ void sys_exit(int code)
{
    exit(code);
}
