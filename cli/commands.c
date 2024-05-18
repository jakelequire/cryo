#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "commands.h"

void print_help() {
    printf("-------------------------------------------\n");
    printf("Cryo Programming Language  | CLI Tools\n");
    printf("Version 0.1.0\n");
    printf("-------------------------------------------\n");
    printf("Commands:\n");
    printf("--------------------\n");
    
    printf("  - help  | cryo <help>\n");
    printf("  usage: cryo -h \n\n");
    printf("  Print this help message\n\n");

    printf("--------------------\n");

    printf("  - build  | cryo <build> {args?} \n");
    printf("  usage: cryo build -v\n\n");
    printf("    Build a cryo project\n");
    printf("    {args}:\n");
    printf("      - -o <output file>: The output file\n");
    printf("      - -c: Compile only\n");
    printf("      - -s: Compile and assemble only\n");
    printf("      - -r: Compile, assemble, and link only\n");
    printf("      - -d: Print debug information\n");
    printf("      - -v: Print verbose information\n");

    printf("-------------------------------------------\n");
}


void build_program(char* args) {
    printf("Building program with args: %s\n", args);

    // For now, let's grab a file manually, and pass it to the compiler.
    cryo_compile("./src/tests/data/test1.cryo");
}


void cryo_compile(char* file) {
    char command[512];  // Increased size for the complete command
    char* cryo_path = getenv("CRYO_PATH");
    if (cryo_path == NULL) {
        printf("Error: CRYO_PATH environment variable not set\n");
        return;
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
        return;
    }

    // Read the output a line at a time and print to the console.
    char output[1024];
    while (fgets(output, sizeof(output) - 1, fp) != NULL) {
        printf("%s", output);
    }

    // Close the file pointer.
    pclose(fp);
}

