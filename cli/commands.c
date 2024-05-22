#define _CRT_SECURE_NO_WARNINGS

#ifdef _WIN32
#include <windows.h>
#define popen _popen
#define pclose _pclose
#else
#include <unistd.h>
#include <dirent.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands.h"

void print_help() {
    printf("\n#################################################\n");
    printf("##                                             ##\n");
    printf("##  Cryo Programming Language  | CLI Tools     ##\n");
    printf("##  Version 0.1.0                              ##\n");
    printf("##                                             ##\n");
    printf("#################################################\n\n");
    printf("Commands:\n");
    printf("-------------------------------------------\n\n");
    
    printf(" Print this help message\n\n");
    printf("  - help  | cryo <help | -h>\n");
    printf("\n");
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

    printf("\n Build a cryo project\n\n");
    printf("  - build  | cryo <build | -b> {args?} \n\n");
    printf("    {args}:\n");
    printf("      - -s Single File: Only compile one individual file\n");
    printf("        cryo build -s <file>\n\n");
    printf("\n");
    printf("-------------------------------------------\n");
}

void build_program(char* args) {
    if (args[0] == '-') {
        switch (args[1]) {
            case 's': {
                const char* file_arg = args + 2;
                while (*file_arg == ' ') {
                    file_arg++;
                }

                if (*file_arg == '\0') {
                    printf("\n-------------------------------------------\n");
                    printf("\nError: Missing argument for single file build\n\n");
                    printf("*required argument: <file>\n");
                    printf("Example: cryo build -s ./example.cryo\n");
                    printf("\n-------------------------------------------\n\n");
                } else {
                    // printf("\nfile: %s\n", file_arg);
                    build_single(file_arg);
                }
                break;
            }
            case 'd':
                scan_dir();
                break;
            default:
                printf("Error: Unknown argument\n");
                break;
        }
    } else {
        printf("Error: Unknown argument\n");
    }
}

void build_single(const char* file) {
    // DEBUG printf("Building single file: %s\n", file);
    char* _file = (char*)file;

    cryo_compile(_file);
}

void scan_dir() {
    printf("Scanning directory for cryo files\n");

    char* local_path;
    size_t len;
    _dupenv_s(&local_path, &len, "PWD");

    if (local_path == NULL) {
        printf("Error: PWD environment variable not set\n");
        return;
    } else {
        printf("PWD: %s\n", local_path);

#ifdef _WIN32
        WIN32_FIND_DATA findFileData;
        HANDLE hFind = FindFirstFile("*", &findFileData);

        if (hFind == INVALID_HANDLE_VALUE) {
            printf("Error: Could not open directory\n");
        } else {
            do {
                printf("  %s\n", findFileData.cFileName);
            } while (FindNextFile(hFind, &findFileData) != 0);
            FindClose(hFind);
        }
#else
        DIR* dir;
        struct dirent* ent;

        if ((dir = opendir(local_path)) != NULL) {
            while ((ent = readdir(dir)) != NULL) {
                printf("  %s\n", ent->d_name);
            }
            closedir(dir);
        } else {
            printf("Error: Could not open directory\n");
        }
#endif
    }
    free(local_path);
}

char* read_file(const char* file_path) {
    FILE* file;
    fopen_s(&file, file_path, "rb");  // Use "rb" to read in binary mode
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open file %s\n", file_path);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(file_size + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    size_t read_size = fread(buffer, 1, file_size, file);
    buffer[read_size] = '\0';

    fclose(file);

    // Check and strip BOMs if present
    if (read_size >= 3 && (unsigned char)buffer[0] == 0xEF && (unsigned char)buffer[1] == 0xBB && (unsigned char)buffer[2] == 0xBF) {
        // UTF-8 BOM
        memmove(buffer, buffer + 3, read_size - 2);
    } else if (read_size >= 2 && (unsigned char)buffer[0] == 0xFF && (unsigned char)buffer[1] == 0xFE) {
        // UTF-16 LE BOM
        memmove(buffer, buffer + 2, read_size - 1);
    } else if (read_size >= 2 && (unsigned char)buffer[0] == 0xFE && (unsigned char)buffer[1] == 0xFF) {
        // UTF-16 BE BOM
        memmove(buffer, buffer + 2, read_size - 1);
    }

    return buffer;
}

void cryo_compile(const char* file) {
    char* buffer = read_file(file);
    if (buffer == NULL) {
        // Error message already printed in read_file
        return;
    }

    char* cryo_path;
    size_t len;
    _dupenv_s(&cryo_path, &len, "CRYO_PATH");

    if (cryo_path == NULL) {
        fprintf(stderr, "Error: CRYO_PATH environment variable not set\n");
        free(buffer);
        return;
    }

    printf("[DEBUG] Cryo Environment Found, Continuing Compiling...\n");
    printf("---------------------------------------------------\n\n");

    size_t src_path_len = snprintf(NULL, 0, "%s/src/bin/main.exe", cryo_path) + 1;
    char* src_path = (char*)malloc(src_path_len);
    if (src_path == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        free(buffer);
        return;
    }
    snprintf(src_path, src_path_len, "%s/src/bin/main.exe", cryo_path);

    size_t command_len = snprintf(NULL, 0, "%s %s", src_path, file) + 1;
    char* command = (char*)malloc(command_len);
    if (command == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        free(buffer);
        free(src_path);
        return;
    }
    snprintf(command, command_len, "%s %s", src_path, file);

    FILE* pipe_fp = popen(command, "r");
    if (pipe_fp == NULL) {
        fprintf(stderr, "Error: Failed to run command\n");
        free(buffer);
        free(src_path);
        free(command);
        return;
    }

    char output[1024];
    while (fgets(output, sizeof(output) - 1, pipe_fp) != NULL) {
        printf("%s", output);
    }

    pclose(pipe_fp);
    free(buffer);
    free(src_path);
    free(command);
}
