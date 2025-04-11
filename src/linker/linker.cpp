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
    CryoLinker *globalLinker = nullptr;

    // This function will take the build directory, seek into the `runtime` folder,
    // and compile and combine all `.ll` files under this directory. The resulting
    // module will be the Cryo Runtime Module.
    llvm::Module *Linker::mergeRuntimeToModule(void)
    {
        __STACK_FRAME__
        std::cout << "Merging Runtime Module to Main Module..." << std::endl;

        // At this step of the compilation process, this module being passed is the newly created
        // module from the Cryo Compiler. We will add the required dependencies to this module before
        // it is passed to the LLVM backend code generator.
        Cryo::Linker *cLinker = GetCXXLinker();
        if (!cLinker)
        {
            logMessage(LMI, "ERROR", "Linker", "Cryo Linker is null");
            CONDITION_FAILED;
            return nullptr;
        }

        std::string runtimeDir = cLinker->dirInfo->runtimeDir;
        if (runtimeDir.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "Runtime Directory is empty");
            CONDITION_FAILED;
            return nullptr;
        }

        // List the files in the runtime directory
        std::vector<std::string> files = listDir(runtimeDir.c_str());
        if (files.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "No files found in runtime directory");
            CONDITION_FAILED;
            return nullptr;
        }
        int fileCount = files.size();

        // Print out the files
        std::cout << "\n=============== {Files } ===============\n";
        for (int i = 0; i < fileCount; i++)
        {
            std::cout << "File: " << files[i] << std::endl;
        }
        std::cout << "========================================\n\n";

        // Create the runtime module
        llvm::Module *runtimeModule = new llvm::Module("CryoRuntime", cLinker->context);

        // Merge all the files into the runtime module
        for (int i = 0; i < fileCount; i++)
        {
            std::string filePath = runtimeDir + "/" + files[i];
            std::cout << "File Path: " << filePath << std::endl;
            if (!fs->fileExists(filePath.c_str()))
            {
                logMessage(LMI, "ERROR", "Linker", "File does not exist: %s", filePath.c_str());
                CONDITION_FAILED;
            }
            else
            {
                // Print out the file contents
                std::string fileContents = fs->readFile(filePath.c_str());
                std::cout << "File Contents: \n"
                          << fileContents << std::endl;
            }
            llvm::SMDiagnostic err;
            llvm::LLVMContext &context = cLinker->context;
            llvm::Module *mod = llvm::parseIRFile(filePath, err, context).release();
            if (!mod)
            {
                logMessage(LMI, "ERROR", "Linker", "Failed to parse IR file: %s", filePath.c_str());
                std::string errMsg;
                llvm::raw_string_ostream rso(errMsg);
                err.print("Linker", rso);
                logMessage(LMI, "ERROR", "Linker", "Error: %s", errMsg.c_str());
                CONDITION_FAILED;
                return nullptr;
            }

            // Merge the module into the runtime module
            cLinker->mergeInModule(runtimeModule, mod);
        }

        return runtimeModule;
    }

    // This function will merge the source module into the destination module
    void Linker::mergeInModule(llvm::Module *destModule, llvm::Module *srcModule)
    {
        __STACK_FRAME__
        if (!destModule)
        {
            logMessage(LMI, "ERROR", "Linker", "Destination Module is null");
            CONDITION_FAILED;
            return;
        }
        if (!srcModule)
        {
            logMessage(LMI, "ERROR", "Linker", "Source Module is null");
            CONDITION_FAILED;
            return;
        }

        logMessage(LMI, "INFO", "Linker", "Merging modules");
        logMessage(LMI, "INFO", "Linker", "Destination Module: %s", destModule->getName().str().c_str());

        llvm::Linker::Flags linkerFlags = llvm::Linker::Flags::LinkOnlyNeeded;
        bool result = llvm::Linker::linkModules(
            *destModule,
            llvm::CloneModule(*srcModule),
            linkerFlags);
        if (result)
        {
            logMessage(LMI, "ERROR", "Linker", "Failed to merge modules");
            CONDITION_FAILED;
            return;
        }

        std::cout << "@mergeModule Module merged successfully" << std::endl;
    }

    // ======================================================================
    // Supporting Functions

    // This function will list the files in a directory and return a vector of strings
    // It will ignore `.`, `..`, and hidden files.
    std::vector<std::string> Linker::listDir(const char *dirPath)
    {
        __STACK_FRAME__
        std::vector<std::string> files;
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir(dirPath)) != NULL)
        {
            while ((ent = readdir(dir)) != NULL)
            {
                std::string fileName = ent->d_name;
                if (fileName == "." || fileName == ".." || fileName[0] == '.')
                {
                    continue;
                }
                files.push_back(fileName);
            }
            closedir(dir);
        }
        else
        {
            logMessage(LMI, "ERROR", "Linker", "Failed to open directory: %s", dirPath);
        }

        return files;
    }

} // namespace Cryo
