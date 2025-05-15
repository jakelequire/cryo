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
#include "diagnostics/diagnostics.h"

namespace Cryo
{
    void Linker::generateIRFromCodegen(llvm::Module *mod, const char *outputPath)
    {
        __STACK_FRAME__
        logMessage(LMI, "INFO", "Linker", "@generateIRFromCodegen Generating IR from Codegen");
        logMessage(LMI, "INFO", "Linker", "@generateIRFromCodegen Output Path: %s", outputPath);
        if (!mod)
        {
            logMessage(LMI, "ERROR", "Linker", "Module is null");
            return;
        }
        if (!outputPath)
        {
            logMessage(LMI, "ERROR", "Linker", "Output path is null");
            return;
        }

        // Verify the module
        // if (llvm::verifyModule(*mod, &llvm::errs()))
        // {
        //     logMessage(LMI, "ERROR", "Linker", "Module verification failed");
        //     return;
        // }

        // The outputPath should be the full path + the file name
        // e.g /path/to/file/file.ll
        // Make the empty file before writing to it
        std::string trimmedFilePath = fs->removeFileFromPath(outputPath);
        fs->createDir(trimmedFilePath.c_str());
        fs->createNewEmptyFileWpath(outputPath);
        logMessage(LMI, "INFO", "Linker", "Starting IR Generation...");
        std::error_code EC;
        llvm::raw_fd_ostream fileStream(outputPath, EC);
        if (EC)
        {
            logMessage(LMI, "ERROR", "Linker", "Failed to open file!");
            logMessage(LMI, "ERROR", "Linker", "Error: %s", EC.message().c_str());
            return;
        }

        llvm::AssemblyAnnotationWriter *annotWriter = nullptr;
        mod->print(fileStream, annotWriter);
        fileStream.close();

        logMessage(LMI, "INFO", "Linker", "IR File generated successfully: %s", outputPath);
        return;
    }

    void Linker::compileModule(CompilationUnit *unit, llvm::Module *mod)
    {
        __STACK_FRAME__
        logMessage(LMI, "INFO", "Linker", "@compileModule Compiling Module");
        if (!unit)
        {
            logMessage(LMI, "ERROR", "Linker", "CompilationUnit is null");
            return;
        }
        if (!mod)
        {
            logMessage(LMI, "ERROR", "Linker", "Module is null");
            return;
        }

        std::string outputPath = std::string(unit->dir.out_filePath) + std::string(unit->dir.out_fileExt);
        logMessage(LMI, "INFO", "Linker", "Output Path: %s", outputPath.c_str());
        generateIRFromCodegen(mod, outputPath.c_str());
        logMessage(LMI, "INFO", "Linker", "Module compiled successfully");

        return;
    }

} // namespace Cryo
