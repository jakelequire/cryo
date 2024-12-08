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
#include "tools/utils/c_logger.h"

void logMessage(const char *type, int line, const char *module, const char *message, ...)
{
    va_list args;
    va_start(args, message);
    printf("[%-5s] @%-7d { %-7s}  ", typeBufferFormatter(type), line, module);
    vprintf(message, args);
    printf("\n");
    va_end(args);
    return;
}

// The type string being passed in is one of these options: INFO, ERROR, WARN, DEBUG
const char *typeBufferFormatter(const char *type)
{
    const char *buffer = (const char *)malloc(sizeof(char) * 1024);

    if (strcmp(type, "INFO") == 0)
    {
        sprintf((char *)buffer, "%s%s%s", GREEN, type, COLOR_RESET);
    }
    else if (strcmp(type, "ERROR") == 0)
    {
        sprintf((char *)buffer, "%s%s%s", RED, type, COLOR_RESET);
    }
    else if (strcmp(type, "WARN") == 0)
    {
        sprintf((char *)buffer, "%s%s%s", YELLOW, type, COLOR_RESET);
    }
    else if (strcmp(type, "DEBUG") == 0)
    {
        sprintf((char *)buffer, "%s%s%s", CYAN, type, COLOR_RESET);
    }
    else if (strcmp(type, "CRITICAL") == 0)
    {
        sprintf((char *)buffer, "%s%s%s", LIGHT_RED, type, COLOR_RESET);
    }
    else
    {
        sprintf((char *)buffer, "%s%s%s", WHITE, type, COLOR_RESET);
    }

    return buffer;
}
