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

namespace Cryo 
{

    void Linker::generateIRFromCodegen(llvm::Module *mod, const char *outputPath)
    {
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
        // The outputPath should be the full path + the file name
        // e.g /path/to/file/file.ll
        // Make the empty file before writing to it
        std::string trimmedFilePath = fs->removeFileFromPath(outputPath);
        std::cout << "Trimmed File Path: " << trimmedFilePath << std::endl;
        fs->createDir(trimmedFilePath.c_str());
        fs->createNewEmptyFileWpath(outputPath);
        logMessage(LMI, "INFO", "Linker", "Starting IR Generation...");
        std::error_code EC;
        llvm::raw_fd_ostream fileStream(outputPath, EC, llvm::sys::fs::OF_Text);
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

} // namespace Cryo
