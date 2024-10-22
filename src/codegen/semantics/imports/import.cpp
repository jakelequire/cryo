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

std::string getCryoPath()
{
    const char *cryoPathPtr = std::getenv("CRYO_PATH");
    if (cryoPathPtr == nullptr)
    {
        throw std::runtime_error("CRYO_PATH environment variable not set.");
    }
    return std::string(cryoPathPtr);
}

namespace Cryo
{
    void Imports::handleImportStatement(ASTNode *node)
    {
        Generator &generator = compiler.getGenerator();
        DevDebugger::logMessage("INFO", __LINE__, "Imports", "Handling Import Statement");

        CryoImportNode *importNode = node->data.import;

        std::cout << "Is STD Module: " << importNode->isStdModule << std::endl;
        std::cout << "Module Name: " << importNode->moduleName << std::endl;
        if (importNode->subModuleName)
        {
            std::cout << "Submodule Name: " << importNode->subModuleName << std::endl;
        }

        if (importNode->isStdModule)
        {
            std::cout << "Importing Cryo Standard Library" << std::endl;
            generator.addCommentToIR("Importing Cryo Standard Library");
            importCryoSTD(importNode->subModuleName);
        }
        else
        {
            // Handle non-standard library imports here
            std::cout << "Importing non-standard module" << std::endl;
            // Add your code to handle non-standard imports
        }

        DevDebugger::logMessage("INFO", __LINE__, "Imports", "Import Statement Handled");
    }

    void Imports::importCryoSTD(std::string subModuleName)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Imports", "Importing Cryo Standard Library");

        try
        {
            std::string cryoPath = getCryoPath();
            std::string relativePath = "./cryo/std/" + subModuleName + ".cryo";

            // Check if the file exists
            if (!std::filesystem::exists(relativePath))
            {
                throw std::runtime_error("Cryo STD file not found: " + relativePath);
            }
            DevDebugger::logMessage("INFO", __LINE__, "Imports", "Cryo STD file found.");

            CompiledFile _compiledFile;
            _compiledFile.fileName = subModuleName.c_str();
            _compiledFile.filePath = relativePath.c_str();
            _compiledFile.outputPath = "./build/out/imports";
            compiler.getContext().addCompiledFileInfo(_compiledFile);

            CompilerSettings *importSettings = (CompilerSettings *)malloc(sizeof(CompilerSettings));
            importSettings->rootDir = cryoPath.c_str();
            importSettings->inputFile = relativePath.c_str();
            importSettings->customOutputPath = "./build/out/imports";
            importSettings->activeBuild = true;

            // Get the current module
            llvm::Module *currentModule = &compiler.getModule();
            compiler.dumpModule();

            DevDebugger::logMessage("INFO", __LINE__, "Imports", "Compiling Cryo STD file...");
            int compiledFile = compileImportFileCXX(relativePath.c_str(), importSettings);
            if (compiledFile != 0)
            {
                throw std::runtime_error("Failed to compile Cryo STD file: " + relativePath);
            }
            DevDebugger::logMessage("INFO", __LINE__, "Imports", "Cryo STD file compiled successfully.");

            // Find the compiled IR file
            std::string IRFilePath = "./build/out/imports/" + subModuleName + ".ll";
            std::string compiledIRFile = findIRBuildFile(IRFilePath);
            if (compiledIRFile.empty())
            {
                throw std::runtime_error("Failed to find compiled IR file for Cryo STD file: " + relativePath);
            }

            DevDebugger::logMessage("INFO", __LINE__, "Imports", "Found compiled IR file.");
            std::cout << "Compiled IR File: " << compiledIRFile << std::endl;

            // Load and link the compiled IR file
            llvm::SMDiagnostic Err;
            // std::unique_ptr<llvm::Module> ImportedModule = llvm::parseIRFile(compiledIRFile, Err, compiler.getContext().context);
            // if (!ImportedModule)
            // {
            //     std::cerr << "Failed to load IR file: " << compiledIRFile << std::endl;
            //     throw std::runtime_error("Failed to load IR file: " + compiledIRFile);
            // }
            //
            // DevDebugger::logMessage("INFO", __LINE__, "Imports", "Successfully loaded IR file.");
            //
            // compiler.dumpModule();
            //
            // // Link the imported module with the main module
            // llvm::Linker Linker(*currentModule);
            // bool isLinked = Linker.linkInModule(std::move(ImportedModule));
            // if (isLinked)
            // {
            //     std::cerr << "\n\n\n <!> Failed to link module, aborting compilation <!> \n\n\n";
            //     throw std::runtime_error("Failed to link imported module.");
            // }

            DevDebugger::logMessage("INFO", __LINE__, "Imports", "Successfully linked imported module.");

            // Continue with the main compilation process
            // This should be handled by returning to the main compilation flow
            // The calling function should continue with the rest of the compilation process

            return;
        }
        catch (const std::exception &e)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Imports", e.what());
            // Handle the error appropriately, e.g., set a default path or exit the program
            throw; // Re-throw the exception to be handled by the caller
        }

        return;
    }

    std::string Imports::findIRBuildFile(std::string filePath)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Imports", "Finding IR Build File");

        // Check if the file exists
        if (!std::filesystem::exists(filePath))
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Imports", "File does not exist.");
            return "";
        }
        DevDebugger::logMessage("INFO", __LINE__, "Imports", "File found.");

        return filePath;
    }

}