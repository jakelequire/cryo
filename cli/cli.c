#include <stdio.h>
#include <string.h>

#include "commands.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        help_command();
        return 1;
    }

    // Pass all arguments except the program name
    execute_command(argc - 1, &argv[1]);

    return 0;
}
