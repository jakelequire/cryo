/********************************************************************************
 *  Copyright 2025 Jacob LeQuire                                                *
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

/**
 * @file EOC.cpp
 * @brief Implements the End of Compilation linking processes
 *
 * This file contains functionality for the final linking stage of the compilation process.
 * It handles converting IR to object files, linking with standard libraries, and creating
 * the final executable.
 */

#include "linker/linker.hpp"
#include "diagnostics/diagnostics.h"
#include <filesystem>
#include <iostream>
#include <vector>
#include <cstring>

namespace Cryo
{

    /**
     * @brief Main entry point for the linking process
     *
     * Called at the end of compilation to link all components together and create
     * the final executable.
     */
    void Linker::linkAll()
    {
        __STACK_FRAME__
        logMessage(LMI, "INFO", "Linker", "Starting final linking process...");

        // Convert IR to object files
        if (!convertIRToObjects())
        {
            logMessage(LMI, "ERROR", "Linker", "Failed to convert IR to object files");
            return;
        }

        // Link all components into a final executable
        if (!linkFinalExecutable())
        {
            logMessage(LMI, "ERROR", "Linker", "Failed to link final executable");
            return;
        }

        // Run the completed binary if requested
        if (this->autoRunEnabled)
        {
            runCompletedBinary();
        }

        logMessage(LMI, "INFO", "Linker", "Linking process completed successfully");
    }

    void Linker::linkMain(llvm::Module *mod)
    {
        __STACK_FRAME__
        logMessage(LMI, "INFO", "Linker", "@linkMain Linking main module");
        if (!mod)
        {
            logMessage(LMI, "ERROR", "Linker", "Module is null");
            return;
        }

        // Generate IR from the module
        std::string outputPath = this->dirInfo->buildDir + "/out/main.ll";
        generateIRFromCodegen(mod, outputPath.c_str());
    }

