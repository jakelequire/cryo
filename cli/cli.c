#include <stdio.h>
#include <string.h>

#include "commands.h"

int main(int argc, char** argv) {

    if (argc > 1) {
        // <DEBUG> printf("Arguments:\n");
        // <DEBUG> for (int i = 1; i < argc; i++) {
        // <DEBUG>    printf("  %s\n", argv[i]);
        // <DEBUG> }

        if (strcmp(argv[1], "-h") == 0) {
            print_help();
        } else if (strcmp(argv[1], "build") == 0) {
            if (argc > 2) {
                build_program(argv[2]);
            } else {
                printf("Error: Missing argument for 'build' command\n");
            }
        } else {
            printf("Unknown command: %s\n", argv[1]);
        }
    } else {
        printf("No arguments provided. Use '-h' for help.\n");
    }

    return 0;
}
