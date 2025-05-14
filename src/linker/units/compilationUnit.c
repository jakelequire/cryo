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
#include "tools/logger/logger_config.h"
#include "linker/compilationUnit.h"
#include "diagnostics/diagnostics.h"

CompilationUnit *CompilationUnit_Create(CompilationUnitType type, CompilationUnitDir dir)
{
    CompilationUnit *unit = (CompilationUnit *)malloc(sizeof(CompilationUnit));
    if (!unit)
    {
        logMessage(LMI, "ERROR", "CompilationUnit", "Failed to allocate memory for CompilationUnit");
        return NULL;
    }

    unit->type = type;
    unit->dir = dir;
    unit->ast = NULL;
    unit->isASTSet = false;
    unit->isVerified = false;

    unit->setAST = CompilationUnit_SetAST;
    unit->verify = CompilationUnit_Verify;

    return unit;
}
void CompilationUnit_SetAST(CompilationUnit *self, ASTNode *ast)
{
    if (!self)
    {
        logMessage(LMI, "ERROR", "CompilationUnit", "CompilationUnit is null");
        return;
    }

    self->ast = ast;
    self->isASTSet = true;
    return;
}

// TODO: Add more verification checks
int CompilationUnit_Verify(CompilationUnit *self)
{
    if (!self)
    {
        logMessage(LMI, "ERROR", "CompilationUnit", "CompilationUnit is null");
        return 1;
    }

    if (!self->isASTSet)
    {
        logMessage(LMI, "ERROR", "CompilationUnit", "AST is not set");
        return 1;
    }

    self->isVerified = true;

    logMessage(LMI, "INFO", "CompilationUnit", "CompilationUnit is verified");

    return 0;
}

void CompilationUnitDir_print(CompilationUnitDir dir)
{
    DEBUG_PRINT_FILTER({
        printf("\n");
        printf("\n");
        printf(BOLD GREEN);
        printf("+ ------------------------------- CompilationUnitDir ------------------------------- +\n");
        printf("| Compilation Unit Type: %s\n", compilationUnitTypeToString(dir.type));
        printf(COLOR_RESET);
        printf("|\n");
        printf("| Source File Name: \t\t%s\n", dir.src_fileName);
        printf("| Source File Full Name: \t%s\n", dir.src_fullFileName);
        printf("| Source File Dir: \t\t%s\n", dir.src_fileDir);
        printf("| Source File Path: \t\t%s\n", dir.src_filePath);
        printf("| Source File Ext: \t\t%s\n", dir.src_fileExt);
        printf(BOLD GREEN);
        printf("| ---------------------------------------------------------------------------------- |\n");
        printf(COLOR_RESET);
        printf("| Output File Name: \t\t%s\n", dir.out_fileName);
        printf("| Output File Full Name: \t%s\n", dir.out_fullFileName);
        printf("| Output File Dir: \t\t%s\n", dir.out_fileDir);
        printf("| Output File Path: \t\t%s\n", dir.out_filePath);
        printf("| Output File Ext: \t\t%s\n", dir.out_fileExt);
        printf(BOLD GREEN);
        printf("+ ---------------------------------------------------------------------------------- +\n");
        printf(COLOR_RESET);
        printf("\n");
        printf("\n");
    });
}

char *compilationUnitTypeToString(CompilationUnitType type)
{
    switch (type)
    {
    case CRYO_MODULE:
        return "CRYO_MODULE";
    case CRYO_MAIN:
        return "CRYO_MAIN";
    case CRYO_RUNTIME:
        return "CRYO_RUNTIME";
    case CRYO_DEPENDENCY:
        return "CRYO_DEPENDENCY";
    case CRYO_STDLIB:
        return "CRYO_STDLIB";
    default:
        return "UNKNOWN";
    }
}

// ================================================================ //

