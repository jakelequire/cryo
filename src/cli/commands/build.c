
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
#include "cli/build.h"

void compile_main(char* file) {
    printf("<compile_main> Compiling main file: %s\n", file);
    // Pass to compiler
    run_compiler(file);
}

void build_command(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: cryo build <path_to_file>\n");
        return;
    }
    BuildArgs build_args = get_build_args(argv[1]);

    if(build_args == BUILD_SINGLE) {
        single_build(argv[2]);
    } else if (build_args == BUILD_DIR) {
        printf("Directory build not yet implemented.\n");
    } else {
        printf("Invalid build arguments provided.\n");
    }

}

BuildArgs get_build_args(const char* build_args) {
    printf("[DEBUG] Build Args: %s\n", build_args);
    if ( strcmp(build_args, "-s") == 0 ||
        strcmp(build_args, "--single") == 0
    ) {
        printf("[DEBUG] Single File Build Selected\n");
        return BUILD_SINGLE;
    } else if (strcmp(build_args, "-d") == 0 ||
        strcmp(build_args, "--dir") == 0
    ) {
        printf("[DEBUG] Directory Build Selected\n");
        return BUILD_DIR;
    } else {
        printf("No build arguments provided. Defaulting to full directory file build.\n");
        return BUILD_SINGLE;
    }
}



void single_build(char* filename) {
    printf("<single_build> Building single file, path: %s\n", filename);

    // Grab the file being passed
    char* file = readFile(filename);
    if (file == NULL) {
        perror("Failed to read file");
        return;
    }

    printf("<single_build> File contents: %s\n", file);

    // pass to compiler `main.exe`
    compile_main(filename);

    free(file);
}


void compile(char* file) {
    printf("<compile> Compiling file: %s\n", file);
    // Pass to compiler
    
    printf("<compile> File compiled.\n");
}

