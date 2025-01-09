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
#include "codegen/utility/highlighter.hpp"
#include "codegen/oldCodeGen.hpp"
#include "tools/logger/logger_config.h"

namespace Cryo
{
    void Compilation::compileIRFile(void)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Compilation", "Compiling IR File");
        CryoContext &cryoContext = compiler.getContext();
        CompilerSettings *settings = compiler.getCompilerSettings();

        // Linker *linker = compiler.getLinker();

        // Hoist the declarations from the linker
        DevDebugger::logMessage("INFO", __LINE__, "Compilation", "Hoisting Declarations");
        // linker->hoistDeclarations(compiler.getContext().module.get());

        if (llvm::verifyModule(*cryoContext.module, nullptr))
        {
            DEBUG_PRINT_FILTER({
                LLVM_MODULE_FAILED_MESSAGE_START;

                LLVMIRHighlighter highlighter;
                llvm::formatted_raw_ostream formatted_errs(llvm::errs());
                highlighter.printWithHighlighting(cryoContext.module.get(), formatted_errs);

                LLVM_MODULE_FAILED_MESSAGE_END;
                LLVM_MODULE_ERROR_START;
            });
            // Get the error itself without the module showing up
            std::string errorMessage = getErrorMessage();
            if (!errorMessage.empty())
            {
                llvm::errs() << errorMessage;
                std::cout << "\n\n";
            }
            DEBUG_PRINT_FILTER({
                LLVM_MODULE_ERROR_END;
                std::cout << "\n\n";
            });

            DevDebugger::logMessage("ERROR", __LINE__, "Compilation", "LLVM module verification failed");

            // Output the broken IR to a file
            // outputFailedIR();

            exit(1);
        }

