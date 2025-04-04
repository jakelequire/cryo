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
#include "../include/commands.h"

HelpOptions *parse_help_options(int argc, char *argv[], int start_index)
{
    HelpOptions *options = (HelpOptions *)malloc(sizeof(HelpOptions));
    if (!options)
        return NULL;

    // Default to general help
    options->command = CLI_HELP;

    // If we have a command argument
    if (start_index < argc)
    {
        // Convert the command argument to the corresponding CLI_ARGS
        if (stringCompare(argv[start_index], "build"))
        {
            options->command = CLI_BUILD;
        }
        else if (stringCompare(argv[start_index], "init"))
        {
            options->command = CLI_INIT;
        }
        else if (stringCompare(argv[start_index], "build-compiler"))
        {
            options->command = CLI_BUILD_COMPILER;
        }
        else if (stringCompare(argv[start_index], "clean-compiler"))
        {
            options->command = CLI_CLEAN_COMPILER;
        }
        else if (stringCompare(argv[start_index], "version"))
        {
            options->command = CLI_VERSION;
        }
        // Add other commands as needed
    }

    return options;
}

void exe_CLI_help(void)
{
    // Title and Version
    printf("\n" BOLD CYAN "Cryo Build System" COLOR_RESET GRAY ITALIC " %s%s\n", __CLI_VERSION__, COLOR_RESET);
    printf("A modern build system for Cryo projects\n\n");

    // Basic Usage
    printf(BOLD "USAGE\n" COLOR_RESET);
    printf(YELLOW "  cryo <command> [options]\n\n" COLOR_RESET);

    // Main Commands Section
    printf(BOLD "COMMANDS\n" COLOR_RESET);
    printf("  " BOLD "build" COLOR_RESET "     Build a project or single file\n");
    printf("  " BOLD "help" COLOR_RESET "      Display this help message\n");
    printf("  " BOLD "version" COLOR_RESET "   Display version information\n");
    printf("  " BOLD "env" COLOR_RESET "       Enter the Cryo development environment\n\n");

    printf(BOLD "DEV COMMANDS\n" COLOR_RESET);
    printf("  " BOLD "bc" COLOR_RESET "  Build the Cryo compiler (runs makefile)\n");
    printf("  " BOLD "cc" COLOR_RESET "  Clean the Cryo compiler build directory\n\n");
    printf("  " BOLD "devserver" COLOR_RESET "  Start the Cryo development server\n\n");

    printf(BOLD "For more information on a specific command, run:\n" COLOR_RESET);
    printf(YELLOW "    cryo -h|help <command>\n\n" COLOR_RESET);

    // Additional Information
    printf(BOLD "ADDITIONAL INFORMATION\n" COLOR_RESET);
    printf("  For more detailed documentation, visit:\n");
    printf(LIGHT_BLUE "  https://github.com/jakelequire/cryo\n\n" COLOR_RESET);
}

void exe_CLI_help_options(HelpOptions *options)
{
    switch (options->command)
    {
    case CLI_BUILD:
        helpcmd_build();
        break;
    case CLI_INIT:
        helpcmd_init();
        break;
    case CLI_VERSION:
        helpcmd_version();
        break;
    case CLI_BUILD_COMPILER:
        helpcmd_build_compiler();
        break;
    case CLI_CLEAN_COMPILER:
        helpcmd_clean_compiler();
        break;
    case CLI_DEVSERVER:
        helpcmd_devserver();
        break;
    default:
        exe_CLI_help();
        break;
    }
}

// =============================================================================
// Specific Command Help Functions

// ============================================================================= //
//                                cryo build                                     //
// ============================================================================= //

void helpcmd_build(void)
{
    // Build Command Details
    printf("\n\n");
    printf(BOLD UNDERLINE "BUILD COMMAND\n\n" COLOR_RESET);
    printf("   " BOLD YELLOW "cryo build" COLOR_RESET ITALIC GRAY " [options]\n\n" COLOR_RESET);
    printf("  " BOLD "Usage:" COLOR_RESET "\n");
    printf("    cryo build [options]              Build project in current directory\n");
    printf("    cryo build -f <file>              Build a single source file\n");
    printf("\n");

    printf("  " BOLD "Options:" COLOR_RESET "\n");
    printf("    -f, --file <path>          Specify input source file\n");
    printf("    -o, --output <path>        Specify output file location " UNIMPLEMENTED "\n");
    printf("    -r, --run                  Automatically run the compiled binary\n");
    printf("    --debug                    Build with debug symbols\n");
    printf("    --release                  Build with optimizations \n");
    printf("\n");

    printf(" " BOLD "Advanced Options:" COLOR_RESET "\n");
    printf("    -d, --dev                  Enable development mode with logging\n");
    printf("    -g, --gdb                  Enable debugging with GDB\n");
    printf("\n");

    // Examples Section
    printf(BOLD UNDERLINE "EXAMPLES\n" COLOR_RESET);
    printf("\n");
    printf("  Build a project:\n");
    printf(YELLOW "    $ cryo build\n" COLOR_RESET);
    printf("\n");

    printf("  Build a single file:\n");
    printf(YELLOW "    $ cryo build -f src/main.cryo\n" COLOR_RESET);
    printf("\n");

    printf("  Build with custom output:\n");
    printf(YELLOW "    $ cryo build -f src/main.c -o bin/program\n" COLOR_RESET);
    printf("\n");

    printf("  Build in development mode:\n");
    printf(YELLOW "    $ cryo build -f src/main.cryo --dev\n" COLOR_RESET);
    printf("\n");
}

