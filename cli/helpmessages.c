#include <helpmessages.h>


void todo() {
    printf("TODO: Implement this function\n");
}


void help_with_command(char* command) {
    if (strcmp(command, "build") == 0) {
        todo();
    } else if (strcmp(command, "run") == 0) {
        todo();
    } else if (strcmp(command, "help") == 0) {
        todo();
    } else {
        printf("Error: Unknown command\n");
    }
}