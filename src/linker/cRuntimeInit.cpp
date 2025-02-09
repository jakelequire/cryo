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

#define C_RUNTIME_FILENAME "cRuntime"

namespace Cryo
{
    /// @brief This function will return the path to the Cryo Runtime file path.
    /// @return The path to the Cryo Runtime file path.
    std::string Linker::getCRuntimePath()
    {
        __STACK_FRAME__
        std::string unsafe_cryoRoot = getCryoRootPath();
        if (unsafe_cryoRoot.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "Cryo Root is empty");
            return "";
        }

        std::string cryoRoot = fs->cleanFilePath((char *)unsafe_cryoRoot.c_str());
        logMessage(LMI, "INFO", "Linker", "Cryo Root Path: %s", cryoRoot.c_str());
        std::string fullPath = cryoRoot + "/Std/Runtime";
        logMessage(LMI, "INFO", "Linker", "C Runtime Path: %s", fullPath.c_str());

        return fullPath;
    }

    void Linker::createCRuntimeFile(void)
    {
        __STACK_FRAME__
        DirectoryInfo *dirInfo = getDirInfo();
        if (!dirInfo)
        {
            logMessage(LMI, "ERROR", "Linker", "Directory Info is null");
            return;
        }

        std::string runtimeDir = dirInfo->outDir + "/obj/";
        if (runtimeDir.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "Runtime directory is empty");
            return;
        }

        std::string unsafe_cRuntimePath = getCRuntimePath();
        if (unsafe_cRuntimePath.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "C Runtime path is empty");
            return;
        }

        std::string cRuntimePath = fs->cleanFilePath((char *)unsafe_cRuntimePath.c_str());

        // Then, convert the C runtime to IR
        int cRuntimeResult = covertCRuntimeToLLVMIR(cRuntimePath, runtimeDir);
        if (cRuntimeResult != 0)
        {
            logMessage(LMI, "ERROR", "Linker", "Failed to convert C Runtime to IR");
            CONDITION_FAILED;
        }

        logMessage(LMI, "INFO", "Linker", "C Runtime IR is not empty");

        return;
    }

    int Linker::covertCRuntimeToLLVMIR(std::string cRuntimePath, std::string outDir)
    {
        __STACK_FRAME__
        if (cRuntimePath.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "C Runtime path is empty");
            return 1;
        }

        logMessage(LMI, "INFO", "Linker", "Converting C Runtime to IR...");
        logMessage(LMI, "INFO", "Linker", "Output Directory: %s", outDir.c_str());
        logMessage(LMI, "INFO", "Linker", "C Runtime Path: %s", cRuntimePath.c_str());
        // Check and see if the `cRuntime.c` file exists
        std::string safe_cRuntimePath = fs->cleanFilePath((char *)cRuntimePath.c_str());
        std::string cRuntimeFile = safe_cRuntimePath + "/" + C_RUNTIME_FILENAME + ".c";
        if (!fileExists(cRuntimeFile.c_str()))
        {
            logMessage(LMI, "ERROR", "Linker", "C Runtime file does not exist: %s", cRuntimeFile.c_str());
            return 1;
        }

        // Now that we have the file, we can convert it to IR
        std::string unsafe_outPath = outDir + "/" + C_RUNTIME_FILENAME + ".ll";
        std::string outPath = fs->cleanFilePath((char *)unsafe_outPath.c_str());
        std::string cmd = "clang -S -emit-llvm " + cRuntimeFile + " -o " + outPath;

        logMessage(LMI, "INFO", "Linker", "Full System Command: \n");
        printf(BOLD GREEN "%s\n" COLOR_RESET, cmd.c_str());

        int result = system(cmd.c_str());
        if (result != 0)
        {
            logMessage(LMI, "ERROR", "Linker", "Failed to convert C Runtime to IR");
            return 1;
        }
        else
        {
            logMessage(LMI, "INFO", "Linker", "C Runtime converted to IR: %s", outPath.c_str());
        }

        // After creating the IR (`.ll`) file, we will convert it to an object file (`.o`)
        std::string objPath = outDir + "/" + C_RUNTIME_FILENAME + ".o";
        std::string objCmd = "clang -c " + outPath + " -o " + objPath;
        int objResult = system(objCmd.c_str());
        if (objResult != 0)
        {
            logMessage(LMI, "ERROR", "Linker", "Failed to convert C Runtime IR to Object file");
            return 1;
        }
        else
        {
            logMessage(LMI, "INFO", "Linker", "C Runtime IR converted to Object: %s", objPath.c_str());
        }

        // Clean up the `.ll` file after converting it to an object file
        fs->removeFile(outPath.c_str());

        logMessage(LMI, "INFO", "Linker", "C Runtime converted to IR: %s", outPath.c_str());

        return 0;
    }

    void Linker::initCRuntime(void)
    {
        __STACK_FRAME__
        logMessage(LMI, "INFO", "Linker", "Initializing C Runtime...");
        createCRuntimeFile();
        logMessage(LMI, "INFO", "Linker", "C Runtime Initialized");
        return;
    }
} // namespace Cryo