// ============================================================================= //
//                            cryo build-compiler                                //
// ============================================================================= //

void helpcmd_build_compiler(void)
{
    // Build Compiler Command Details
    printf("\n\n");
    printf(BOLD UNDERLINE "BUILD COMPILER COMMAND\t" COLOR_RESET DEV_COMMAND "\n\n");
    printf("   " BOLD YELLOW "cryo build-compiler\n\n" COLOR_RESET);
    printf("  " BOLD "Usage:" COLOR_RESET "\n");
    printf("    cryo build-compiler              Build the Cryo compiler\n\n");

    printf("  " BOLD "Description:" COLOR_RESET "\n");
    printf("    This command will build the Cryo compiler using the makefile\n\n");

    printf("  " BOLD "Options:" COLOR_RESET "\n");
    printf("    None\n\n");

    // Examples Section
    printf(BOLD "EXAMPLES\n" COLOR_RESET);
    printf("  Build the Cryo compiler:\n");
    printf(YELLOW "    $ cryo build-compiler\n\n" COLOR_RESET);
    printf("\n");
}

// ============================================================================= //
//                            cryo clean-compiler                                //
// ============================================================================= //

void helpcmd_clean_compiler(void)
{
    // Clean Compiler Command Details
    printf("\n\n");
    printf(BOLD UNDERLINE "CLEAN COMPILER COMMAND\t" COLOR_RESET DEV_COMMAND "\n\n");
    printf("   " BOLD YELLOW "cryo clean-compiler\n\n" COLOR_RESET);
    printf("  " BOLD "Usage:" COLOR_RESET "\n");
    printf("    cryo clean-compiler              Clean the Cryo compiler build directory\n\n");

    printf("  " BOLD "Description:" COLOR_RESET "\n");
    printf("    This command will clean the Cryo compiler build directory\n\n");

    printf("  " BOLD "Options:" COLOR_RESET "\n");
    printf("    --all                           Clean all compiler directories\n");
    printf("    --custom <name>                 Clean a custom directory\n\n");

    // Examples Section
    printf(BOLD "EXAMPLES\n" COLOR_RESET);
    printf("  Clean all compiler directories:\n");
    printf(YELLOW "    $ cryo clean-compiler --all\n\n" COLOR_RESET);

    printf("  Clean a custom directory:\n");
    printf(YELLOW "    $ cryo clean-compiler --custom <name>\n\n" COLOR_RESET);
    printf("\n");
}

// ============================================================================= //
//                                 cryo init                                     //
// ============================================================================= //

void helpcmd_init(void)
{
    // Init Command Details
    printf("\n\n");
    printf(BOLD UNDERLINE "INIT COMMAND\n\n" COLOR_RESET);
    printf("   " BOLD YELLOW "cryo init\n\n" COLOR_RESET);
    printf("  " BOLD "Usage:" COLOR_RESET "\n");
    printf("    cryo init              Initialize a new Cryo project\n\n");

    printf("  " BOLD "Description:" COLOR_RESET "\n");
    printf("    This command will initialize a new Cryo project in the current directory\n\n");

    printf("  " BOLD "Options:" COLOR_RESET "\n");
    printf("    None\n\n");

    // Examples Section
    printf(BOLD "EXAMPLES\n" COLOR_RESET);
    printf("  Initialize a new Cryo project:\n");
    printf(YELLOW "    $ cryo init\n\n" COLOR_RESET);
    printf("\n");
}

// ============================================================================= //
//                               cryo version                                    //
// ============================================================================= //

void helpcmd_version(void)
{
    // Version Command Details
    printf("\n\n");
    printf(BOLD UNDERLINE "VERSION COMMAND\n\n" COLOR_RESET);
    printf("   " BOLD YELLOW "cryo version\n\n" COLOR_RESET);
    printf("  " BOLD "Usage:" COLOR_RESET "\n");
    printf("    cryo version              Display version information\n\n");

    printf("  " BOLD "Description:" COLOR_RESET "\n");
    printf("    This command will display the version information for the Cryo build system\n\n");

    printf("  " BOLD "Options:" COLOR_RESET "\n");
    printf("    None\n\n");

    // Examples Section
    printf(BOLD "EXAMPLES\n" COLOR_RESET);
    printf("  Display version information:\n");
    printf(YELLOW "    $ cryo version\n\n" COLOR_RESET);
    printf("\n");
}

// ============================================================================= //
//                               cryo devserver                                  //
// ============================================================================= //

void helpcmd_devserver(void)
{
    // Dev Server Command Details
    printf("\n\n");
    printf(BOLD UNDERLINE "DEV SERVER COMMAND\t" COLOR_RESET DEV_COMMAND "\n\n");
    printf("   " BOLD YELLOW "cryo devserver\n\n" COLOR_RESET);
    printf("  " BOLD "Usage:" COLOR_RESET "\n");
    printf("    cryo devserver              Start the Cryo development server\n\n");

    printf("  " BOLD "Description:" COLOR_RESET "\n");
    printf("    This command will start the Cryo development server\n\n");
    printf("    The development server watches the Cryo Compiler root directory for changes\n");
    printf("    and automatically recompiles the compiler when changes are detected\n\n");

    printf("  " BOLD "Options:" COLOR_RESET "\n");
    printf("    None\n\n");

    // Examples Section
    printf(BOLD "EXAMPLES\n" COLOR_RESET);
    printf("  Start the Cryo development server:\n");
    printf(YELLOW "    $ cryo devserver\n\n" COLOR_RESET);
    printf("\n");
}
