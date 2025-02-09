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
#include "linker/linker.hpp"
#include "symbolTable/cInterfaceTable.h"
#include "runtime/bootstrap.h"
#include "tools/logger/logger_config.h"
#include "diagnostics/diagnostics.h"

char *runtimePaths[] = {
    "/home/phock/Programming/apps/cryo/cryo/runtime.cryo",
    "/home/phock/Programming/apps/cryo/build/out/deps/runtime.ll",
    "/workspaces/cryo/cryo/runtime.cryo",
    "/workspaces/cryo/build/out/deps/runtime.ll"};

// This is being hard coded for now, but will be replaced with a more dynamic solution later.
#define RUNTIME_SRC_FILE runtimePaths[0]
#define RUINTIME_OBJ_FILE runtimePaths[1]

// CRYO_ROOT/Std/Runtime/runtime.cryo (CRYO_ROOT is an environment variable)
char *getRuntimeSrcFile(void)
{
    __STACK_FRAME__
    char *runtimeBuffer = (char *)malloc(sizeof(char) * 1024);
    char *envRoot = getenv("CRYO_ROOT");
    if (!envRoot)
    {
        printf(LIGHT_RED BOLD "Error: CRYO_ROOT environment variable not set!!\n" COLOR_RESET);
        CONDITION_FAILED;
    }

    sprintf(runtimeBuffer, "%s/Std/Runtime/runtime.cryo", envRoot);

    DEBUG_PRINT_FILTER({
        printf(LIGHT_GREEN BOLD "Runtime Found Environment: %s\n" COLOR_RESET, runtimeBuffer);
        return runtimeBuffer;
    });

    logMessage(LMI, "INFO", "Bootstrap", "Runtime Found Environment: %s", runtimeBuffer);

    return runtimeBuffer;
}

// {CWD}/build/out/deps/runtime.ll (CRYO_ROOT is an environment variable)
char *getRuntimeObjFile(CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    char *runtimePathBuffer = (char *)malloc(sizeof(char) * 1024);
    const char *buildDir = GetBuildDir(globalTable);
    sprintf(runtimePathBuffer, "%s/out/deps/runtime.ll", buildDir);

    DEBUG_PRINT_FILTER({
        printf(LIGHT_GREEN BOLD "Runtime Object Found Environment: %s\n" COLOR_RESET, runtimePathBuffer);
        return runtimePathBuffer;
    });

    logMessage(LMI, "INFO", "Bootstrap", "Runtime Object Found Environment: %s", runtimePathBuffer);

    return runtimePathBuffer;
}

