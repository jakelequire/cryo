
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

            // Check if the file exists
            std::ifstream file(cryoSTDPath); // This function is from the <fstream> library
            debugger.logMessage("INFO", __LINE__, "Imports", "Cryo STD file found.");

            const char *cryoSTDPathCStr = cryoSTDPath.c_str();
            const char *compilerFlags = "-a";
            std::cout << "\n\n\n\n\n\n";
            compileFile(cryoSTDPathCStr, compilerFlags);

            return;
        }
        catch (const std::exception &e)
        {
            debugger.logMessage("ERROR", __LINE__, "Imports", e.what());
            // Handle the error appropriately, e.g., set a default path or exit the program
        }

        return;
    }
}
