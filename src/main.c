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

#include "main.h"
#include "build.h"

#ifdef __cplusplus
extern "C"
{
#endif

    int generateCodeWrapper(ASTNode *node, CompilerState *state);

#ifdef __cplusplus
}
#endif

void printUsage(const char *programName)
{
    fprintf(stderr, "Usage: %s -f <file> [options]\n", programName);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -f, --file <path>    Specify input file path (required)\n");
    fprintf(stderr, "  -o, --output <path>  Specify output file path\n");
    fprintf(stderr, "  -s, --source <text>  Specify source text directly\n");
    fprintf(stderr, "  -a, --active-build  Flag that indicates the build is active\n");
    fprintf(stderr, "  -v, --verbose        Enable verbose output\n");
    fprintf(stderr, "  -h, --help           Display this help message\n");
}

int main(int argc, char *argv[])
{
    const char *inputFile = NULL;
    const char *outputFile = NULL;
    bool verbose = false;
    bool activeBuild = false;
    bool isSource = false;
    int opt;

    static struct option long_options[] = {
        {"file", required_argument, 0, 'f'},
        {"source", required_argument, 0, 's'},
        {"active-build", no_argument, 0, 'a'},
        {"output", required_argument, 0, 'o'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}};

    // First pass: check for help flag
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            printUsage(argv[0]);
            return 0;
        }
    }

    // Reset optind for the main parsing pass
    optind = 1;

    while ((opt = getopt_long(argc, argv, "f:o:s:avh", long_options, NULL)) != -1)
    {
        switch (opt)
        {
        case 'f':
            printf("Input file: %s\n", optarg);
            inputFile = optarg;
            break;
        case 's':
            isSource = true;
            break;
        case 'a':
            printf("Active build flag set\n");
            activeBuild = true;
            break;
        case 'o':
            outputFile = optarg;
            break;
        case 'v':
            verbose = true;
            break;
        case 'h':
            printUsage(argv[0]);
            return 0;
        default:
            fprintf(stderr, "Unknown option: %c\n", opt);
            printUsage(argv[0]);
            return 1;
        }
    }

    if (!inputFile)
    {
        fprintf(stderr, "Error: Input file (-f) is required.\n");
        printUsage(argv[0]);
        return 1;
    }

    printf("\n\n\n");
    printf("<!> ### =============================================================== ### <!>\n");
    printf("<!> ### - - - - - - - - - - - - Start of Compilation - - - - - - - - -  ### <!>\n");
    printf("<!> ### =============================================================== ### <!>\n");
    printf("\n\n\n");

    // Initialize the Arena
    Arena *arena = createArena(ARENA_SIZE, ALIGNMENT);

    const char *fileName;
    char *source;
    if (strcmp(argv[1], "-s") == 0)
    {
        if (argc < 3)
        {
            fprintf(stderr, "Error: No source text provided after -s flag\n");
            return 1;
        }
        source = argv[2];
    }
    else
    {
        fileName = inputFile;
        source = readFile(inputFile);
        if (source == NULL)
        {
            fprintf(stderr, "Failed to read source file.\n");
            return 1;
        }
    }



    // Initialize the call stack
    initCallStack(&callStack, 10);

    // Initialize the symbol table
    CryoSymbolTable *table = createSymbolTable(arena);

    // Initialize the lexer
    Lexer lexer;
    CompilerState state = initCompilerState(arena, &lexer, table, fileName);
    state.isActiveBuild = activeBuild;
    initLexer(&lexer, source, fileName, &state);
    logMessage("INFO", __LINE__, "Main", "Lexer Initialized... ");

    // Parse the source code
    ASTNode *programNode = parseProgram(&lexer, table, arena, &state);

    if (programNode != NULL)
    {
        dumpCompilerState(state);
        int size = programNode->data.program->statementCount;
        ASTNode *nodeCpy = (ASTNode *)malloc(sizeof(ASTNode) * size);
        memcpy(nodeCpy, programNode, sizeof(ASTNode));

        printSymbolTable(table);
        printAST(nodeCpy, 0, arena);
        DEBUG_ARENA_PRINT(arena);

        printf("[Main] Generating IR code...\n");
        if (generateCodeWrapper(nodeCpy, &state) == 0)
        {
            printf("Compilation completed successfully.\n");
            if (activeBuild)
            {
                // Extract the file name from the full path
                const char *trimmedFileName = strrchr(fileName, '/');
                if (trimmedFileName == NULL)
                {
                    // If there's no '/' in the path, use the whole fileName
                    trimmedFileName = fileName;
                }
                else
                {
                    // Skip the '/' character
                    trimmedFileName++;
                }

                // Find the position of the last '.'
                const char *dotPosition = strrchr(trimmedFileName, '.');
                size_t nameLength = dotPosition ? (size_t)(dotPosition - trimmedFileName) : strlen(trimmedFileName);

                // Allocate memory for the new file name ("./" + nameLength + ".ll" + null terminator)
                char *outputFileName = (char *)malloc(nameLength + 6);
                if (outputFileName == NULL)
                {
                    fprintf(stderr, "Memory allocation failed\n");
                    return 1;
                }

                // Add "./" prefix
                strcpy(outputFileName, "./");

                // Copy the file name without the extension
                strncat(outputFileName, trimmedFileName, nameLength);
                strcat(outputFileName, ".ll");

                // Move the build file to the build directory
                moveBuildFile(outputFileName, getcwd(NULL, 0));

                // Don't forget to free the allocated memory
                free(outputFileName);
            }
            else
            {
                printf("\nActive build flag not set, skipping move.\n");
            }
        }
        printf("\n>===------------- CPP End Code Generation -------------===<\n");
        printf("[Main] IR code generated, freeing AST.\n");

        // Free the Arena
        freeArena(arena);
    }
    else
    {
        fprintf(stderr, "[Main] Failed to parse program.\n");
        freeArena(arena);
    }

    printf("\n\n\n");
    printf("<!> ### =============================================================== ### <!>\n");
    printf("<!> ### - - - - - - - - - - - - End of Compilation - - - - - - - - - -  ### <!>\n");
    printf("<!> ### =============================================================== ### <!>\n");
    printf("\n\n\n");

    free(source);
    return 0;
}
