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
#include "utils/support.h"

/// @brief      Duplicate a string in memory.
/// @param s    The string to duplicate.
/// @return     A pointer to the duplicated string.
char *strdup(const char *s)
{
    size_t slen = strlen(s);
    char *result = malloc(slen + 1);
    if (result == NULL)
    {
        return NULL;
    }

    memcpy(result, s, slen + 1);
    return result;
}

/// @brief      Duplicate a string in memory with a maximum length.
/// @param s    The string to duplicate.
/// @param n    The maximum length of the string.
/// @return     A pointer to the duplicated string.
char *strndup(const char *s, size_t n)
{
    size_t slen = strlen(s);
    size_t len = slen < n ? slen : n;
    char *result = malloc(len + 1);
    if (result == NULL)
    {
        return NULL;
    }

    memcpy(result, s, len);
    result[len] = '\0';
    return result;
}
