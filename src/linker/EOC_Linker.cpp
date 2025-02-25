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
    void Linker::linkAll(void)
    {
        // Create the main object file
        createMainObject();
        
        // Run the completed binary
        runCompletedBinary();
    }

    // This function is triggered when the `main.ll` file is complete. The runtime definitions are
    // imported into the `main.ll` file, and now we need to link the objects that are created under
    // the `/build/out/obj` directory.
    void Linker::createMainObject(void)
    {
        __STACK_FRAME__
        std::string buildDir = GetCXXLinker()->getDirInfo()->buildDir;
        if (buildDir.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "Build directory is empty");
            return;
        }

        std::string objDir = buildDir + "/out/obj";
        if (objDir.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "Object directory is empty");
            return;
        }

        std::string mainFile = buildDir + "/out/main.ll";

        // llc -filetype=obj {buildDir}/out/main.ll -o {buildDir}/out/main.o
        std::string sys_cmd = "llc-18 -filetype=obj -relocation-model=pic " + mainFile + " -o " + objDir + "/main.o";
        int result = system(sys_cmd.c_str());
        if (result != 0 || WEXITSTATUS(result) != 0)
        {
            logMessage(LMI, "ERROR", "Linker", "Failed to create object file");
            std::string error = "Error: " + std::to_string(result);
            std::string errReason = std::strerror(errno);
            std::cerr << error << std::endl;
            std::cerr << errReason << std::endl;
            CONDITION_FAILED;
        }

        logMessage(LMI, "INFO", "Linker", "Object file created successfully");

        // Link all object files
        combineAllObjectsToMainExe();
    }

    void Linker::combineAllObjectsToMainExe(void)
    {
        __STACK_FRAME__
        std::string buildDir = GetCXXLinker()->getDirInfo()->buildDir;
        if (buildDir.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "Build directory is empty");
            return;
        }

        std::string objDir = buildDir + "/out/obj";
        if (objDir.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "Object directory is empty");
            return;
        }

        // Gather all .o files from the object directory
        std::vector<std::string> objFiles;
        for (const auto &entry : std::filesystem::directory_iterator(objDir))
        {
            if (entry.path().extension() == ".o")
            {
                objFiles.push_back(entry.path().string());
            }
        }

        if (objFiles.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "No object files found in directory");
            return;
        }

        // Construct the linking command
        std::string outPath = buildDir + "/main";
        std::string cmd = "clang-18 -pie -o " + outPath;

        // Add all object files to the command
        for (const auto &obj : objFiles)
        {
            cmd += " " + obj;
        }

        std::cout << "\n\nSystem Command: " << cmd << "\n\n"
                  << std::endl;

        // Execute the linking command
        int result = system(cmd.c_str());
        if (result != 0)
        {
            logMessage(LMI, "ERROR", "Linker", "Failed to link objects into executable");
            return;
        }

        logMessage(LMI, "INFO", "Linker", "Successfully linked executable at %s", outPath.c_str());
    }

    void Linker::completeCodeGeneration(void)
    {
        __STACK_FRAME__
        std::cout << "End of Compilation Signal Received..." << std::endl;

        // Look for the `main.ll` file that should be under {buildDir}/out/main.ll
        std::string buildDir = GetCXXLinker()->getDirInfo()->buildDir;
        if (buildDir.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "Build directory is empty");
            return;
        }
        std::string exe_output = buildDir + "/";
        std::string exe_name = "main";
        std::string mainFile = buildDir + "/out/main.ll";
        std::string sys_cmd = "clang-18 -o " + exe_output + exe_name + " " + mainFile;
        int result = system(sys_cmd.c_str());
        if (result != 0 || WEXITSTATUS(result) != 0)
        {
            logMessage(LMI, "ERROR", "Linker", "Failed to compile main.ll");
            std::string error = "Error: " + std::to_string(result);
            std::string errReason = std::strerror(errno);
            logMessage(LMI, "ERROR", "Linker", error.c_str());
            logMessage(LMI, "ERROR", "Linker", errReason.c_str());
            CONDITION_FAILED;
        }

        logMessage(LMI, "INFO", "Linker", "Main Binary Compiled Successfully");
        // Run the binary
        // runCompletedBinary();
    }

    void Linker::runCompletedBinary()
    {
        __STACK_FRAME__
        std::string buildDir = GetCXXLinker()->getDirInfo()->buildDir;
        if (buildDir.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "Build directory is empty");
            return;
        }
        // In the future, the binary name may change. Seek the only bianry
        // file within the build directory and run it.

        logMessage(LMI, "INFO", "Linker", "Running main binary...");
        std::string exe_output = buildDir + "/";
        std::string exe_name = "main";
        std::string sys_cmd = exe_output + exe_name;
        int result = system(sys_cmd.c_str());
        // if (result != 0 || WEXITSTATUS(result) != 0)
        // {
        //     logMessage(LMI, "ERROR", "Linker", "Failed to run main binary");
        //     std::string error = "Error: " + std::to_string(result);
        //     std::string errReason = std::strerror(errno);
        //     logMessage(LMI, "ERROR", "Linker", error.c_str());
        //     logMessage(LMI, "ERROR", "Linker", errReason.c_str());
        //     CONDITION_FAILED;
        // }
        return;
    }
} // namespace Cryo
