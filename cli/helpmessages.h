#ifndef HELPMESSAGES_H
#define HELPMESSAGES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commands.h>

typedef enum {
    CMD_HELP,
    CMD_BUILD,
    CMD_UNKNOWN
} CommandType;
CommandType get_command_type(const char* command);
void execute_command(int argc, char* argv[]);
void build_program_(int argc, char* argv[]);
void help_with_command(int argc, char* argv[]);
void help_command();


#endif // HELPMESSAGES_H