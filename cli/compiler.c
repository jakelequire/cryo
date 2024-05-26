
#include <compiler.h>

// Function to run the compiler
void run_compiler(const char *input_file) {
    char command[256];
    snprintf(command, sizeof(command), "../src/bin/main.exe %s", input_file);

    FILE *fp;
    char path[1035];

    // Open the command for reading.
    fp = popen(command, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to run compiler command\n");
        exit(EXIT_FAILURE);
    }

    // Read the output a line at a time - output it.
    while (fgets(path, sizeof(path), fp) != NULL) {
        printf("%s", path);
    }

    // Close the process.
    int status = pclose(fp);
    if (status == -1) {
        fprintf(stderr, "Error: Failed to close command stream.\n");
        exit(EXIT_FAILURE);
    } else if (WIFEXITED(status)) {
        if (WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Error: Compiler returned a non-zero exit code: %d\n", WEXITSTATUS(status));
            exit(EXIT_FAILURE);
        }
    }
}









