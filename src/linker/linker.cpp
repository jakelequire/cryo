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
    CryoLinker *globalLinker = nullptr;

    /// @brief This function will initialize the creation of the Cryo Runtime Module.
    /// This module is compiled along with a C runtime module to create the final
    /// runtime module that will be used in the Cryo Compiler before the main Cryo module.
    ///
    /// @return The Cryo Runtime Module
    ///
    /// @note this function is called from the C++ CodeGen API before code generation.
    llvm::Module *Linker::initMainModule(void)
    {
        std::cout << "Initializing Main Module before CodeGen..." << std::endl;

        // At this step of the compilation process, this module being passed is the newly created
        // module from the Cryo Compiler. We will add the required dependencies to this module before
        // it is passed to the LLVM backend code generator.
        Cryo::Linker *cLinker = GetCXXLinker();
        if (!cLinker)
        {
            logMessage(LMI, "ERROR", "Linker", "Cryo Linker is null");
            return nullptr;
        }

        std::cout << "Cryo Linker is not undefined" << std::endl;
        // Add the dependencies to the root module

        // init the C runtime module
        cLinker->createCRuntimeFile();

        llvm::Module *runtimeMod = cLinker->getCryoRuntimeModule();
        std::cout << "Runtime Module:" << std::endl;
        runtimeMod->print(llvm::errs(), nullptr);

        return runtimeMod;
    }

    /// @brief This function will seek for the `cryo_runtime.ll` file that should have been created
    /// at an earlier stage in the compiler. This function is called to parse this file and returns
    /// the `llvm::Module *`.
    /// @param
    /// @return The Cryo Runtime Module
    llvm::Module *Linker::getCryoRuntimeModule(void)
    {
        Cryo::Linker *cLinker = GetCXXLinker();
        if (!cLinker)
        {
            logMessage(LMI, "ERROR", "Linker", "Cryo Linker is null");
            return nullptr;
        }

        std::string cryoRuntimefile = cLinker->getDirInfo()->runtimeDir + "/cryo_runtime.ll";
        if (cryoRuntimefile.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "Cryo Runtime file is empty");
            return nullptr;
        }

        // Parse the cryo runtime file
        llvm::SMDiagnostic err;
        llvm::Module *cryoRuntimeModule = llvm::parseIRFile(cryoRuntimefile, err, getLinkerContext()).release();
        if (!cryoRuntimeModule)
        {
            logMessage(LMI, "ERROR", "Linker", "Failed to parse Cryo Runtime file");
            std::cout << "Runtime File: " << cryoRuntimefile << std::endl;
            CONDITION_FAILED;
            return nullptr;
        }
        if (err.getMessage().str().size() > 0)
        {
            logMessage(LMI, "ERROR", "Linker", "Error parsing Cryo Runtime file");
            std::cout << "Error: " << err.getMessage().str() << std::endl;
            CONDITION_FAILED;
            return nullptr;
        }

        // Safely clone the module from LLVM's API
        std::cout << "Cryo Runtime Module Compiled Successfully" << std::endl;

        return cryoRuntimeModule;
    }

    /// @brief This function is called after the cryo runtime module has been parsed and
    /// and at the end of the code generation process. This will create the `cryo_runtime.ll`
    /// file that will be used in the final compilation process and used in a later stage.
    ///
    /// @param mod The module to be used to create the IR file (runtime.cryo)
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

        std::cout << "\n\nModule:\n--------\n"
                  << std::endl;
        mod->print(llvm::errs(), nullptr);
        std::cout << "\n--------\n\n"
                  << std::endl;

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

        // Now that the `runtime.ll` and `cRuntime.ll` files are generated, we will
        // merge them into one file and set that as the `preprocessedModule`.
        std::string outputFilePath = mergeTwoIRFiles(modIR, cRuntimeIR, "cryo_runtime");
        if (outputFilePath.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "Failed to merge IR files");
            return;
        }

        std::cout << "Output File Path is not undefined" << std::endl;
        std::cout << "Output File Path: " << outputFilePath << std::endl;

        // Parse the merged IR file
        llvm::SMDiagnostic err;
        llvm::Module *mergedModule = llvm::parseIRFile(outputFilePath, err, context).get();
        if (!mergedModule)
        {
            logMessage(LMI, "ERROR", "Linker", "Failed to parse merged IR file");
            CONDITION_FAILED;
            return;
        }

        std::cout << "Merged Module is not undefined" << std::endl;

        // Now that we have the merged module, we can set it as the preprocessed module
        GetCXXLinker()->setPreprocessedModule(mergedModule);
        std::cout << "Merged Module Set as Preprocessed Module" << std::endl;

        std::cout << "\n\nMerged Module @addPreprocessingModule:\n--------\n"
                  << std::endl;
        mergedModule->print(llvm::errs(), nullptr);
        std::cout << "\n--------\n\n"
                  << std::endl;

        return;
    }

    void Linker::createCRuntimeFile(void)
    {
        DirectoryInfo *dirInfo = getDirInfo();
        if (!dirInfo)
        {
            logMessage(LMI, "ERROR", "Linker", "Directory Info is null");
            return;
        }

        std::string runtimeDir =dirInfo->runtimeDir + "/";
        if (runtimeDir.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "Runtime directory is empty");
            return;
        }

        std::string cRuntimePath = getCRuntimePath();
        if (cRuntimePath.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "C Runtime path is empty");
            return;
        }

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

        std::cout << "@createIRFromModule | ourDir passed for file creation: " << outDir << std::endl;
        std::string moduleName = module->getName().str();
        std::cout << "Module Name passed to @createIRFromModule: " << moduleName << std::endl;

        std::string outPath = outDir + "/" + moduleName + ".ll";

        fs->createNewEmptyFile(moduleName.c_str(), ".ll", outDir.c_str());

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

    /// @brief This function will return the path to the Cryo Runtime file path.
    /// @return The path to the Cryo Runtime file path.
    std::string Linker::getCRuntimePath()
    {
        std::string cryoRoot = getCryoRootPath();
        if (cryoRoot.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "Cryo Root is empty");
            return "";
        }

        std::cout << "@getCRuntimePath | Cryo Root: " << cryoRoot << std::endl;

        std::string fullPath = cryoRoot + "/Std/Runtime";

        std::cout << "@getCRuntimePath | Full Path: " << fullPath << std::endl;

        return fullPath;
    }

    std::string Linker::covertCRuntimeToLLVMIR(std::string cRuntimePath, std::string outDir)
    {
        if (cRuntimePath.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "C Runtime path is empty");
            return "";
        }

        std::cout << "@convertCRuntimeToLLVMIR | C Runtime Path: " << cRuntimePath << std::endl;

        // Check and see if the `cRuntime.c` file exists
        std::string cRuntimeFile =  cRuntimePath + "/" + C_RUNTIME_FILENAME + ".c";
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

    // This function will be used to merge two IR files together.
    // This function will return the full path to where the file was created.
    std::string Linker::mergeTwoIRFiles(std::string file1, std::string file2, std::string fileName)
    {
        logMessage(LMI, "INFO", "Linker", "Merging two IR files...");

        if (file1.empty() || file2.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "One or both files are empty");
            CONDITION_FAILED;
            return "";
        }

        std::string outDir = file1.substr(0, file1.find_last_of("/"));
        std::string outPath = outDir + "/" + fileName + ".ll";

        std::string cmd = "llvm-link-18 " + file1 + " " + file2 + " -S -o " + outPath;
        int result = system(cmd.c_str());
        if (result != 0)
        {
            logMessage(LMI, "ERROR", "Linker", "Failed to merge IR files");
            CONDITION_FAILED;
            return "";
        }

        logMessage(LMI, "INFO", "Linker", "IR files merged");

        std::string fullPathToFile = file1.substr(0, file1.find_last_of("/")) + "/" + fileName + ".ll";
        logMessage(LMI, "INFO", "Linker", "Merged IR file: %s", fullPathToFile.c_str());

        llvm::LLVMContext context;
        llvm::SMDiagnostic err;
        std::unique_ptr<llvm::Module> mergedModule = llvm::parseIRFile(fullPathToFile, err, context);
        if (!mergedModule)
        {
            logMessage(LMI, "ERROR", "Linker", "Failed to parse merged IR file");
            CONDITION_FAILED;
            return "";
        }

        std::cout << "Module Merged Successfully" << std::endl;

        return fullPathToFile;
    }

    std::string Linker::getCryoRuntimeFilePath(void)
    {
        std::string runtimeDir = dirInfo->runtimeDir;
        if (runtimeDir.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "Runtime directory is empty");
            return "";
        }

        std::string cryoRuntimeFile = runtimeDir + "/cryo_runtime.ll";
        if (!fileExists(cryoRuntimeFile.c_str()))
        {
            logMessage(LMI, "ERROR", "Linker", "Cryo Runtime file does not exist: %s", cryoRuntimeFile.c_str());
            return "";
        }

        return cryoRuntimeFile;
    }

    void Linker::mergeTwoModules(llvm::Module *destMod, std::unique_ptr<llvm::Module> srcMod)
    {
        if (!destMod)
        {
            logMessage(LMI, "ERROR", "Linker", "Destination module is null");
            CONDITION_FAILED;
        }

        if (!srcMod)
        {
            logMessage(LMI, "ERROR", "Linker", "Source module is null");
            CONDITION_FAILED;
        }

        bool result = llvm::Linker::linkModules(*destMod, std::move(srcMod));
        if (result)
        {
            logMessage(LMI, "ERROR", "Linker", "Failed to merge modules");
            CONDITION_FAILED;
        }

        std::cout << "@mergeTwoModule | Module Merged Successfully" << std::endl;
    }

    // ================================================================ //
    // End of Compilation API

    void Linker::completeCodeGeneration(void)
    {
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

        std::cout << "Main File Compiled Successfully" << std::endl;

        // Run the binary
        runCompletedBinary();
    }

    void Linker::runCompletedBinary()
    {
        std::string buildDir = GetCXXLinker()->getDirInfo()->buildDir;
        if (buildDir.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "Build directory is empty");
            return;
        }
        // In the future, the binary name may change. Seek the only bianry
        // file within the build directory and run it.

        std::cout << "Running Main Binary..." << std::endl;

        std::string exe_output = buildDir + "/";
        std::string exe_name = "main";
        std::string sys_cmd = exe_output + exe_name;
        int result = system(sys_cmd.c_str());
        if (result != 0 || WEXITSTATUS(result) != 0)
        {
            logMessage(LMI, "ERROR", "Linker", "Failed to run main binary");
            std::string error = "Error: " + std::to_string(result);
            std::string errReason = std::strerror(errno);
            logMessage(LMI, "ERROR", "Linker", error.c_str());
            logMessage(LMI, "ERROR", "Linker", errReason.c_str());
            CONDITION_FAILED;
        }
    }    

} // namespace Cryo
