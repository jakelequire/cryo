
#include "cpp/codegen.h"

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
        CryoDebugger &debugger = compiler.getDebugger();
        Generator &generator = compiler.getGenerator();
        debugger.logMessage("INFO", __LINE__, "Imports", "Handling Import Statement");

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

        debugger.logMessage("INFO", __LINE__, "Imports", "Import Statement Handled");
    }

    void Imports::importCryoSTD(std::string subModuleName)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Imports", "Importing Cryo Standard Library");

        try
        {
            std::string cryoPath = getCryoPath();
            std::string relativePath = "./cryo/std/" + subModuleName + ".cryo";

            // Check if the file exists
            if (!std::filesystem::exists(relativePath))
            {
                throw std::runtime_error("Cryo STD file not found: " + relativePath);
            }
            debugger.logMessage("INFO", __LINE__, "Imports", "Cryo STD file found.");

            const char *compilerFlags = "-a -o ./build/out/imports";

            debugger.logMessage("INFO", __LINE__, "Imports", "Compiling Cryo STD file...");
            CompiledFile compiledFile = compileFile(relativePath.c_str(), compilerFlags);
            debugger.logMessage("INFO", __LINE__, "Imports", "Cryo STD file compilation completed.");

            std::string compiledIRFile = findIRBuildFile(compiledFile.fileName);
            if (compiledIRFile.empty())
            {
                throw std::runtime_error("Compiled IR file not found for " + std::string(compiledFile.fileName));
            }

            // Load and link the compiled IR file
            llvm::SMDiagnostic Err;
            llvm::LLVMContext &Context = compiler.getContext().context;
            std::unique_ptr<llvm::Module> ImportedModule = llvm::parseIRFile(compiledIRFile, Err, Context);

            if (!ImportedModule)
            {
                throw std::runtime_error("Failed to load IR file: " + compiledIRFile);
            }

            // Link the imported module with the main module
            llvm::Linker Linker(*compiler.getContext().module);
            if (Linker.linkInModule(std::move(ImportedModule)))
            {
                throw std::runtime_error("Failed to link imported module");
            }

            debugger.logMessage("INFO", __LINE__, "Imports", "Successfully linked imported module.");

            // Continue with the main compilation process
            // This should be handled by returning to the main compilation flow
            // The calling function should continue with the rest of the compilation process

            return;
        }
        catch (const std::exception &e)
        {
            debugger.logMessage("ERROR", __LINE__, "Imports", e.what());
            // Handle the error appropriately, e.g., set a default path or exit the program
            throw; // Re-throw the exception to be handled by the caller
        }

        return;
    }

    std::string Imports::findIRBuildFile(std::string fileName)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Imports", "Finding IR Build File");

        // Get the cryo ENV variable
        const char *cryoEnv = std::getenv("CRYO_PATH");
        if (cryoEnv == nullptr || std::strcmp(cryoEnv, "") == 0)
        {
            debugger.logMessage("ERROR", __LINE__, "Imports", "CRYO_PATH environment variable not set.");
            return "";
        }

        // Get the executable path (CRYO_PATH/build/out/imports)
        std::string executablePath = std::string(cryoEnv) + "/build/out/imports/";
        std::cout << "Executable Path: " << executablePath << std::endl;

        // Change the file extension from `.cryo` to `.ll`
        std::string irFileName = fileName.substr(0, fileName.find_last_of(".")) + ".ll";
        std::cout << "IR File Name: " << irFileName << std::endl;

        std::string fullFilePath = executablePath + irFileName;
        std::cout << "Full File Path: " << fullFilePath << std::endl;

        // Check if the file exists
        if (!std::filesystem::exists(fullFilePath))
        {
            debugger.logMessage("ERROR", __LINE__, "Imports", "File does not exist.");
            return "";
        }
        debugger.logMessage("INFO", __LINE__, "Imports", "File found.");

        return fullFilePath;
    }

}
