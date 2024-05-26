#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commands.h>
#include "../src/main.h"

// Command Types
typedef enum {
    CMD_HELP,       // help     (subcommands)
    CMD_BUILD,      // build    (subcommands)
    CMD_INIT,       // init     (subcommands)
    CMD_VERSION,    // version  (no subcommands)
    CMD_RUN,        // run      (subcommands)
    CMD_UNKNOWN     // unknown  (no subcommands)
} CommandType;

// Build Args
typedef enum {
    BUILD_SINGLE,   // -s
    BUILD_DIR       // -d
} BuildArgs;

// Help Args
typedef enum {
    HELP_HELP,      // help
    HELP_VERSION,   // version
    HELP_BUILD,     // build
    HELP_INIT,      // init
    HELP_RUN,       // run
    HELP_UNKNOWN    // unknown
} HelpArgs;


// Function prototypes
CommandType get_command_type(const char* command);

void execute_command(int argc, char* argv[]);
void build_program_(int argc, char* argv[]);
void help_command(void);
void version_command(void);

void help_with_command(char* help_args);


#endif // COMMANDS_H