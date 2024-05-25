#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <include/init.h>
#include <commands.h>

#define CRYO_VERSION "0.1.0"

char* cryo_logo = 
"############################################################\n"
"#####                                                  #####\n"
"###                                                      ###\n"
"##     .,-:::::   :::::::..   .-:.     ::-.    ...        ##\n"
"##   ,;;;'````'   ;;;;``;;;;   ';;.   ;;;;' .;;;;;;;.     ##\n"
"##   [[[          [[[,/[[['     '[[,[[['  ,[[     \\[[,    ##\n"
"##   $$$          $$$$$$$c        c$$\"   $$$,     $$$     ##\n"
"##   `88bo,__,o,  888b \"88bo,   ,8P\"`   \"888,_ _,88P      ##\n"
"##    \"YUMMMMMP\"  MMMM   \"WMY ,mM\"        \"YMMMMMP\"       ##\n"
"##                                                        ##\n"
"##               Programming Language v0.1.0              ##\n"
"###                                                      ###\n"
"#####                                                  #####\n"
"############################################################\n";

char* cryo_help_menu =
"---------------------------------------------------------------------------------------------------\n"
"| Command            | Alisis         | Description                       | Example               |\n"
"|-------------------------------------------------------------------------------------------------|\n"
"| help               | -h --help      | Show help information             | cryo help             |\n"
"|                                                                                                 |\n"
"| version            | -v --version   | Show version information          | cryo version          |\n"
"|                                                                                                 |\n"
"| build              | -b --build     | Build the project                 | cryo -b               |\n"
"|                                                                                                 |\n"
"| *init              | --init         | Initialize a new project          | cryo --init           |\n"
"|                                                                                                 |\n"
"| *run               | -r --run       | Run the project                   | cryo run              |\n"
"---------------------------------------------------------------------------------------------------\n"
"\n"
" * - Unimplemented\n";

/*
**************************************************
************* Arguments For Commands *************
**************************************************
*/

//
// Command Type
//
CommandType get_command_type(const char* command) {
    if (
        strcmp(command, "help") == 0 || 
        strcmp(command, "-h") == 0 || 
        strcmp(command, "--help") == 0 
    ) {
        return CMD_HELP;
    } else if (
        strcmp(command, "build") == 0 || 
        strcmp(command, "-b") == 0 || 
        strcmp(command, "--build") == 0
    ) {
        return CMD_BUILD;
    } else if (
        strcmp(command, "init") == 0 || 
        strcmp(command, "--init") == 0
    ) {
        return CMD_INIT;
    } else if (
        strcmp(command, "version") == 0 || 
        strcmp(command, "-v") == 0 || 
        strcmp(command, "--version") == 0
    ) {
        return CMD_VERSION;
    } else if (
        strcmp(command, "run") == 0  || 
        strcmp(command, "-r") == 0   || 
        strcmp(command, "--run") == 0
    ) {
        return CMD_RUN;
    } else {
        return CMD_UNKNOWN;
    }
}

//
// Help Args
//
HelpArgs get_help_arg (const char* help_args) {
    if (strcmp(help_args, "help") == 0) {
        return HELP_HELP;
    } else if (strcmp(help_args, "version") == 0) {
        return HELP_VERSION;
    } else if (strcmp(help_args, "build") == 0) {
        return HELP_BUILD;
    } else if (strcmp(help_args, "init") == 0) {
        return HELP_INIT;
    } else if (strcmp(help_args, "run") == 0) {
        return HELP_RUN;
    } else {
        return HELP_UNKNOWN;
    }
}

//
// Build Args
//
BuildArgs get_build_arg(const char* build_args) {
    if (strcmp(build_args, "-s") == 0) {
        return BUILD_SINGLE;
    } else if (strcmp(build_args, "-d") == 0) {
        return BUILD_DIR;
    } else {
        return BUILD_SINGLE;
    }
}

/*
**************************************************
************* Command Implementations *************
**************************************************
*/

//
// Help Command
//
void help_command(void) {
    printf("\n%s\n", cryo_logo);
    printf("\n%s\n", cryo_help_menu);
}


//
// Version Command
void help_version(void) {
    printf("Cryo Programming Language v%s\n", CRYO_VERSION);
}


//
// Help wtih Command
// `help <command>`
//
void help_with_command(char* help_args) {
    if (help_args != NULL) {
        HelpArgs arg = get_help_arg(help_args);
        switch (arg) {
            case HELP_HELP:
                help_command();
                break;
            case HELP_VERSION:
                help_version();
                break;
            case HELP_BUILD:
                printf("Help for build command.\n");
                break;
            case HELP_INIT:
                printf("Help for init command.\n");
                break;
            case HELP_RUN:
                printf("Help for run command.\n");
                break;
            case HELP_UNKNOWN:
                printf("Unknown help argument: %s\n", help_args);
                break;
            default:
                printf("Unknown help argument: %s\n", help_args);
                break;
        }
    } else {
        help_command();
    }

}

//
// Build Command
//
void build_program_(int argc, char* argv[]) {
    // Implementation of build_program
    printf("Build command executed.\n");
    // Handle arguments if any
    if (argc > 1) {
        printf("Building: %s\n", argv[1]);
    }
}


//
// Main Entry Point for the CLI Application
// `execute_command(argc - 1, &argv[1]);`
//
void execute_command(int argc, char* argv[]) {
    if (argc == 0) {
        printf("No command provided.\n");
        help_command();
        return;
    }
    
    CommandType cmd_type = get_command_type(argv[0]);
    switch (cmd_type) {
        case CMD_HELP:
            help_with_command(argv[1]);
            break;
        case CMD_BUILD:
            build_program_(argc, argv);
            break;
        case CMD_INIT:
            init_command();
            break;
        case CMD_UNKNOWN:
            printf("Unknown command: %s\n", argv[0]);
            break;
        default:
            printf("Unknown command: %s\n", argv[0]);
            break;
    }
}



void todo() {
    printf("TODO: Implement this function.\n");
}