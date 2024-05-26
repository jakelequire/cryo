
#include <include/build.h>
#include <compiler.h>

void compile_main(char* file) {
    printf("<compile_main> Compiling main file: %s\n", file);
    // Pass to compiler
    run_compiler(file);
}

void build_command(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: cryo build <path_to_file>\n");
        return;
    }
    BuildArgs build_args = get_build_args(argv[1]);

    if(build_args == BUILD_SINGLE) {
        single_build(argv[2]);
    } else if (build_args == BUILD_DIR) {
        printf("Directory build not yet implemented.\n");
    } else {
        printf("Invalid build arguments provided.\n");
    }

}

BuildArgs get_build_args(const char* build_args) {
    printf("[DEBUG] Build Args: %s\n", build_args);
    if ( strcmp(build_args, "-s") == 0 ||
        strcmp(build_args, "--single") == 0
    ) {
        printf("[DEBUG] Single File Build Selected\n");
        return BUILD_SINGLE;
    } else if (strcmp(build_args, "-d") == 0 ||
        strcmp(build_args, "--dir") == 0
    ) {
        printf("[DEBUG] Directory Build Selected\n");
        return BUILD_DIR;
    } else {
        printf("No build arguments provided. Defaulting to full directory file build.\n");
        return BUILD_SINGLE;
    }
}



void single_build(char* filename) {
    printf("<single_build> Building single file: %s\n", filename);

    // Grab the file being passed
    char* file = readFile(filename);
    if (file == NULL) {
        perror("Failed to read file");
        return;
    }

    printf("<single_build> File contents: %s\n", file);

    // pass to compiler `main.exe`


    free(file);
}


char* readFile(const char* path) {
    FILE* file;
    errno_t err = fopen_s(&file, path, "rb");  // Open the file in binary mode to avoid transformations
    if (err != 0) {
        perror("Could not open file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    // handle fseek error
    fseek(file, 0, SEEK_SET);

    if (length == 0) {
        perror("File is empty");
        fclose(file);
        return NULL;
    }

    char* buffer = (char*)malloc(length + 1);
    if (buffer == NULL) {
        perror("Not enough memory to read file");
        fclose(file);
        return NULL;
    }

    size_t bytesRead = fread(buffer, 1, length, file);
    if (bytesRead < length) {
        perror("Failed to read the full file");
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[length] = '\0';  // Null-terminate the buffer
    fclose(file);
    return buffer;
}

void compile(char* file) {
    printf("<compile> Compiling file: %s\n", file);
    // Pass to compiler
    
    printf("<compile> File compiled.\n");
}