/// @brief Creates a CompilationUnitDir struct with the given input file and build directory
/// @param inputFile The input file path (e.g. `path/to/file.cryo`)
/// @param buildDir The build directory
/// @param type The type of the compilation unit
/// @return CompilationUnitDir
///
/// `fileName` is the name of the file without the extension
/// `fileDir` `path/to` is the directory of the file
/// `filePath` `path/to/file.*` is the full path to the file
/// `fileExt` `.*` is the file extension
CompilationUnitDir createCompilationUnitDir(const char *inputFile, const char *buildDir, CompilationUnitType type)
{
    CompilationUnitDir dir;
    dir.type = type;
    dir.print = CompilationUnitDir_print;

    // Get the file name
    const char *fileName = fs->getFileName(inputFile);
    if (!fileName)
    {
        logMessage(LMI, "ERROR", "CompilationUnit", "Failed to get file name");
        return dir;
    }

    // Get the file directory
    const char *fileDir = fs->removeFileFromPath(inputFile);
    if (!fileDir)
    {
        logMessage(LMI, "ERROR", "CompilationUnit", "Failed to get file directory");
        return dir;
    }

    // Get the file extension
    const char *fileExt = fs->getFileExt(inputFile);
    if (!fileExt)
    {
        logMessage(LMI, "ERROR", "CompilationUnit", "Failed to get file extension");
        return dir;
    }

    String *fullFileName = _Str(fileName);
    fullFileName->append(fullFileName, ".");
    fullFileName->append(fullFileName, (char *)fileExt);

    // Set the file name
    dir.src_fileName = fileName;
    dir.src_fullFileName = fullFileName->c_str(fullFileName);
    dir.src_fileDir = fileDir;
    dir.src_filePath = inputFile;
    dir.src_fileExt = fileExt;

    // Depending on the type, the output paths will be different

    switch (type)
    {
    case CRYO_MODULE:
    {
        // Set the output file name
        dir.out_fileName = fileName;
        // Set the output file directory
        dir.out_fileDir = buildDir;
        // Set the output file path
        dir.out_filePath = fs->appendPathToFileName(buildDir, fileName, false);
        // Set the output file extension
        dir.out_fileExt = ".ll";

        String *fullFileName = _Str(fileName);
        fullFileName->append(fullFileName, (char *)dir.out_fileExt);
        dir.out_fullFileName = fullFileName->c_str(fullFileName);
        break;
    }
    case CRYO_MAIN: // The output for this should be `/build/out/.ll/main.ll`
    {               // Set the output file name
        dir.out_fileName = "main";
        // Set the output file directory
        dir.out_fileDir = fs->appendPathToFileName(buildDir, "out/.ll/", true);
        // Set the output file path
        dir.out_filePath = fs->appendPathToFileName(dir.out_fileDir, "main", false);
        // Set the output file extension
        dir.out_fileExt = ".ll";

        String *fullFileName = _Str("main");
        fullFileName->append(fullFileName, (char *)dir.out_fileExt);
        dir.out_fullFileName = fullFileName->c_str(fullFileName);
        break;
    }
    case CRYO_RUNTIME: // The ouput for this should be `/build/out/runtime/{fileName}.ll`
    {                  // Set the output file name
        dir.out_fileName = fileName;
        // Set the output file directory
        dir.out_fileDir = fs->appendPathToFileName(buildDir, "out/runtime/", true);
        // Set the output file path
        dir.out_filePath = fs->appendPathToFileName(dir.out_fileDir, fileName, false);
        // Set the output file extension
        dir.out_fileExt = ".ll";

        String *fullFileName = _Str(fileName);
        fullFileName->append(fullFileName, (char *)dir.out_fileExt);
        dir.out_fullFileName = fullFileName->c_str(fullFileName);
        break;
    }
    case CRYO_DEPENDENCY: // The output for this should be `/build/out/deps/{fileName}.ll`
    {                     // Set the output file name
        dir.out_fileName = fileName;
        // Set the output file directory
        dir.out_fileDir = fs->appendPathToFileName(buildDir, "out/deps/", true);
        // Set the output file path
        dir.out_filePath = fs->appendPathToFileName(dir.out_fileDir, fileName, false);
        // Set the output file extension
        dir.out_fileExt = ".ll";

        String *fullFileName = _Str(fileName);
        fullFileName->append(fullFileName, (char *)dir.out_fileExt);
        dir.out_fullFileName = fullFileName->c_str(fullFileName);

        break;
    }
    case CRYO_STDLIB:
    {
        break;
    }
    default:
        logMessage(LMI, "ERROR", "CompilationUnit", "Unknown CompilationUnitType");
        printf("Received CompilationUnitType: %s\n", compilationUnitTypeToString(type));
        break;
    }

    return dir;
}

