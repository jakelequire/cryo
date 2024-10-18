/********************************************************************************
 *  Copyright 2024 Jacob LeQuire                                                *
 *  SPDX-License-Identifier: Apache-2.0                                         *
 *    Licensed under the Apache License, Version 2.0 (the "License");           *
 *    you may not use this file except in compliance with the License.          *
 *    You may obtain a copy of the License at                                   *
 *                                                                              *
 *    http://www.apache.org/licenses/LICENSE-2.0                                *
 *                                                                              *
 *    Unless required by applicable law or agreed to in writing, software       *
 *    distributed under the License is distributed on an "AS IS" BASIS,         *
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
 *    See the License for the specific language governing permissions and       *
 *    limitations under the License.                                            *
 *                                                                              *
 ********************************************************************************/
#include "cli/init.h"

// <getInitArgs>
InitArgs getInitArgs(char *arg)
{
    if (strcmp(arg, "-dir") == 0)
        return INIT_ARG_DIR;

    return INIT_ARG_UNKNOWN;
}
// </getInitArgs>

// <executeInitCmd>
void executeInitCmd(char *argv[])
{
    char *argument = argv[2];
    if (argument == NULL)
    {
        printf("Initializing new Cryo project...\n");
        // executeInit();
    }

    InitArgs arg = getInitArgs(argument);
    switch (arg)
    {
    case INIT_ARG_DIR:
        // Execute Command
        break;

    case INIT_ARG_UNKNOWN:
        // Handle Unknown Command
        break;

    default:
        // Do something
    }
}
// </executeInitCmd>

void executeInit()
{
    open_root_dir();
    printf("Cryo project initialized.\n");
}

char *cryo_init_data =
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

char *cryo_config_data =
    "{\n"
    "    \"cryo_version\": \"0.1.0\",\n"
    "    \"entry\": \"./src/main.cryo\",\n"
    "    \"debugging\": \"verbose\",\n"
    "    \"outdir\": \"./build\",\n"
    "    \"dependencies\": {\n\n"
    "    }\n"
    "}\n";

char *cryo_main_data =
    "\n"
    "public function main() -> void {\n"
    "    printStr(\"Hello, World!\");\n"
    "    return;\n"
    "}\n"
    "\n";

void create_cryo_config()
{
    create_file("cryo.config.json", cryo_config_data);
}

void create_cryo_main()
{
    create_file("src/main.cryo", cryo_main_data);
}

void create_cryo_init()
{
    create_file("cryo.init", cryo_init_data);
}

void create_directory(const char *name)
{
    struct stat st = {0};

    if (stat(name, &st) == -1)
    {
        mkdir(name, 0700);
        printf("Directory created: %s\n", name);
    }
}

void open_root_dir()
{
    DIR *dir;
    struct dirent *ent;

    if ((dir = opendir(".")) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            printf("\nOpening directory: %s\n", ent->d_name);
            create_cryo_config();
            create_directory("src");
            create_cryo_main();
            create_cryo_init();
        }
        closedir(dir);
    }
    else
    {
        perror("Error opening dir   ectory");
        exit(1);
    }
}

void create_file(char *filename, char *data)
{
    FILE *file;
#ifdef _WIN32
    if (fopen_s(&file, filename, "w") != 0)
    {
        printf("Error creating file: %s\n", filename);
        exit(1);
    }
#else
    file = fopen(filename, "w");
    if (file == NULL)
    {
        printf("Error creating file: %s\n", filename);
        exit(1);
    }
#endif

    fprintf(file, "%s", data);
    fclose(file);
}
