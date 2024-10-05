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
        std::cout << "[Compilation] Params passed: " << irFilePath << ", " << irFileName << std::endl;
        std::string _irFilePath;
        std::string customOutputPath = compiler.getCompilerState()->settings->customOutputPath;
        if (customOutputPath != "")
        {
            _irFilePath = customOutputPath + "/" + irFilePath;

            std::cout << "[Compilation] Output Path: " << customOutputPath << std::endl;
            // Check if the directory exists
            isValidDir(customOutputPath);

            // Compile the file
            compile(_irFilePath);
        }
    }

    void Compilation::compile(std::string outputPath)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Compilation", "Compiling Source File");

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
