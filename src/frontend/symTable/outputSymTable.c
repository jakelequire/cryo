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
#include "frontend/symTable.h"

SymTableOutput *createSymTableOutput(const char *fileName, const char *filePath, const char *fileExt, const char *cwd)
{
    SymTableOutput *output = (SymTableOutput *)malloc(sizeof(SymTableOutput));
    output->fileName = fileName;
    output->filePath = filePath;
    output->fileExt = fileExt;
    output->cwd = cwd;
    return output;
}

void outputSymTable(CryoSymbolTable *table, CompilerSettings *settings)
{
    logMessage(LMI, "INFO", "SymTableOut", "Outputting symbol table to file");
    // The file name of the compiled file
    const char *compiledFile = settings->inputFile;
    // The root dir where the command was run
    const char *rootDir = settings->rootDir;
    const char *fileName = "symTableOutput";
    const char *ext = ".txt";

    // The outdir is the directory where the file will be outputted
    const char *outPath = "/build/debug";
    const char *outDir = strcat((char *)rootDir, outPath);

    // Create the file buffer from the symTable
    char *buffer = symTableOutBuffer(table);

    // Create the output struct
    SymTableOutput *output = createSymTableOutput(fileName, outDir, ext, rootDir);

    // Create the output file
    createSymTableOutputFile(buffer, output);

    // Free the buffer
    free(buffer);
    free(output);

    logMessage(LMI, "INFO", "SymTableOut", "Symbol table output complete");

    return;
}

void createSymTableOutputFile(const char *buffer, SymTableOutput *symTableOutput)
{
    // Create the file path
    char *outFilePath = (char *)malloc(strlen(symTableOutput->filePath) + 1);
    sprintf(outFilePath, "%s/", symTableOutput->filePath);

    if (!dirExists(outFilePath))
    {
        logMessage(LMI, "WARN", "SymTableOut", "Output directory does not exist");
        printf("Directory does not exist: %s\n", strdup(outFilePath));
        createDir(outFilePath);
    }

    char *fullFilePath = (char *)malloc(sizeof(outFilePath) + strlen(symTableOutput->fileName) + strlen(symTableOutput->fileExt) + 1);
    sprintf(fullFilePath, "%s%s", symTableOutput->fileName, symTableOutput->fileExt);

    char *filePath = (char *)malloc(strlen(outFilePath) + strlen(fullFilePath) + 1);
    sprintf(filePath, "%s%s", outFilePath, fullFilePath);

    logMessage(LMI, "INFO", "SymTableOut", "Writing symbol table to file: %s", filePath);

    // Remove the previous output file
    removePrevSymTableOutput(filePath);

    // Open the file
    FILE *file = fopen(filePath, "w");
    if (!file)
    {
        logMessage(LMI, "ERROR", "SymTableOut", "Failed to open file for writing");
        return;
    }

    // Write the buffer to the file
    fprintf(file, "%s", buffer);

    logMessage(LMI, "INFO", "SymTableOut", "Symbol table written to file");

    // Close the file
    int closeStatus = fclose(file);
    if (closeStatus != 0)
    {
        logMessage(LMI, "ERROR", "SymTableOut", "Failed to close file");
        return;
    }

    return;
}

char *symTableOutBuffer(CryoSymbolTable *table)
{
    int SIZE_OF_BUFFER = 10240;
    char *buffer = (char *)malloc(sizeof(char) * SIZE_OF_BUFFER);
    if (!buffer)
    {
        logMessage(LMI, "ERROR", "SymTableOut", "Failed to allocate memory for buffer");
        return NULL;
    }

    int offset = 0;
    offset += snprintf(buffer + offset, SIZE_OF_BUFFER - offset, "Symbol count: %d\n", table->count);
    offset += snprintf(buffer + offset, SIZE_OF_BUFFER - offset, "Scope depth: %d\n", table->scopeDepth);
    offset += snprintf(buffer + offset, SIZE_OF_BUFFER - offset, "Table capacity: %d\n", table->capacity);
    offset += snprintf(buffer + offset, SIZE_OF_BUFFER - offset, "\n\n");
    offset += snprintf(buffer + offset, SIZE_OF_BUFFER - offset, "Namespace: %s\n", table->namespaceName ? table->namespaceName : "Unnamed");
    offset += snprintf(buffer + offset, SIZE_OF_BUFFER - offset, "\n");
    offset += snprintf(buffer + offset, SIZE_OF_BUFFER - offset, "Name                 Node Type                Data Type        Scope        L:C       Args      Module\n");
    offset += snprintf(buffer + offset, SIZE_OF_BUFFER - offset, "----------------------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < table->count; i++)
    {
        if (table->symbols[i] == NULL)
        {
            offset += snprintf(buffer + offset, SIZE_OF_BUFFER - offset, "Error: symbol at index %d is null\n", i);
            continue;
        }
        if (table->symbols[i]->node == NULL)
        {
            offset += snprintf(buffer + offset, SIZE_OF_BUFFER - offset, "Error: node in symbol at index %d is null\n", i);
            continue;
        }

        char locationStr[16];

        // Create the "L:C" string
        snprintf(locationStr, sizeof(locationStr), "%d:%d",
                 table->symbols[i]->line,
                 table->symbols[i]->column);

        offset += snprintf(buffer + offset, SIZE_OF_BUFFER - offset, "%-20s %-24s %-18s %-10d %-10s %-7d %-15s\n",
                           table->symbols[i]->name ? table->symbols[i]->name : "Unnamed",
                           CryoNodeTypeToString(table->symbols[i]->nodeType),
                           DataTypeToString(table->symbols[i]->type),
                           table->symbols[i]->scopeLevel,
                           locationStr,
                           table->symbols[i]->argCount,
                           table->symbols[i]->module ? table->symbols[i]->module : "Unnamed");
    }
    offset += snprintf(buffer + offset, SIZE_OF_BUFFER - offset, "----------------------------------------------------------------------------------------------------------\n");

    return buffer;
}

void removePrevSymTableOutput(const char *filePath)
{
    printf("\nFile Path passed: %s\n", filePath);
    // Remove the previous output file
    bool doesFileExist = fileExists(filePath);
    if (doesFileExist)
    {
        removeFile(filePath);
    }
    else
    {
        logMessage(LMI, "WARN", "SymTableOut", "No previous output file to remove");
        return;
    }
}
