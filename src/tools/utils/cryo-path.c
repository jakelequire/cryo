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
#include "tools/utils/cryo-path.h"

typedef struct CompilerSignature
{
    const char *compilerPath;
    const char *binPath;
    const char *version;
} CompilerSignature;

void initCompilerPathExe(const char *rootPath)
{
    printf("\n\n STARTING CRYO PATH INITIALIZATION\n\n");
    // RootPath + /bin/compiler
    char *compilerPath = (char *)malloc(sizeof(char) * 1024);
    sprintf(compilerPath, "%s/bin/compiler", rootPath);

    // RootPath + /bin
    char *binpath = (char *)malloc(sizeof(char) * 1024);
    sprintf(binpath, "%s/bin", rootPath);

    CompilerSignature *signature = (CompilerSignature *)malloc(sizeof(CompilerSignature));
    signature->compilerPath = compilerPath;
    signature->version = "0.0.1";

    // Create the binary
    createCryoPathBinary(binpath);
}

// This function will create an executable binary that outputs the string of where the
// Cryo Compiler is located. This is useful for other tools that need to know where the
// compiler is located.
void createCryoPathBinary(const char *binpath)
{
    // Create the C source file path
    char *sourcePath = (char *)malloc(sizeof(char) * 1024);
    sprintf(sourcePath, "%s/cryo-path.c", binpath);

    // Create the binary path
    char *binaryPath = (char *)malloc(sizeof(char) * 1024);
    sprintf(binaryPath, "%s/cryo-path", binpath);

    // Create the C source file
    FILE *file = fopen(sourcePath, "w");
    if (!file)
    {
        fprintf(stderr, "Error: Failed to create source file: %s\n", sourcePath);
        free(sourcePath);
        free(binaryPath);
        return;
    }

    // Write a C program that simply outputs the compiler path
    fprintf(file, "#include <stdio.h>\n\n"
                  "int main() {\n"
                  "    printf(\"%s/compiler\\n\");\n" // Add the /compiler suffix
                  "    return 0;\n"
                  "}\n",
            binpath);

    fclose(file);

    // Compile the C program
    char *compileCmd = (char *)malloc(sizeof(char) * 1024);
    char *chmodCmd = (char *)malloc(sizeof(char) * 1024);
    char *sourceBinary = (char *)malloc(sizeof(char) * 1024);
    char *targetLink = (char *)malloc(sizeof(char) * 1024);

    sprintf(compileCmd, "gcc -O2 -o %s %s", binaryPath, sourcePath);

    if (system(compileCmd) != 0)
    {
        fprintf(stderr, "Error: Compilation failed\n");
        goto cleanup;
    }

    // Make executable
    sprintf(chmodCmd, "chmod +x %s", binaryPath);
    system(chmodCmd);
    free(chmodCmd);

    // Create path strings

    sprintf(sourceBinary, "%s/cryo-path", binpath);
    sprintf(targetLink, "/usr/local/bin/cryo-path");

    // Remove existing symlink if it exists
    unlink(targetLink); // It's okay if this fails (file might not exist)

    // Create new symlink
    if (symlink(sourceBinary, targetLink) != 0)
    {
        // Try using sudo if regular symlink fails
        char *symlinkCmd = (char *)malloc(sizeof(char) * 1024);
        sprintf(symlinkCmd, "sudo ln -sf %s %s", sourceBinary, targetLink);
        if (system(symlinkCmd) != 0)
        {
            fprintf(stderr, "Error: Failed to create symbolic link in /usr/local/bin\n");
        }
        free(symlinkCmd);
    }

    free(sourceBinary);
    free(targetLink);

cleanup:
    // Clean up temporary file
    remove(sourcePath);

    // Free allocated memory
    free(sourcePath);
    free(binaryPath);
    free(compileCmd);
}
