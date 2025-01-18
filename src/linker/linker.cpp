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

#define C_RUNTIME_FILENAME "cRuntime"

namespace Cryo
{
    // This function will be used to to handle the runtime IR. The module passed to this function
    // is the cryo runtime file and will be combined with the c runtime file.
    void Linker::addPreprocessingModule(llvm::Module *mod)
    {
        logMessage(LMI, "INFO", "Linker", "Adding Preprocessing Module...");

        if (!mod)
        {
            logMessage(LMI, "ERROR", "Linker", "Module is null");
            return;
        }

        std::cout << "Module is not undefined" << std::endl;

        DirectoryInfo *dirInfo = getDirInfo();
        if (!dirInfo)
        {
            logMessage(LMI, "ERROR", "Linker", "Directory Info is null");
            return;
        }

        std::cout << "Directory Info is not undefined" << std::endl;

        std::string runtimeDir = dirInfo->runtimeDir;
        if (runtimeDir.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "Runtime directory is empty");
            return;
        }

        std::cout << "Runtime Directory is not undefined" << std::endl;
        std::cout << "Runtime Directory: " << runtimeDir << std::endl;

        std::string cRuntimePath = getCRuntimePath();
        if (cRuntimePath.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "C Runtime path is empty");
            return;
        }

        std::cout << "C Runtime Path is not undefined" << std::endl;
        std::cout << "C Runtime Path: " << cRuntimePath << std::endl;

        // Create the IR for the mod first and output it to the runtime directory
        std::string modIR = createIRFromModule(mod, runtimeDir);
        if (modIR.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "Failed to create IR from module");
            return;
        }

        std::cout << "Module IR is not undefined" << std::endl;
        std::cout << "Module IR: " << modIR << std::endl;

        // Then, convert the C runtime to IR
        std::string cRuntimeIR = covertCRuntimeToLLVMIR(cRuntimePath, runtimeDir);
        if (cRuntimeIR.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "Failed to convert C Runtime to IR");
            return;
        }

        std::cout << "C Runtime IR is not undefined" << std::endl;

        return;
    }

    void Linker::appendDependenciesToRoot(llvm::Module *root)
    {
        if (!root)
        {
            logMessage(LMI, "ERROR", "Linker", "Root module is null");
            return;
        }

        std::string rootName = root->getName().str();
        if (rootName.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "Root module name is empty");
            return;
        }

        return;
    }

    void Linker::newInitDependencies(llvm::Module *srcModule)
    {
    }

    /// @brief Create a `.ll` file from the given module and output directory.
    /// @param module The module to create the IR from.
    /// @param outDir The output directory to write the IR file to.
    /// @return The path to the created IR file.
    std::string Linker::createIRFromModule(llvm::Module *module, std::string outDir)
    {
        if (!module)
        {
            logMessage(LMI, "ERROR", "Linker", "Module is null");
            CONDITION_FAILED;
        }

        if (outDir.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "Output directory is empty");
            CONDITION_FAILED;
        }

        logMessage(LMI, "INFO", "Linker", "Creating IR from module...");

        std::string outPath = outDir + "/" + module->getName().str() + ".ll";
        std::error_code EC;
        llvm::raw_fd_ostream out(outPath, EC, llvm::sys::fs::OF_Text);
        if (EC)
        {
            logMessage(LMI, "ERROR", "Linker", "Failed to open file: %s", outPath.c_str());
            CONDITION_FAILED;
        }

        module->print(out, nullptr);

        logMessage(LMI, "INFO", "Linker", "IR file created: %s", outPath.c_str());

        return outPath;
    }

    // ================================================================ //

    std::string Linker::getCRuntimePath()
    {
        std::string cryoRoot = getCryoCompilerPath();
        if (cryoRoot.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "Cryo Root is empty");
            return "";
        }

        return cryoRoot + "/Std/Runtime";
    }

    std::string Linker::covertCRuntimeToLLVMIR(std::string cRuntimePath, std::string outDir)
    {
        if (cRuntimePath.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "C Runtime path is empty");
            return "";
        }

        // Check and see if the `cRuntime.c` file exists
        std::string cRuntimeFile = cRuntimePath + "/" + C_RUNTIME_FILENAME + ".c";
        if (!fileExists(cRuntimeFile.c_str()))
        {
            logMessage(LMI, "ERROR", "Linker", "C Runtime file does not exist: %s", cRuntimeFile.c_str());
            return "";
        }

        // Create the output directory if it doesn't exist
        if (!dirExists(outDir.c_str()))
        {
            if (mkdir(outDir.c_str(), 0777) != 0)
            {
                logMessage(LMI, "ERROR", "Linker", "Failed to create output directory: %s", outDir.c_str());
                return "";
            }
        }

        // Now that we have the file, we can convert it to IR
        std::string outPath = outDir + "/" + C_RUNTIME_FILENAME + ".ll";
        std::string cmd = "clang -S -emit-llvm " + cRuntimeFile + " -o " + outPath;
        int result = system(cmd.c_str());
        if (result != 0)
        {
            logMessage(LMI, "ERROR", "Linker", "Failed to convert C Runtime to IR");
            return "";
        }

        logMessage(LMI, "INFO", "Linker", "C Runtime converted to IR: %s", outPath.c_str());

        return outPath;
    }

    // This function is going to be used to merge all of the runtime files into one.
    // It will look into the `DirectoryInfo` struct and find the runtime directory.
    // It will then look for all of the `.ll` files in that directory and merge them
    // Into one file.
    // It will return `false` if there are no files in the runtime directory.
    bool Linker::mergeAllRuntimeFiles()
    {
        logMessage(LMI, "INFO", "Linker", "Merging all runtime files...");

        DirectoryInfo *dirInfo = getDirInfo();
        if (!dirInfo)
        {
            logMessage(LMI, "ERROR", "Linker", "Directory Info is null");
            return false;
        }

        std::string runtimeDir = dirInfo->runtimeDir;
        if (runtimeDir.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "Runtime directory is empty");
            return false;
        }

        // Get all of the files in the runtime directory
        std::vector<std::string> files = listDir(runtimeDir.c_str());
        if (files.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "No files found in runtime directory");
            return false;
        }

        // Now we need to merge all of the files into one
        for (auto file : files)
        {
            std::string filePath = runtimeDir + "/" + file;
            logMessage(LMI, "INFO", "Linker", "Merging file: %s", filePath.c_str());
        }

        return true;
    }

    std::vector<std::string> Linker::listDir(const char *path)
    {
        std::vector<std::string> files;
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir(path)) != NULL)
        {
            while ((ent = readdir(dir)) != NULL)
            {
                files.push_back(ent->d_name);
            }
            closedir(dir);
        }
        else
        {
            logMessage(LMI, "ERROR", "Linker", "Failed to open directory: %s", path);
        }

        return files;
    }

} // namespace Cryo
