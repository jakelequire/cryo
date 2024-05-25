#include <stdio.h>
#include <string.h>

#include "commands.h"

int main(int argc, char** argv) {

    if (argc > 1) {
        // <DEBUG> printf("Arguments:\n");
        // <DEBUG> for (int i = 1; i < argc; i++) {
        // <DEBUG>    printf("  %s\n", argv[i]);
        // <DEBUG> }

        if ((strcmp(argv[1], "help")) == 0 || (strcmp(argv[1], "-h") == 0))
        {
            print_help();
        }
        else if ((strcmp(argv[1], "build") == 0) || (strcmp(argv[1], "-b") == 0))
        {
            if (argc > 2) {
                int length = 0;
                for (int i = 2; i < argc; i++) {
                    length += strlen(argv[i]) + 1; // +1 for space or null terminator
                }

                char* args = (char*)malloc(length);
                if (!args) {
                    printf("Memory allocation error\n");
                    return 1;
                }

                args[0] = '\0'; // Initialize the string
                for (int i = 2; i < argc; i++) {
                    strcat(args, argv[i]);
                    if (i < argc - 1) {
                        strcat(args, " "); // Add space between arguments
                    }
                }

                // Pass the concatenated string to the build function
                build_program(args);
                free(args);
            } else {
                build_help_message();
            }
        }
        else {
            printf("Unknown command: %s\n", argv[1]);
        }

    } else {
        print_help();
    }

    return 0;
}



/*
    interface Command {

    }


    interface SubCommand {

    }

*/

void command(char* command) {

}