        bool isPreprocessing = compiler.isPreprocessing;
        if (isPreprocessing)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Compilation", "Preprocessing Compilation");
            std::string outputPath = compiler.customOutputPath;
            compileUniquePath(outputPath);
            return;
        }

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

        DEBUG_PRINT_FILTER({
            std::cout << "\n\n\n Output Path: " << outputPath << "\n";
            std::cout << "Trimmed File: " << trimmedOutputFile << "\n";
            std::cout << "Trimmed File Ext: " << trimmedFileExt << "\n";
            std::cout << "Output File IR: " << outputFileIR << "\n";
            std::cout << "Output File Dir: " << outputFileDir << "\n\n\n";
        });
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

        DEBUG_PRINT_FILTER({
            LLVM_MODULE_COMPLETE_START;
            LoadStoreWhitespaceAnnotator LSWA;

            LLVMIRHighlighter highlighter;
            // We need to print the highlighted version to the console
            // and the raw version to the file
            cryoContext.module->print(dest /* llvm::outs() */, &LSWA);
            dest.close();

            llvm::formatted_raw_ostream formatted_out(llvm::outs());
            highlighter.printWithHighlighting(cryoContext.module.get(), formatted_out);
            LLVM_MODULE_COMPLETE_END;
        });

        DevDebugger::logMessage("INFO", __LINE__, "Compilation", "Compilation Complete");
        return;
    }

    void Compilation::outputFailedIR(void)
    {
        // Put the broken IR in the build directory under /out/errors/{MODULE_NAME}.error.ll
        std::string outputPath = compiler.getCompilerSettings()->inputFile;
        std::filesystem::path cwd = std::filesystem::current_path();
        std::string cwd_str = cwd.c_str();
        std::string outputDir = cwd_str + "/build/out/errors/";
        std::string outputFileName = outputPath.substr(outputPath.find_last_of("/\\") + 1);
        std::string outputFilePath = outputDir + outputFileName + ".error.ll";

        std::error_code EC;
        llvm::raw_fd_ostream dest(outputFilePath, EC, llvm::sys::fs::OF_None);
        if (EC)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Compilation", "Error opening file for writing");
            return;
        }

        DEBUG_PRINT_FILTER({ LLVM_MODULE_COMPLETE_START; });
        LoadStoreWhitespaceAnnotator LSWA;

        // Use the custom annotator when printing
        compiler.getContext().module->print(dest, &LSWA);
        DEBUG_PRINT_FILTER({ compiler.getContext().module->print(llvm::outs(), &LSWA); });

        DEBUG_PRINT_FILTER({ LLVM_MODULE_COMPLETE_END; });

        dest.close();

        return;
    }

    void Compilation::DumpModuleToDebugFile(void)
    {
        // Put the broken IR in the build directory under /out/errors/{MODULE_NAME}.debug.ll
        std::filesystem::path cwd = std::filesystem::current_path();
        std::string cwd_str = cwd.c_str();
        std::string outputDir = cwd_str + "/build/out/errors/";
        std::string outputFileName = "debug.ll";
        std::string outputFilePath = outputDir + outputFileName;

        // Make the directory
        DevDebugger::logMessage("INFO", __LINE__, "Compilation", "Creating output directory:\n " + outputDir);
        makeOutputDir(outputDir);

        // Clean the directory
        cleanErrorDir();

        std::error_code EC;
        llvm::raw_fd_ostream dest(outputFilePath, EC, llvm::sys::fs::OF_None);
        if (EC)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Compilation", "Error opening file for writing: " + EC.message());
            std::cout << "File Path: " << outputFilePath << std::endl;
            CONDITION_FAILED;
            return;
        }

        DEBUG_PRINT_FILTER({ LLVM_MODULE_COMPLETE_START; });
        LoadStoreWhitespaceAnnotator LSWA;

        // Use the custom annotator when printing
        compiler.getContext().module->print(dest, &LSWA);
        DEBUG_PRINT_FILTER({ compiler.getContext().module->print(llvm::outs(), &LSWA); });

        DEBUG_PRINT_FILTER({ LLVM_MODULE_COMPLETE_END; });

        dest.close();

        return;
    }

    void Compilation::compileUniquePath(std::string outputPath)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Compilation", "Compiling Unique Path");
        CryoContext &cryoContext = compiler.getContext();

        // Ensure the output directory exists
        std::string outputDir = outputPath.substr(0, outputPath.find_last_of("/\\"));
        isValidDir(outputDir);

        std::error_code EC;
        llvm::raw_fd_ostream dest(outputPath, EC, llvm::sys::fs::OF_None);
        if (EC)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Compilation", "Error opening file for writing");
            return;
        }

        DevDebugger::logMessage("INFO", __LINE__, "Compilation", "Code CodeGen Complete");

        DEBUG_PRINT_FILTER({ LLVM_MODULE_COMPLETE_START; });
        LoadStoreWhitespaceAnnotator LSWA;

        // Use the custom annotator when printing
        cryoContext.module->print(dest, &LSWA);
        DEBUG_PRINT_FILTER({ cryoContext.module->print(llvm::outs(), &LSWA); });
        DEBUG_PRINT_FILTER({ LLVM_MODULE_COMPLETE_END; });

        dest.close();

        return;
    }

    std::string Compilation::getErrorMessage(void)
    {
        CryoContext &cryoContext = compiler.getContext();
        std::string ErrorMsg;
        llvm::raw_string_ostream ErrorStream(ErrorMsg);

        // Verify module and capture output in our string stream instead of errs()
        bool Err = llvm::verifyModule(*cryoContext.module, &ErrorStream);
        ErrorStream.flush();

        // Apply syntax highlighting to the error message
        LLVMIRHighlighter highlighter;
        std::string highlighted = highlighter.highlightText(ErrorMsg);

        return highlighted;
    }

    /// @private
    void Compilation::compile(std::string inputFile, std::string outputPath)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Compilation", "outputPath: " + outputPath);

        std::error_code EC;
        llvm::raw_fd_ostream dest(outputPath, EC, llvm::sys::fs::OF_None);
        if (EC)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Compilation", "Error opening file for writing");
        }
        else
        {
            DEBUG_PRINT_FILTER({
                std::cout << "\n>===------- LLVM IR Code -------===<\n"
                          << std::endl;
            });

            compiler.getContext().getModules()->back()->print(dest, nullptr);

            // Create our custom annotator
            LoadStoreWhitespaceAnnotator LSWA;

            // Use the custom annotator when printing
            compiler.getModule().print(dest, &LSWA);
            DEBUG_PRINT_FILTER({ compiler.getModule().print(llvm::outs(), &LSWA); });

            DEBUG_PRINT_FILTER({
                std::cout << "\n>===------- End IR Code ------===<\n"
                          << std::endl;
            });

            dest.flush();
            dest.close();

            DevDebugger::logMessage("INFO", __LINE__, "Compilation", "Code CodeGen Complete");

            return;
        }
    }

    llvm::Module *Compilation::compileAndMergeModule(std::string inputFile)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Compilation", "Compiling and Merging Module");

        llvm::LLVMContext &context = compiler.getContext().context;
        llvm::SMDiagnostic err;

        std::unique_ptr<llvm::Module> module = llvm::parseIRFile(inputFile, err, context);
        if (!module)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Compilation", "Failed to parse IR file");
            CONDITION_FAILED;
        }

        return module.release();
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
            DevDebugger::logMessage("INFO", __LINE__, "Compilation", "Creating directory: " + dirPath);
            if (!std::filesystem::create_directories(dir))
            {
                DevDebugger::logMessage("ERROR", __LINE__, "Compilation", "Failed to create directory");
                std::cout << "Error: " << std::strerror(errno) << std::endl;
                CONDITION_FAILED;
            }
        }

        return;
    }

    void Compilation::cleanErrorDir(void)
    {
        std::filesystem::path cwd = std::filesystem::current_path();
        std::string cwd_str = cwd.c_str();
        std::string outputDir = cwd_str + "/build/out/errors/";

        // Only clear the files, keep the directory
        for (const auto &entry : std::filesystem::directory_iterator(outputDir))
        {
            std::filesystem::remove(entry.path());
        }

        return;
    }
}
