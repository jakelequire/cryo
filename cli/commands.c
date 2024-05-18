#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#include "commands.h"

void print_help() {
    printf("\n#################################################\n");
    printf("##                                             ##\n");
    printf("##  Cryo Programming Language  | CLI Tools     ##\n");
    printf("##  Version 0.1.0                              ##\n");
    printf("##                                             ##\n");
    printf("#################################################\n\n");
    printf("Commands:\n");
    printf("-------------------------------------------\n\n");
    
    printf(" Print this help message\n\n");
    printf("  - help  | cryo <help | -h>\n");
    printf("\n");
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

    printf("\n Build a cryo project\n\n");
    printf("  - build  | cryo <build | -b> {args?} \n\n");
    printf("    {args}:\n");
    printf("      - -s Single File: Only compile one individual file\n");
    printf("        cryo build -s <file>\n\n");
    printf("\n");
    printf("-------------------------------------------\n");
}


void build_program(char* args) {
    printf("Building program with args: %s\n", args);

    if (args[0] == '-') {
        switch (args[1]) {
            case 's': {
                const char* file_arg = args + 2;
                while (*file_arg == ' ') {
                    file_arg++;
                }

                if (*file_arg == '\0') {
                    printf("\n-------------------------------------------\n");
                    printf("\nError: Missing argument for single file build\n\n");
                    printf("*required argument: <file>\n");
                    printf("Example: cryo build -s ./example.cryo\n");
                } else {
                    printf("file_arg: %s\n", file_arg);
                    build_single(file_arg);
                }
                break;
            }
            case 'd':
                scan_dir();
                break;
            default:
                printf("Error: Unknown argument\n");
                break;
        }
    } else {
        printf("Error: Unknown argument\n");
    }
}

void build_single(const char* file) {
    printf("Building single file: %s\n", file);

    char* _file = (char*)file;

    if(cryo_compile(_file) == 0) {
        printf("Build successful\n");
    } else {
        printf("Build failed\n");
    }

}


void scan_dir() {
    printf("Scanning directory for cryo files\n");

    char* local_path = getenv("PWD");
    if (local_path == NULL) {
        printf("Error: PWD environment variable not set\n");
        return;
    } else {
        printf("PWD: %s\n", local_path);

        // Open the directory
        DIR* dir;
        struct dirent* ent;

        if ((dir = opendir(local_path)) != NULL) {
            while ((ent = readdir(dir)) != NULL) {
                printf("  %s\n", ent->d_name);
            }
            closedir(dir);
        } else {
            printf("Error: Could not open directory\n");
        }
    }
}


int cryo_compile(char* file) {
    char command[512];  // Increased size for the complete command
    char* cryo_path = getenv("CRYO_PATH");
    if (cryo_path == NULL) {
        printf("Error: CRYO_PATH environment variable not set\n");
        return 1;
    } else {
        printf("CRYO_PATH: %s\n", cryo_path);
    }

    char src_path[256] = "";
    snprintf(src_path, sizeof(src_path), "%s/src/bin/main.exe", cryo_path);
    
    snprintf(command, sizeof(command), "%s %s", src_path, file);

    // Open the command for reading.
    FILE* fp = popen(command, "r");
    if (fp == NULL) {
        printf("Failed to run command\n");
        return 1;
    }

    // Read the output a line at a time and print to the console.
    char output[1024];
    while (fgets(output, sizeof(output) - 1, fp) != NULL) {
        printf("%s", output);
    }

    // Close the file pointer.
    pclose(fp);
    return 0;
}
