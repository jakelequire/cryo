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
#include "codegen/oldCodeGen.hpp"

namespace Cryo
{
    void Compilation::compileIRFile(void)
    {
        CryoContext &cryoContext = compiler.getContext();
        CompilerSettings *settings = compiler.getCompilerSettings();

        DevDebugger::logMessage("INFO", __LINE__, "Compilation", "Compiling IR File");
        std::cout << "\n\n";
        if (llvm::verifyModule(*cryoContext.module, &llvm::errs()))
        {
            LLVM_MODULE_FAILED_MESSAGE_START;
            cryoContext.module->print(llvm::errs(), nullptr);
            LLVM_MODULE_FAILED_MESSAGE_END;
            LLVM_MODULE_ERROR_START;
            // Get the error itself without the module showing up
            std::string errorMessage = getErrorMessage();
            if (!errorMessage.empty())
            {
                llvm::errs() << errorMessage;
                std::cout << "\n\n";
            }
            LLVM_MODULE_ERROR_END;
            std::cout << "\n\n";

            DevDebugger::logMessage("ERROR", __LINE__, "Compilation", "LLVM module verification failed");

            exit(1);
        }

        std::cout << "\n Getting the output path\n";
        const char *unsafe_filePath = strdup(settings->inputFile);
        std::string outputFilePath(unsafe_filePath);
        std::filesystem::path cwd = std::filesystem::current_path();

        // Trim the directory path from the file name
        std::string cwd_str = cwd.c_str();
        std::string outputPath = cwd_str + "/build/out/";
        // trimmedOutputFile = "main.cryo"
        std::string trimmedOutputFile = outputFilePath.substr(outputFilePath.find_last_of("/\\") + 1);
        // trimmedFileExt = "main"
        std::string trimmedFileExt = trimmedOutputFile.substr(0, trimmedOutputFile.find_last_of("."));
        std::string outputFileIR = trimmedFileExt + ".ll";
        std::string outputFileDir = outputPath + outputFileIR;

        std::cout << "\n\n\n Output Path: " << outputPath << "\n\n\n";
        std::cout << "\n\n\n Trimmed File: " << trimmedOutputFile << "\n\n\n";
        std::cout << "\n\n\n Trimmed File Ext: " << trimmedFileExt << "\n\n\n";
        std::cout << "\n\n\n Output File IR: " << outputFileIR << "\n\n\n";
        std::cout << "\n\n\n Output File Dir: " << outputFileDir << "\n\n\n";
        if (settings->customOutputPath)
        {
            // outputPath = std::string(settings->customOutputPath) + "/" + outputFile;
            // std::cout << "\n\n\n <!> <!> <!> \nCUSTOM OUTPUT PATH: " << outputPath << "\n\n\n";
        }

        // Check the output path if it's a valid string (non-utf8 characters)
        if (outputPath.empty())
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Compilation", "Invalid output path");
            return;
        }

        std::cout << "\n\n\n <!> <!> <!> \nFINAL OUTPUT PATH: " << outputPath << "\n\n\n";
        // Ensure the output directory exists
        bool didMkDir = std::filesystem::create_directories(outputPath);
        if (!didMkDir)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Compilation", "Output directory already exists");
        }
        else
        {
            DevDebugger::logMessage("INFO", __LINE__, "Compilation", "Output directory created");
        }

        std::error_code EC;
        llvm::raw_fd_ostream dest(outputFileDir, EC, llvm::sys::fs::OF_None);
        if (EC)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Compilation", "Error opening file for writing: " + EC.message());
            return;
        }
        LLVM_MODULE_COMPLETE_START;
        LoadStoreWhitespaceAnnotator LSWA;

        // Use the custom annotator when printing
        cryoContext.module->print(dest, &LSWA);
        cryoContext.module->print(llvm::outs(), &LSWA);
        LLVM_MODULE_COMPLETE_END;

        dest.close();

        DevDebugger::logMessage("INFO", __LINE__, "Compilation", "Compilation Complete");
        return;
    }

    std::string Compilation::getErrorMessage(void)
    {
        CryoContext &cryoContext = compiler.getContext();
        std::string ErrorMsg;
        llvm::raw_string_ostream ErrorStream(ErrorMsg);

        bool Err = llvm::verifyModule(*cryoContext.module, &llvm::errs());

        return ErrorStream.str();
    }

    /// @private
    void Compilation::compile(std::string inputFile, std::string outputPath)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Compilation", "outputPath: " + outputPath);
        // Check if the file exists
        // std::ifstream file(outputPath);
        // if (!file)
        // {
        //     DevDebugger::logMessage("ERROR", __LINE__, "Compilation", "Source file not found");
        //     CONDITION_FAILED;
        // }

        std::error_code EC;
        llvm::raw_fd_ostream dest(outputPath, EC, llvm::sys::fs::OF_None);
        if (EC)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Compilation", "Error opening file for writing");
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

            DevDebugger::logMessage("INFO", __LINE__, "Compilation", "Code CodeGen Complete");

            return;
        }
    }

    /// @private
    void Compilation::isValidDir(std::string dirPath)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Compilation", "Checking if directory exists");

        std::filesystem::path dir(dirPath);
        if (!std::filesystem::exists(dir))
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Compilation", "Directory does not exist, creating one...");
            makeOutputDir(dirPath);
            return;
        }

        return;
    }

    void Compilation::isValidFile(std::string filePath)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Compilation", "Checking if file exists");

        std::ifstream file(filePath);
        if (!file)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Compilation", "File does not exist");
            CONDITION_FAILED;
        }

        return;
    }

    /// @private
    void Compilation::makeOutputDir(std::string dirPath)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Compilation", "Creating output directory");

        std::filesystem::path dir(dirPath);
        if (!std::filesystem::exists(dir))
        {
            std::filesystem::create_directories(dir);
        }
    }
}