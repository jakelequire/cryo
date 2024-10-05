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
    void Compilation::compileIRFile(std::string irFilePath, std::string irFileName)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Compilation", "Compiling IR File");
        if (compiler.getCompilerState()->settings->customOutputPath == nullptr)
        {
            std::cout << "[Compilation] No custom output path provided" << std::endl;

            std::string outputDir = std::string(compiler.getCompilerState()->settings->rootDir);
            std::string outputFile = std::string(compiler.getCompilerState()->settings->inputFile);
            // Trim the directory path from the file name
            outputFile = outputFile.substr(outputFile.find_last_of("/") + 1);
            outputFile = outputFile.substr(0, outputFile.find_last_of("."));
            outputFile += ".ll";
            std::string outputPath = outputDir + "/" + outputFile;

            std::cout << "[Compilation] Output Path: " << outputPath << std::endl;
            // Check if the directory exists
            isValidDir(outputDir);

            // Get the input file
            std::string inputFile = std::string(compiler.getCompilerState()->settings->inputFile);

            // Compile the file
            compile(inputFile, outputPath);

            return;
        }
        std::string customOutputPath = std::string(compiler.getCompilerState()->settings->customOutputPath);
        if (!customOutputPath.empty())
        {
            std::cout << "[Compilation] Output Path: " << customOutputPath << std::endl;
            // Check if the directory exists
            isValidDir(customOutputPath);

            // Get the input file
            std::string inputFile = std::string(compiler.getCompilerState()->settings->inputFile);

            // Compile the file
            compile(inputFile, customOutputPath);

            return;
        }
        else
        {
            debugger.logMessage("ERROR", __LINE__, "Compilation", "No output path provided");
            CONDITION_FAILED;
        }

        return;
    }

    /// @private
    void Compilation::compile(std::string inputFile, std::string outputPath)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Compilation", "Compiling Source File");

        debugger.logMessage("INFO", __LINE__, "Compilation", "outputPath: " + outputPath);
        // Check if the file exists
        std::ifstream file(outputPath);
        if (!file)
        {
            debugger.logMessage("ERROR", __LINE__, "Compilation", "Source file not found");
            CONDITION_FAILED;
        }

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
