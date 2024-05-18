#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/*
    - Command struct
        - name: The name of the command
        - func: The function to call when the command is executed
        - args: A string describing the arguments the command takes
        - description: A description of what the command does


    - Args struct
        - commands: An array of Command structs
        - args: A string describing the arguments the program takes

    === Commands ===
    - help
        - Print a help message
    - build
        - Build the program
        {args}:
            - -o <output file>: The output file
            - -c: Compile only
            - -s: Compile and assemble only
            - -r: Compile, assemble, and link only
            - -d: Print debug information
            - -v: Print verbose information

*/

void print_help();
void build_program(char* args);



typedef struct {
    char* name;
    void (*func)();
    char* args;
    char* description;
} Command;

typedef struct {
    Command* commands;
    char* args;
} Args;


#endif