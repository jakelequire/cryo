#include <stdio.h>
#include <string.h>
#include <stdlib.h>


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

    // use /src/bin/main.exe as the compiler for now
    
}


void cryo_compile(char* file) {
    char* command = malloc(100);
    sprintf(command, "src/bin/main.exe %s", file);
    system(command);
    free(command);
}
