#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


void print_help();

void build_help_message();
void build_program(char* args);
void build_single(const char* file);


void cryo_compile(const char* file);
char* read_file(const char* file_path);
void scan_dir();


// Build Command
typedef struct {
    bool mode;
    char* output;
    // More options to come...    
} BuildArgs;


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