    /**
     * @brief Converts all LLVM IR files to object files
     *
     * @return true if all conversions were successful, false otherwise
     */
    bool Linker::convertIRToObjects()
    {
        __STACK_FRAME__
        std::string buildDir = GetCXXLinker()->getDirInfo()->buildDir;
        if (buildDir.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "Build directory is empty");
            return false;
        }
        std::string compilerDir = GetCXXLinker()->getDirInfo()->compilerDir;
        if (compilerDir.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "Compiler directory is empty");
            return false;
        }

        std::string objDir = buildDir + "/out/obj";
        std::string irDir = buildDir + "/out";

        // Ensure output directory exists
        if (!ensureDirectoryExists(objDir))
        {
            logMessage(LMI, "ERROR", "Linker", "Failed to create object directory");
            return false;
        }

        // Convert main.ll to main.o
        if (!convertMainIRToObject(irDir, objDir))
        {
            return false;
        }

        // Convert any additional module IR files
        return convertModuleIRsToObjects(irDir, objDir);
    }

    /**
     * @brief Ensures a directory exists, creating it if needed
     *
     * @param dirPath Directory path to check/create
     * @return true if directory exists or was created, false otherwise
     */
    bool Linker::ensureDirectoryExists(const std::string &dirPath)
    {
        if (!std::filesystem::exists(dirPath))
        {
            try
            {
                std::filesystem::create_directories(dirPath);
                logMessage(LMI, "INFO", "Linker", "Created directory: %s", dirPath.c_str());
            }
            catch (const std::filesystem::filesystem_error &e)
            {
                logMessage(LMI, "ERROR", "Linker", "Failed to create directory %s: %s",
                           dirPath.c_str(), e.what());
                return false;
            }
        }
        return true;
    }

    /**
     * @brief Converts the main IR file to an object file
     *
     * @param irDir Directory containing IR files
     * @param objDir Directory for output object files
     * @return true if conversion was successful, false otherwise
     */
    bool Linker::convertMainIRToObject(const std::string &irDir, const std::string &objDir)
    {
        std::string mainIRFile = irDir + "/main.ll";
        std::string mainObjFile = objDir + "/main.o";

        if (!std::filesystem::exists(mainIRFile))
        {
            logMessage(LMI, "ERROR", "Linker", "Main IR file does not exist: %s", mainIRFile.c_str());
            return false;
        }

        std::string dso_path = this->dirInfo->compilerDir + "/cryo/Std/bin/libcryo_core.so";
        std::string load_path = "--load=" + dso_path;

        // Convert main.ll to main.o
        std::string llcCommand = "llc-18 -filetype=obj -relocation-model=pic " +
                                 load_path + " " +
                                 mainIRFile + " -o " + mainObjFile;

        logMessage(LMI, "INFO", "Linker", "Converting main IR to object: %s", llcCommand.c_str());
        int result = system(llcCommand.c_str());
        if (result != 0)
        {
            logMessage(LMI, "ERROR", "Linker", "Failed to convert main IR to object: %s",
                       std::strerror(errno));
            CONDITION_FAILED;
            return false;
        }

        logMessage(LMI, "INFO", "Linker", "Successfully converted main IR to object: %s",
                   mainObjFile.c_str());
        return true;
    }

    /**
     * @brief Converts all module IR files to object files
     *
     * @param irDir Directory containing IR files
     * @param objDir Directory for output object files
     * @return true if all conversions were successful, false otherwise
     */
    bool Linker::convertModuleIRsToObjects(const std::string &irDir, const std::string &objDir)
    {
        // Scan for additional module IR files
        for (const auto &entry : std::filesystem::directory_iterator(irDir))
        {
            if (entry.path().extension() == ".ll" &&
                entry.path().filename() != "main.ll")
            {

                std::string moduleName = entry.path().stem().string();
                std::string moduleIRPath = entry.path().string();
                std::string moduleObjPath = objDir + "/" + moduleName + ".o";

                // Convert module IR to object file
                std::string llcCommand = "llc-18 -filetype=obj -relocation-model=pic " +
                                         moduleIRPath + " -o " + moduleObjPath;

                logMessage(LMI, "INFO", "Linker", "Converting module IR to object: %s",
                           llcCommand.c_str());
                int result = system(llcCommand.c_str());

                if (result != 0)
                {
                    logMessage(LMI, "ERROR", "Linker", "Failed to convert module IR to object: %s",
                               std::strerror(errno));
                    CONDITION_FAILED;
                    return false;
                }

                logMessage(LMI, "INFO", "Linker", "Successfully converted module IR to object: %s",
                           moduleObjPath.c_str());
            }
        }

        return true;
    }

    /**
     * @brief Links all object files and standard libraries into a final executable
     *
     * @return true if linking was successful, false otherwise
     */
    bool Linker::linkFinalExecutable()
    {
        __STACK_FRAME__

        // Collect paths and files
        std::string buildDir = GetCXXLinker()->getDirInfo()->buildDir;
        std::string compilerDir = GetCXXLinker()->getDirInfo()->compilerDir;
        std::string objDir = buildDir + "/out/obj";
        std::string irDir = buildDir + "/out/.ll";
        std::string outputPath = buildDir + "/main";

        if (!std::filesystem::exists(objDir))
        {
            logMessage(LMI, "ERROR", "Linker", "Object directory does not exist: %s", objDir.c_str());
            return false;
        }

        // Collect all object files
        std::vector<std::string> objectFiles = collectObjectFiles(objDir);
        if (objectFiles.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "No object files found in %s", objDir.c_str());
            return false;
        }

        // Collect all standard library files
        std::vector<std::string> stdLibFiles = collectStandardLibraries(compilerDir);

        // Create linking command
        std::string linkCommand = buildLinkCommand(outputPath, objectFiles, stdLibFiles);

        // Execute linking command
        return executeLinkCommand(linkCommand, outputPath);
    }

    /**
     * @brief Collects all object files in the specified directory
     *
     * @param objDir Directory containing object files
     * @return Vector of object file paths
     */
    std::vector<std::string> Linker::collectObjectFiles(const std::string &objDir)
    {
        std::vector<std::string> objectFiles;

        for (const auto &entry : std::filesystem::directory_iterator(objDir))
        {
            if (entry.path().extension() == ".o")
            {
                objectFiles.push_back(entry.path().string());
            }
        }

        logMessage(LMI, "INFO", "Linker", "Found %zu object files", objectFiles.size());
        return objectFiles;
    }

    /**
     * @brief Collects all standard library files
     *
     * @param compilerDir Compiler root directory
     * @return Vector of standard library file paths
     */
    std::vector<std::string> Linker::collectStandardLibraries(const std::string &compilerDir)
    {
        std::vector<std::string> stdLibFiles;
        std::string stdLibDir = compilerDir + "/cryo/Std/bin/.ll";

        if (!std::filesystem::exists(stdLibDir))
        {
            logMessage(LMI, "WARNING", "Linker", "Standard library directory does not exist: %s",
                       stdLibDir.c_str());
            return stdLibFiles;
        }

        // Find all .ll files in the standard library directory
        for (const auto &entry : std::filesystem::directory_iterator(stdLibDir))
        {
            if (entry.path().extension() == ".ll")
            {
                // Convert to a .o file
                std::string libFile = entry.path().string();
                std::string libName = entry.path().stem().string();
                std::string libPath = stdLibDir + "/" + libName + ".o";
                std::string llcCommand = "llc-18 -filetype=obj -relocation-model=pic " +
                                         libFile + " -o " + libPath;
                int result = system(llcCommand.c_str());
                if (result != 0)
                {
                    logMessage(LMI, "ERROR", "Linker", "Failed to convert standard library file: %s",
                               std::strerror(errno));
                    CONDITION_FAILED;
                    continue;
                }

                stdLibFiles.push_back(libPath);
                logMessage(LMI, "INFO", "Linker", "Converted standard library file to object: %s",
                           libPath.c_str());
            }
            else
            {
                logMessage(LMI, "WARNING", "Linker", "Non-IR file found in standard library directory: %s",
                           entry.path().string().c_str());
                logMessage(LMI, "WARNING", "Linker", "Ignoring file: %s",
                           entry.path().filename().string().c_str());
            }
        }

        logMessage(LMI, "INFO", "Linker", "Found %zu standard library files", stdLibFiles.size());
        return stdLibFiles;
    }

    /**
     * @brief Builds the linking command string
     *
     * @param outputPath Path for the output executable
     * @param objectFiles List of object files to link
     * @param stdLibFiles List of standard library files to link
     * @return Complete linking command
     */
    std::string Linker::buildLinkCommand(const std::string &outputPath,
                                         const std::vector<std::string> &objectFiles,
                                         const std::vector<std::string> &stdLibFiles)
    {
        std::string command = "clang++-18 -pie -o " + outputPath;

        for (const auto &lib : stdLibFiles)
        {
            command += " " + lib;
        }

        // Add the object files to the command
        for (const auto &obj : objectFiles)
        {
            command += " " + obj;
        }

        logMessage(LMI, "INFO", "Linker", "<!> Linking command: %s", command.c_str());

        return command;
    }

    /**
     * @brief Executes the linking command
     *
     * @param linkCommand Command to execute
     * @param outputPath Expected output path
     * @return true if linking was successful, false otherwise
     */
    bool Linker::executeLinkCommand(const std::string &linkCommand, const std::string &outputPath)
    {
        logMessage(LMI, "INFO", "Linker", "Executing linking command: %s", linkCommand.c_str());
        int result = system(linkCommand.c_str());
        if (result != 0)
        {
            logMessage(LMI, "ERROR", "Linker", "Linking failed: %s", std::strerror(errno));
            CONDITION_FAILED;
            return false;
        }

        if (!std::filesystem::exists(outputPath))
        {
            logMessage(LMI, "ERROR", "Linker", "Linking appeared to succeed, but output file not found");
            return false;
        }

        logMessage(LMI, "INFO", "Linker", "Successfully linked executable at %s", outputPath.c_str());
        return true;
    }

    /**
     * @brief Legacy function for compatibility
     *
     * Creates the main object file and links it with other objects to form the final executable.
     */
    void Linker::createMainObject()
    {
        __STACK_FRAME__
        logMessage(LMI, "INFO", "Linker", "Creating main object file...");

        // Use the new implementation through convertIRToObjects
        if (!convertIRToObjects())
        {
            logMessage(LMI, "ERROR", "Linker", "Failed to create main object file");
            CONDITION_FAILED;
            return;
        }

        // Link all objects to create the final executable
        combineAllObjectsToMainExe();
    }

    /**
     * @brief Legacy function for compatibility
     *
     * Links all object files to create the final executable.
     */
    void Linker::combineAllObjectsToMainExe()
    {
        __STACK_FRAME__
        logMessage(LMI, "INFO", "Linker", "Combining objects to create executable...");

        // Use the new implementation through linkFinalExecutable
        if (!linkFinalExecutable())
        {
            logMessage(LMI, "ERROR", "Linker", "Failed to combine objects to create executable");
            CONDITION_FAILED;
        }
    }

    /**
     * @brief Signals the end of code generation and begins the linking process
     *
     * This function is called when IR generation is complete and the main file
     * is ready to be linked with other components.
     */
    void Linker::completeCodeGeneration()
    {
        __STACK_FRAME__
        logMessage(LMI, "INFO", "Linker", "Code generation completed, starting linking process...");

        // Start the linking process
        linkAll();
    }

    /**
     * @brief Runs the completed binary
     *
     * This function is called after successful linking to execute the program.
     */
    void Linker::runCompletedBinary()
    {
        __STACK_FRAME__
        std::string buildDir = GetCXXLinker()->getDirInfo()->buildDir;
        if (buildDir.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "Build directory is empty");
            return;
        }

        std::string executablePath = buildDir + "/main";
        if (!std::filesystem::exists(executablePath))
        {
            logMessage(LMI, "ERROR", "Linker", "Executable does not exist: %s", executablePath.c_str());
            return;
        }

        logMessage(LMI, "INFO", "Linker", "Running executable: %s", executablePath.c_str());
        std::cout << "\nExecuting binary: " << executablePath << "\n"
                  << std::endl;

        int result = system(executablePath.c_str());
        int exitCode = WEXITSTATUS(result);

        logMessage(LMI, "INFO", "Linker", "Program completed with exit code %d", exitCode);
    }

} // namespace Cryo
