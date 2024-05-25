#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <helpmessages.h>

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
"| *version           | -v --version   | Show version information          | cryo version          |\n"
"|                                                                                                 |\n"
"| build              | -b --build     | Build the project                 | cryo -b               |\n"
"|                                                                                                 |\n"
"| *init              | --init         | Initialize a new project          | cryo --init           |\n"
"|                                                                                                 |\n"
"| *run               | -r --run       | Run the project                   | cryo run              |\n"
"---------------------------------------------------------------------------------------------------\n"
"\n"
" * - Unimplemented\n";

CommandType get_command_type(const char* command) {
    if (strcmp(command, "help") == 0) {
        return CMD_HELP;
    } else if (strcmp(command, "build") == 0) {
        return CMD_BUILD;
    } else {
        return CMD_UNKNOWN;
    }
}

void help_command() {
    printf("\n%s\n", cryo_logo);
    printf("\n%s\n", cryo_help_menu);
}

void help_with_command(int argc, char* argv[]) {
    // Implementation of help_with_command
    printf("Help command executed.\n");
    // Handle arguments if any
    if (argc > 1) {
        printf("Help for: %s\n", argv[1]);
    }
}

void build_program_(int argc, char* argv[]) {
    // Implementation of build_program
    printf("Build command executed.\n");
    // Handle arguments if any
    if (argc > 1) {
        printf("Building: %s\n", argv[1]);
    }
}

void execute_command(int argc, char* argv[]) {
    if (argc == 0) {
        printf("No command provided.\n");
        return;
    }
    
    CommandType cmd_type = get_command_type(argv[0]);
    switch (cmd_type) {
        case CMD_HELP:
            help_with_command(argc, argv);
            break;
        case CMD_BUILD:
            build_program_(argc, argv);
            break;
        default:
            printf("Unknown command: %s\n", argv[0]);
            break;
    }
}
