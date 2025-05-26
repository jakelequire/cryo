/********************************************************************************
 *  Copyright 2025 Jacob LeQuire                                                *
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
#include "diagnostics/diagnostics.h"

#define MAX_CHAR_SIZE 256 * 1024 // 256 KB

const char *intToConstChar(int num)
{
    __STACK_FRAME__
    static char buffer[12];
    snprintf(buffer, sizeof(buffer), "%d", num);
    return buffer;
}

char *intToChar(int num)
{
    __STACK_FRAME__
    char buffer[MAX_CHAR_SIZE];
    snprintf(buffer, sizeof(buffer), "%d", num);
    return buffer;
}

int charToInt(char *str)
{
    __STACK_FRAME__
    int num = 0;
    for (int i = 0; str[i] != '\0'; i++)
    {
        num = num * 10 + str[i] - '0';
    }
    return num;
}

char *concatStrings(const char *str1, const char *str2)
{
    __STACK_FRAME__
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

char *intToSafeString(int value)
{
    // Calculate buffer size needed for an int
    // log10(INT_MAX) + 1 for digits + 1 for sign + 1 for null terminator
    const size_t BUFFER_SIZE = sizeof(int) * 3 + 2;

    char *buffer = (char *)malloc(BUFFER_SIZE);
    if (buffer == NULL)
    {
        return NULL; // Handle allocation failure
    }

    int written = snprintf(buffer, BUFFER_SIZE, "%d", value);

    if (written < 0 || written >= BUFFER_SIZE)
    {
        // snprintf doesn't set errno on overflow, so use fprintf to stderr instead
        fprintf(stderr, "Failed to write int to string: buffer too small\n");
        free(buffer);
        return NULL;
    }

    return buffer;
}

// #### String To Unformatted String
// This function is to strip away any escape characters from a string
// Such as \n, \t, \r, etc. This is useful for printing strings but not their escape characters for debugging
// (Makes a copy of the original string)
char *stringToUFString(const char *str)
{
    __STACK_FRAME__
    if (str == NULL || str[0] == '\0')
    {
        perror("Cannot convert NULL string to UF string");
        return NULL;
    }
    // We make a string copy of the input string, do not modify the original
    char *buffer = strdup(str);
    if (buffer == NULL)
    {
        perror("Failed to allocate memory for UF string");
        return NULL;
    }

    // We iterate through the string and remove any escape characters
    for (int i = 0; buffer[i] != '\0'; i++)
    {
        if (buffer[i] == '\n' || buffer[i] == '\t' || buffer[i] == '\r')
        {
            buffer[i] = 0x20; // Replace with a space
        }
    }

    return buffer;
}
