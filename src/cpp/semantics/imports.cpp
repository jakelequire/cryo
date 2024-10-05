
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
            importCryoSTD(importNode->subModuleName);
        }

        return;
    }

    void Imports::importCryoSTD(std::string subModuleName)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Imports", "Importing Cryo Standard Library");

        // Find the submodule and import it to the current module
        // The location of the Cryo STD is going to be $CRYO_PATH/cryo/std/{subModuleName}.cryo
        // In your main function or wherever you're using this:
        try
        {
            std::string cryoPath = getCryoPath();
            std::string cryoSTDPath = cryoPath + "/cryo/std/" + subModuleName + ".cryo";
            std::cout << "Cryo STD Path: " << cryoSTDPath << std::endl;

            std::string relativePath = "./cryo/std/" + subModuleName + ".cryo";

            // Check if the file exists
            std::ifstream file(relativePath); // This function is from the <fstream> library
            debugger.logMessage("INFO", __LINE__, "Imports", "Cryo STD file found.");

            const char *compilerFlags = "-a -o ./build/out/imports";

            std::cout << "\n\n\n\n\n\n";
            debugger.logMessage("INFO", __LINE__, "Imports", "Compiling Cryo STD file...");
            std::cout << "File Path passed to `compileFile` in @imports: " << relativePath << std::endl;
            CompiledFile compiledFile = compileFile(relativePath.c_str(), compilerFlags);
            debugger.logMessage("INFO", __LINE__, "Imports", "Cryo STD file compilation compiled.");

            std::string irFileName = compiledFile.fileName;
            std::cout << "IR File Name: " << irFileName << std::endl;
            std::string irFilePath = compiledFile.filePath;
            std::cout << "IR File Path: " << irFilePath << std::endl;

            // Find the IR build file
            std::string compiledIRFile = findIRBuildFile(irFileName);
            std::cout << "Compiled IR File Path: " << compiledIRFile << std::endl;

            return;
        }
        catch (const std::exception &e)
        {
            debugger.logMessage("ERROR", __LINE__, "Imports", e.what());
            // Handle the error appropriately, e.g., set a default path or exit the program
            DEBUG_BREAKPOINT;
        }

        return;
    }

    std::string Imports::findIRBuildFile(std::string fileName)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Imports", "Finding IR Build File");

        // Get the cryo ENV variable
        const char *cryoEnv = std::getenv("CRYO_PATH");
        if (cryoEnv == nullptr || cryoEnv == "")
        {
            debugger.logMessage("ERROR", __LINE__, "Imports", "CRYO_PATH environment variable not set.");
            return nullptr;
        }

        // Get the executable path (CRYO_PATH/build/out/imports)
        std::string executablePath = std::string(cryoEnv) + "/build/out/imports/";
        std::cout << "Executable Path: " << executablePath << std::endl;

        // Change the file extension from `.cryo` to `.ll`
        std::string irFileName = fileName.substr(0, fileName.find_last_of("."));
        irFileName += ".ll";
        std::cout << "IR File Name: " << irFileName << std::endl;

        std::string fullFilePath = executablePath + irFileName;
        std::cout << "Full File Path: " << fullFilePath << std::endl;

        // Check if the file exists
        std::ifstream file(fullFilePath); // This function is from the <fstream> library
        if (!file)
        {
            debugger.logMessage("ERROR", __LINE__, "Imports", "File does not exist.");
            return nullptr;
        }
        debugger.logMessage("INFO", __LINE__, "Imports", "File found.");

        return fullFilePath;
    }

}
