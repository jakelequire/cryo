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
#include "cpp/codegen.h"

namespace Cryo
{
    void Compilation::compileIRFile(void)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        CryoContext &cryoContext = compiler.getContext();
        CompilerSettings *settings = compiler.getCompilerState()->settings;

        debugger.logMessage("INFO", __LINE__, "Compilation", "Compiling IR File");

        if (llvm::verifyModule(*cryoContext.module, &llvm::errs()))
        {
            LLVM_MODULE_FAILED_MESSAGE_START;
            cryoContext.module->print(llvm::errs(), nullptr);
            LLVM_MODULE_FAILED_MESSAGE_END;
            debugger.logMessage("ERROR", __LINE__, "Compilation", "LLVM module verification failed");
            exit(1);
        }

        std::string outputDir = settings->rootDir ? settings->rootDir : ".";
        std::string outputFile = settings->inputFile ? std::string(settings->inputFile) : "output";

        // Trim the directory path from the file name
        outputFile = outputFile.substr(outputFile.find_last_of("/") + 1);
        outputFile = outputFile.substr(0, outputFile.find_last_of(".")) + ".ll";

        std::string outputPath = outputDir + "/build/out/" + outputFile;

        if (settings->customOutputPath)
        {
            outputPath = std::string(settings->customOutputPath) + "/" + outputFile;
        }

        // Ensure the output directory exists
        std::filesystem::create_directories(std::filesystem::path(outputPath).parent_path());

        std::error_code EC;
        llvm::raw_fd_ostream dest(outputPath, EC, llvm::sys::fs::OF_None);
        if (EC)
        {
            debugger.logMessage("ERROR", __LINE__, "Compilation", "Error opening file for writing: " + EC.message());
            return;
        }

        LLVM_MODULE_COMPLETE_START;
        LoadStoreWhitespaceAnnotator LSWA;

        // Use the custom annotator when printing
        cryoContext.module->print(dest, &LSWA);
        cryoContext.module->print(llvm::outs(), &LSWA);

        LLVM_MODULE_COMPLETE_END;

        if (settings->activeBuild)
        {
            debugger.logMessage("INFO", __LINE__, "Compilation", "Active Build");
            // Create the IR File in the current working directory
            std::string irFileName = outputFile;
            std::string cwd = std::filesystem::current_path().string();
            std::string irFilePath = cwd + "/" + irFileName;

            std::error_code EC;
            llvm::raw_fd_ostream irFile(irFilePath, EC, llvm::sys::fs::OF_None);
            if (EC)
            {
                debugger.logMessage("ERROR", __LINE__, "Compilation", "Could not open file: " + EC.message());
            }
            else
            {
                cryoContext.module->print(irFile, nullptr);
                irFile.close();
            }
        }

        dest.close();

        debugger.logMessage("INFO", __LINE__, "Compilation", "Compilation Complete");
        return;
    }

    /// @private
    void Compilation::compile(std::string inputFile, std::string outputPath)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Compilation", "outputPath: " + outputPath);
        // Check if the file exists
        // std::ifstream file(outputPath);
        // if (!file)
        // {
        //     debugger.logMessage("ERROR", __LINE__, "Compilation", "Source file not found");
        //     CONDITION_FAILED;
        // }

        std::error_code EC;
        llvm::raw_fd_ostream dest(outputPath, EC, llvm::sys::fs::OF_None);
        if (EC)
        {
            debugger.logMessage("ERROR", __LINE__, "Compilation", "Error opening file for writing");
        }
        else
        {
            std::cout << "\n>===------- LLVM IR Code -------===<\n"
                      << std::endl;

            compiler.getContext().getModules()->back()->print(dest, nullptr);

            // Create our custom annotator
            LoadStoreWhitespaceAnnotator LSWA;

            // Use the custom annotator when printing
            compiler.getModule().print(dest, &LSWA);
            compiler.getModule().print(llvm::outs(), &LSWA);

            std::cout << "\n>===------- End IR Code ------===<\n"
                      << std::endl;

            dest.flush();
            dest.close();

            debugger.logMessage("INFO", __LINE__, "Compilation", "Code CodeGen Complete");

            return;
        }
    }

    /// @private
    void Compilation::isValidDir(std::string dirPath)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Compilation", "Checking if directory exists");

        std::filesystem::path dir(dirPath);
        if (!std::filesystem::exists(dir))
        {
            debugger.logMessage("ERROR", __LINE__, "Compilation", "Directory does not exist, creating one...");
            makeOutputDir(dirPath);
            return;
        }

        return;
    }

    void Compilation::isValidFile(std::string filePath)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Compilation", "Checking if file exists");

        std::ifstream file(filePath);
        if (!file)
        {
            debugger.logMessage("ERROR", __LINE__, "Compilation", "File does not exist");
            CONDITION_FAILED;
        }

        return;
    }

    /// @private
    void Compilation::makeOutputDir(std::string dirPath)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Compilation", "Creating output directory");

        std::filesystem::path dir(dirPath);
        if (!std::filesystem::exists(dir))
        {
            std::filesystem::create_directories(dir);
        }
    }
}
