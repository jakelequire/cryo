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
#include "main.h"

int main(int argc, char *argv[])
{
    // Initialize the compiler settings
    CompilerSettings settings = getCompilerSettings(argc, argv);
    logCompilerSettings(&settings);

    // Check if the input file exists
    const char *filePath = settings.inputFilePath;
    if (!fileExists(filePath))
    {
        fprintf(stderr, "Error: File not found: %s\n", filePath);
        return 1;
    }

    // Compile the file
    int compilerResult = cryoCompiler(filePath, &settings);
    if (compilerResult != 0)
    {
        CONDITION_FAILED;
        return 1;
    }

    return 0;
}
