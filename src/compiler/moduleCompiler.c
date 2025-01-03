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

ASTNode *compileModuleFileToProgramNode(const char *filePath, const char *outputPath, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    logMessage(LMI, "INFO", "Compiler", "Compiling module file: %s", filePath);

    char *source = readFile(filePath);
    if (!source)
    {
        fprintf(stderr, "Error: Failed to read file: %s\n", filePath);
        CONDITION_FAILED;
    }

    // Initialize the Arena
    Arena *arena = createArena(ARENA_SIZE, ALIGNMENT);

    // Initialize the symbol table
    CryoSymbolTable *table = createSymbolTable(arena);

    // Initialize the type table
    TypeTable *typeTable = initTypeTable();

    // Initialize the lexer
    Lexer lexer;
    initLexer(&lexer, source, filePath, state);

    ASTNode *programNode = parseProgram(&lexer, table, arena, state, typeTable, globalTable);
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

    // Generate the IR code
    processNodeToIRObject(programNode, state, completeFilePath);

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

    // Initialize the symbol table
    CryoSymbolTable *table = createSymbolTable(arena);
    logMessage(LMI, "INFO", "Compiler", "Symbol Table Initialized");

    // Initialize the type table
    TypeTable *typeTable = initTypeTable();
    logMessage(LMI, "INFO", "Compiler", "Type Table Initialized");

    // Initialize the lexer
    Lexer lexer;
    initLexer(&lexer, source, filePath, state);
    logMessage(LMI, "INFO", "Compiler", "Lexer Initialized");

    // Initialize the global symbol table (for reaping)
    CryoGlobalSymbolTable *globalSymbolTable = CryoGlobalSymbolTable_Create_Reaping(true);
    if (!globalSymbolTable)
    {
        logMessage(LMI, "ERROR", "Compiler", "Failed to create global symbol table");
        CONDITION_FAILED;
    }
    logMessage(LMI, "INFO", "Compiler", "Global Symbol Table Initialized");

    ASTNode *programNode = parseProgram(&lexer, table, arena, state, typeTable, globalSymbolTable);
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

    SymbolTable *copiedTable = (SymbolTable *)malloc(sizeof(SymbolTable));
    memcpy(copiedTable, reapedSymbols, sizeof(SymbolTable));

    logMessage(LMI, "INFO", "Compiler", "Symbols reaped successfully");
    PrintSymbolTable(globalSymbolTable, reapedSymbols);

    CleanupAndDestroySymbolTable(globalSymbolTable);

    return copiedTable;
}

void processNodeToIRObject(ASTNode *node, CompilerState *state, const char *outputPath)
{
    logMessage(LMI, "INFO", "Compiler", "Processing node to IR object...");

    // Generate code
    int result = preprocessRuntimeIR(node, state, outputPath);
    if (result != 0)
    {
        CONDITION_FAILED;
        return;
    }

    return;
}
