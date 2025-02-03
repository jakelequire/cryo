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
#include "compiler/compiler.h"
#include "diagnostics/diagnostics.h"

ASTNode *compileModuleFileToProgramNode(const char *filePath, const char *outputPath, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Compiler", "Compiling module file: %s", filePath);

    char *source = readFile(filePath);
    if (!source)
    {
        fprintf(stderr, "Error: Failed to read file: %s\n", filePath);
        CONDITION_FAILED;
    }

    // Initialize the Arena
    Arena *arena = createArena(ARENA_SIZE, ALIGNMENT);

    // Initialize the lexer
    Lexer lexer;
    initLexer(&lexer, source, filePath, state);

    state->isModuleFile = true;

    ASTNode *programNode = parseProgram(&lexer, arena, state, globalTable);
    if (programNode == NULL)
    {
        fprintf(stderr, "Error: Failed to parse program node\n");
        return NULL;
    }

    logMessage(LMI, "INFO", "Compiler", "MODULE COMPILER: Program node parsed successfully");
    logMessage(LMI, "INFO", "Compiler", "MODULE COMPILER: File path: %s", filePath);
    logMessage(LMI, "INFO", "Compiler", "MODULE COMPILER: Output path: %s", outputPath);

    char *rawFileName = getFileNameFromPathNoExt(filePath);
    const char *IRFileName = appendExtensionToFileName(rawFileName, ".ll");
    const char *completeFilePath = appendPathToFileName(outputPath, IRFileName, true);

    logMessage(LMI, "INFO", "Compiler", "MODULE COMPILER: complete file path: %s", completeFilePath);
    logMessage(LMI, "INFO", "Compiler", "MODULE COMPILER: Generating IR code...");

    CryoLinker *cLinker = GetGSTLinker(globalTable);

    // Generate the IR code
    processNodeToIRObject(programNode, state, completeFilePath, cLinker);

    return programNode;
}

SymbolTable *compileToReapSymbols(const char *filePath, const char *outputPath, CompilerState *state, Arena *arena, CryoGlobalSymbolTable *globalTable)
{
    logMessage(LMI, "INFO", "Compiler", "Compiling to reap symbols...");

    char *source = readFile(filePath);
    if (!source)
    {
        fprintf(stderr, "Error: Failed to read file: %s\n", filePath);
        CONDITION_FAILED;
    }

    logMessage(LMI, "INFO", "Compiler", "File Read. File Path: %s", filePath);

    // Initialize the lexer
    Lexer lexer;
    initLexer(&lexer, source, filePath, state);
    logMessage(LMI, "INFO", "Compiler", "Lexer Initialized");

    const char *globalBuildDir = GetBuildDir(globalTable);
    // Initialize the global symbol table (for reaping)
    CryoGlobalSymbolTable *globalSymbolTable = CryoGlobalSymbolTable_Create_Reaping(true, globalBuildDir);
    if (!globalSymbolTable)
    {
        logMessage(LMI, "ERROR", "Compiler", "Failed to create global symbol table");
        CONDITION_FAILED;
    }
    logMessage(LMI, "INFO", "Compiler", "Global Symbol Table Initialized");

    setGlobalSymbolTable(state, globalSymbolTable);

    ASTNode *programNode = parseProgram(&lexer, arena, state,globalSymbolTable);
    if (programNode == NULL)
    {
        fprintf(stderr, "Error: Failed to parse program node\n");
        CONDITION_FAILED;
    }

    logMessage(LMI, "INFO", "Compiler", "Program node parsed successfully");

    SymbolTable *reapedSymbols = GetReapedTable(globalSymbolTable);
    if (!reapedSymbols)
    {
        logMessage(LMI, "ERROR", "Compiler", "Failed to reap symbols");
        CONDITION_FAILED;
    }
    TypesTable *reapedTypes = GetReapedTypeTable(globalSymbolTable);
    if (!reapedTypes)
    {
        logMessage(LMI, "ERROR", "Compiler", "Failed to reap types");
        CONDITION_FAILED;
    }

    SymbolTable *copiedTable = (SymbolTable *)malloc(sizeof(SymbolTable));
    if (!copiedTable)
    {
        logMessage(LMI, "ERROR", "Compiler", "Failed to allocate memory for copied table");
        CONDITION_FAILED;
    }
    memcpy(copiedTable, reapedSymbols, sizeof(SymbolTable));

    TypesTable *copiedTypes = (TypesTable *)malloc(sizeof(TypesTable));
    if (!copiedTypes)
    {
        logMessage(LMI, "ERROR", "Compiler", "Failed to allocate memory for copied types");
        CONDITION_FAILED;
    }
    memcpy(copiedTypes, reapedTypes, sizeof(TypesTable));

    ImportReapedTypesTable(globalTable, copiedTypes);

    CryoLinker *cLinker = GetGSTLinker(globalTable);

    int resultObj = processNodeToIRObject(programNode, state, outputPath, cLinker);
    if (resultObj != 0)
    {
        logMessage(LMI, "ERROR", "Compiler", "Failed to process node to IR object");
        CONDITION_FAILED;
    }

    logMessage(LMI, "INFO", "Compiler", "Symbols reaped successfully");
    PrintSymbolTable(globalSymbolTable, reapedSymbols);

    // CleanupAndDestroySymbolTable(globalSymbolTable);

    setGlobalSymbolTable(state, globalTable);

    return copiedTable;
}

int processNodeToIRObject(ASTNode *node, CompilerState *state, const char *outputPath, CryoLinker *cLinker)
{
    logMessage(LMI, "INFO", "Compiler", "Processing node to IR object...");
    logMessage(LMI, "INFO", "Compiler", "Output Path: %s", outputPath);

    // Generate code
    int result = generateImportCode(node, state, cLinker, outputPath);
    if (result != 0)
    {
        CONDITION_FAILED;
        return result;
    }

    return 0;
}