CompilationUnitDir createCompilerCompilationUnitDir(
    const char *inputFile, const char *buildDir, const char *compilerDir, CompilationUnitType type)
{
    CompilationUnitDir dir;
    dir.type = type;
    dir.print = CompilationUnitDir_print;

    // Get the file name
    const char *fileName = fs->getFileName(inputFile);
    if (!fileName)
    {
        logMessage(LMI, "ERROR", "CompilationUnit", "Failed to get file name");
        return dir;
    }

    // Get the file directory
    const char *fileDir = fs->removeFileFromPath(inputFile);
    if (!fileDir)
    {
        logMessage(LMI, "ERROR", "CompilationUnit", "Failed to get file directory");
        return dir;
    }

    // Get the file extension
    const char *fileExt = fs->getFileExt(inputFile);
    if (!fileExt)
    {
        logMessage(LMI, "ERROR", "CompilationUnit", "Failed to get file extension");
        return dir;
    }

    String *fullFileName = _Str(fileName);
    fullFileName->append(fullFileName, ".");
    fullFileName->append(fullFileName, (char *)fileExt);
    // Set the file name
    dir.src_fileName = fileName;
    dir.src_fullFileName = fullFileName->c_str(fullFileName);
    dir.src_fileDir = fileDir;
    dir.src_filePath = inputFile;
    dir.src_fileExt = fileExt;

    // Depending on the type, the output paths will be different
    switch (type)
    {
    case CRYO_MAIN:
    {
        // Unsupported for compiler compilation units
        logMessage(LMI, "ERROR", "CompilationUnit", "CRYO_MAIN is not supported for compiler compilation units");
        break;
    }
    case CRYO_STDLIB:
    {
        // Output of the file should be `{compilerDir}/Std/bin/.ll/{fileName}.ll`
        // Set the output file name
        dir.out_fileName = fileName;
        // Set the output file directory
        dir.out_fileDir = fs->appendPathToFileName(compilerDir, "/cryo/Std/bin/.ll/", true);
        // Set the output file path
        dir.out_filePath = fs->appendPathToFileName(dir.out_fileDir, fileName, false);
        // Set the output file extension
        dir.out_fileExt = ".ll";
        String *fullFileName = _Str(fileName);
        fullFileName->append(fullFileName, (char *)dir.out_fileExt);
        dir.out_fullFileName = fullFileName->c_str(fullFileName);
        break;
    }
    default:
    {
        logMessage(LMI, "ERROR", "CompilationUnit", "Unknown CompilationUnitType");
        printf("Received CompilationUnitType: %s\n", compilationUnitTypeToString(type));
        break;
    }
    }
    return dir;
}

CompilationUnit *createNewCompilationUnit(ASTNode *root, CompilationUnitDir dir)
{
    CompilationUnit *unit = CompilationUnit_Create(dir.type, dir);
    if (!unit)
    {
        logMessage(LMI, "ERROR", "CompilationUnit", "Failed to create CompilationUnit");
        return NULL;
    }

    unit->setAST(unit, root);

    return unit;
}