// This function will take the Symbol Table and Type Table from the compiler and bootstrap the runtime definitions
// into the primary compiler state. This will produce an AST Node of the runtime definitions that can be used to
// compile the runtime into the program.
void boostrapRuntimeDefinitions(CryoGlobalSymbolTable *globalTable, CryoLinker *cLinker)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Bootstrap", "Bootstrapping runtime definitions...");

    setDependencyTableStatus(globalTable, true);

    char *runtimePath = getRuntimeSrcFile();
    Bootstrapper *bootstrap = initBootstrapper(runtimePath);

    logMessage(LMI, "INFO", "Bootstrap", "Bootstrapper initialized");

    // Update the bootstrap status
    updateBootstrapStatus(bootstrap, BOOTSTRAP_IN_PROGRESS);

    // Compile the runtime file
    ASTNode *runtimeNode = compileForRuntimeNode(bootstrap, runtimePath, globalTable);
    if (!runtimeNode)
    {
        fprintf(stderr, "Error: Failed to compile runtime node\n");
        updateBootstrapStatus(bootstrap, BOOTSTRAP_FAILED);
        return;
    }
    logMessage(LMI, "INFO", "Bootstrap", "Runtime node compiled successfully");

    // ===========================================

    INIT_LINKER_C_RUNTIME(cLinker);

    CompilationUnitDir runtimeDir = createCompilationUnitDir(runtimePath, GetBuildDir(globalTable), CRYO_RUNTIME);
    runtimeDir.print(runtimeDir);

    CompilationUnit *runtimeUnit = createNewCompilationUnit(runtimeNode, runtimeDir);
    if (!runtimeUnit)
    {
        fprintf(stderr, "Error: Failed to create runtime unit\n");
        updateBootstrapStatus(bootstrap, BOOTSTRAP_FAILED);
        return;
    }
    int runtimeVerification = runtimeUnit->verify(runtimeUnit);
    if (runtimeVerification != 0)
    {
        fprintf(stderr, "Error: Failed to verify runtime unit\n");
        updateBootstrapStatus(bootstrap, BOOTSTRAP_FAILED);
        return;
    }
    int runtimeCompilation = generateIRFromAST(runtimeUnit, bootstrap->state, cLinker, globalTable);
    if (runtimeCompilation != 0)
    {
        fprintf(stderr, "Error: Failed to compile runtime unit\n");
        updateBootstrapStatus(bootstrap, BOOTSTRAP_FAILED);
        return;
    }

    // ===========================================

    String *runtimeMemoryPath = Str(fs->removeFileFromPath(runtimePath));
    runtimeMemoryPath->append(runtimeMemoryPath, "/memory.cryo");
    logMessage(LMI, "INFO", "Bootstrap", "Runtime Directory Path: %s", runtimeMemoryPath->c_str(runtimeMemoryPath));

    // Compile the runtime memory file
    ASTNode *runtimeMemoryNode = compileForRuntimeNode(bootstrap, runtimeMemoryPath->c_str(runtimeMemoryPath), globalTable);
    if (!runtimeMemoryNode)
    {
        fprintf(stderr, "Error: Failed to compile runtime memory node\n");
        updateBootstrapStatus(bootstrap, BOOTSTRAP_FAILED);
        return;
    }
    logMessage(LMI, "INFO", "Bootstrap", "Runtime memory node compiled successfully");

    CompilationUnitDir runtimeMemoryDir = createCompilationUnitDir(runtimeMemoryPath->c_str(runtimeMemoryPath), GetBuildDir(globalTable), CRYO_RUNTIME);
    runtimeMemoryDir.print(runtimeMemoryDir);

    // Update the bootstrap status
    updateBootstrapStatus(bootstrap, BOOTSTRAP_SUCCESS);

    logMessage(LMI, "INFO", "Bootstrap", "Runtime definitions bootstrapped successfully");

    // Signal the completion of the dependency table
    TableFinished(globalTable);

    // Create the runtime object file
    const char *outputFile = getRuntimeObjFile(globalTable);
    bootstrap->state->settings->inputFile = getRuntimeSrcFile();

    // Create the runtime memory file
    const char *buildDir = GetBuildDir(globalTable);
    String *runtimeMemoryObjPath = Str(buildDir);
    runtimeMemoryObjPath->append(runtimeMemoryObjPath, "/out/runtime/memory.ll");
    const char *unsafe_memoryOutputFile = runtimeMemoryObjPath->c_str(runtimeMemoryObjPath);
    const char *memoryOutputFile = fs->cleanFilePath((char *)unsafe_memoryOutputFile);

    logMessage(LMI, "INFO", "Bootstrap", "Runtime Object File: %s", outputFile);
    logMessage(LMI, "INFO", "Bootstrap", "Runtime Memory Object File: %s", memoryOutputFile);

    // Compile the runtime memory object file
    int memResult = generateImportCode(runtimeMemoryNode, bootstrap->state, cLinker, memoryOutputFile, globalTable);
    if (memResult != 0)
    {
        fprintf(stderr, "Error: Failed to compile runtime memory object file\n");
        updateBootstrapStatus(bootstrap, BOOTSTRAP_FAILED);
        DEBUG_BREAKPOINT;
        return;
    }

    logMessage(LMI, "INFO", "Bootstrap", "Runtime memory object file compiled successfully");

    // Compile the runtime object file
    int runtimeResult = preprocessRuntimeIR(runtimeNode, bootstrap->state, outputFile, cLinker, globalTable);
    if (runtimeResult != 0)
    {
        fprintf(stderr, "Error: Failed to compile runtime object file\n");
        updateBootstrapStatus(bootstrap, BOOTSTRAP_FAILED);
        DEBUG_BREAKPOINT;
        return;
    }

    logMessage(LMI, "INFO", "Bootstrap", "Runtime object file compiled successfully");

    // Free the bootstrap state
    free(bootstrap);

    logMessage(LMI, "INFO", "Bootstrap", "Bootstrapper freed");

    return;
}

ASTNode *compileForRuntimeNode(Bootstrapper *bootstrap, const char *filePath, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Bootstrap", "@compileForRuntimeNode Reading file: %s", filePath);
    // This needs to create a whole separate compiler state & arena for each program node
    // This is because the program node is the root of the AST and needs to be compiled separately
    char *source = readFile(filePath);
    if (!source)
    {
        fprintf(stderr, "Error: Failed to read file: %s\n", filePath);
        CONDITION_FAILED;
    }

    // Initialize the lexer
    Lexer lexer;
    CompilerState *state = initCompilerState(bootstrap->arena, &lexer, filePath);
    bootstrap->state = state;

    logMessage(LMI, "INFO", "Bootstrap", "Compiler state initialized");

    initLexer(&lexer, source, filePath, state);

    logMessage(LMI, "INFO", "Bootstrap", "Lexer initialized");

    // Parse the source code
    ASTNode *programNode = parseProgram(&lexer, bootstrap->arena, state, globalTable);

    if (programNode == NULL)
    {
        fprintf(stderr, "Error: Failed to parse program node\n");
        return NULL;
    }

    logMessage(LMI, "INFO", "Bootstrap", "Program node parsed successfully");

    return programNode;
}

void updateBootstrapStatus(Bootstrapper *bootstrapper, enum BootstrapStatus status)
{
    __STACK_FRAME__
    bootstrapper->status = status;
}

Bootstrapper *initBootstrapper(const char *filePath)
{
    __STACK_FRAME__
    Bootstrapper *bootstrapper = (Bootstrapper *)malloc(sizeof(Bootstrapper));

    // Init the bootstrapper state
    bootstrapper->status = BOOTSTRAP_IDLE;

    // Initialize the Arena
    bootstrapper->arena = createArena(ARENA_SIZE, ALIGNMENT);
    // Set the program node to null, we will parse it later.
    bootstrapper->programNode = NULL;

    return bootstrapper;
}
