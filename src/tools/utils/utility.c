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
#include "tools/utils/utility.h"

const char *intToConstChar(int num)
{
    static char buffer[12];
    snprintf(buffer, sizeof(buffer), "%d", num);
    return buffer;
}

char *intToChar(int num)
{
    char buffer[12];
    snprintf(buffer, sizeof(buffer), "%d", num);
    return buffer;
}

int charToInt(char *str)
{
    int num = 0;
    for (int i = 0; str[i] != '\0'; i++)
    {
        num = num * 10 + str[i] - '0';
    }
    return num;
}

char *concatStrings(const char *str1, const char *str2)
{
    char *result = (char *)malloc(strlen(str1) + strlen(str2) + 1);
    if (result == NULL)
    {
        perror("Failed to allocate memory for concatenated string");
        return NULL;
    }
    strcpy(result, str1);
    strcat(result, str2);
    return result;
}
