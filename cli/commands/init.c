#include "include/init.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(name, mode) _mkdir(name)
#endif

#define CRYO_VERSION "0.1.0"
/*
# This is the cryo.init file that is created when a new project is initialized.
# This file is used to store the project's name, version, and other information.
# Most of the information in this file is used by the Cryo compiler to build the project.
# The Cryo compiler will look for this file in the root directory of the project.

# User defined variables
PROJECT_NAME: "my_project";
VERSION: "0.1.0";
AUTHOR: "John Doe";
DESCRIPTION: "This is a new Cryo project.";

# Compiler defined variables
CRYO_VERSION: "0.1.0";
ENTRY: "./src/main.cryo";
CONFIG: "./cryo.config.json";

*/


char* cryo_init_data =
"# This is the cryo.init file that is created when a new project is initialized.\n"
"# This file is used to store the project's name, version, and other information.\n"
"# Most of the information in this file is used by the Cryo compiler to build the project.\n"
"# The Cryo compiler will look for this file in the root directory of the project.\n"
"\n"
"# User defined variables\n"
"PROJECT_NAME: \"my_project\";\n"
"VERSION: \"0.1.0\";\n"
"AUTHOR: \"John Doe\";\n"
"DESCRIPTION: \"This is a new Cryo project.\";\n"
"\n"
"# Compiler defined variables\n"
"CRYO_VERSION: \"0.1.0\";\n"
"ENTRY: \"./src/main.cryo\";\n"
"CONFIG: \"./cryo.config.json\";\n"
"\n";

char* cryo_config_data =
    "{\n"
    "    \"cryo_version\": \"0.1.0\",\n"
    "    \"entry\": \"./src/main.cryo\",\n"
    "    \"debugging\": \"verbose\",\n"
    "    \"outdir\": \"./build\",\n"
    "    \"dependencies\": {\n\n"
    "    }\n"
    "}\n";

char* cryo_main_data =
"\n"
"public fn main() -> void {\n"
"    println(\"Hello, World!\");\n"
"    return;\n"
"}\n"
"\n";

void init_command(void) {
    printf("Init command executed.\n");
    open_root_dir();
}

void create_cryo_config(void) {
    create_file("cryo.config.json", cryo_config_data);
}

void create_cryo_main(void) {
    create_file("src/main.cryo", cryo_main_data);
}

void create_cryo_init(void) {
    create_file("cryo.init", cryo_init_data);
}

void create_directory(const char* name) {
    struct stat st = {0};

    if (stat(name, &st) == -1) {
        mkdir(name, 0700);
        printf("Directory created: %s\n", name);
    }
}

void open_root_dir(void) {
    DIR *dir;
    struct dirent *ent;

    if ((dir = opendir(".")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            printf("\nOpening directory: %s\n", ent->d_name);
            create_cryo_config();
            create_directory("src");
            create_cryo_main();
            create_cryo_init();
        }
        closedir(dir);
    } else {
        perror("Error opening dir   ectory");
        exit(1);
    }
}

void create_file(char* filename, char* data) {
    /*DEBUGGING*/ printf("Creating file: %s\n", filename);
    FILE *file;
#ifdef _WIN32
    if (fopen_s(&file, filename, "w") != 0) {
        printf("Error creating file: %s\n", filename);
        exit(1);
    }
#else 
    file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error creating file: %s\n", filename);
        exit(1);
    }
#endif

    fprintf(file, "%s", data);
    fclose(file);
    /*DEBUGGING*/ printf("File created: %s\n", filename);
}
