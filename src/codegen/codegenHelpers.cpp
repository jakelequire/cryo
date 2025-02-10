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
#include "tools/logger/logger_config.h"

namespace Cryo
{
    // + ======================================================================================== + //
    // +                              Cryo Context Functions                                      + //
    // + ======================================================================================== + //

    llvm::GlobalVariable *CryoContext::getOrCreateGlobalString(const std::string &content)
    {
        // First check if we already have this string
        auto it = stringTable.find(content);
        if (it != stringTable.end())
        {
            return it->second;
        }

        // Create a new global string constant
        llvm::Constant *stringConstant = llvm::ConstantDataArray::getString(context, content);

        // Generate a unique name for this string
        std::string globalName = "str." + std::to_string(stringCounter++);

        // Create the global variable
        llvm::GlobalVariable *globalStr = new llvm::GlobalVariable(
            *module,
            stringConstant->getType(),
            true, // isConstant
            llvm::GlobalValue::PrivateLinkage,
            stringConstant,
            globalName);

        // Store in our table
        stringTable[content] = globalStr;

        return globalStr;
    }

    void CryoContext::initializeContext()
    {
        // Get the filename from the CompilerState
        std::string moduleName = "CryoModuleDefaulted";
        module = std::make_unique<llvm::Module>(moduleName, context);
    }

    void CryoContext::setModuleIdentifier(std::string name)
    {
        module->setModuleIdentifier(name);
        module->setSourceFileName(name);
    }

    void CryoContext::addCompiledFileInfo(CompiledFile file)
    {
        compiledFiles.push_back(file);
    }

    void CryoContext::addStructToInstance(std::string name, llvm::StructType *structType)
    {
        structTypes[name] = structType;
    }

    llvm::StructType *CryoContext::getStruct(std::string name)
    {
        return structTypes[name];
    }

    void CryoContext::addClassToInstance(std::string name, llvm::StructType *classType)
    {
        classTypes[name] = classType;
    }

    llvm::StructType *CryoContext::getClass(std::string name)
    {
        return classTypes[name];
    }

    void CryoContext::addStructDataType(std::string name, DataType *dataType)
    {
        structDataTypes[name] = dataType;
    }

    void CryoContext::addClassDataType(std::string name, DataType *dataType)
    {
        classDataTypes[name] = dataType;
    }

    void CryoCompiler::setBuildDir(std::string dir)
    {
        if (dir.empty())
        {
            CONDITION_FAILED;
            return;
        }
        buildDir = dir;
    }

    void CryoCompiler::setOutputFile(std::string file)
    {
        if (file.empty())
        {
            CONDITION_FAILED;
            return;
        }
        outputFile = file;
    }

    void CryoCompiler::preInitMain(void)
    {
        // Merge in runtime dependencies
        llvm::Module *depMod = this->getLinker()->initMainModule();
        CryoContext::getInstance().mergeModule(depMod);
    }

    // + ======================================================================================== + //
    // +                              Cryo Compiler Functions                                     + //
    // + ======================================================================================== + //

    void CryoCompiler::setModuleIdentifier(std::string name)
    {
        CryoContext::getInstance().setModuleIdentifier(name);
    }

    void CryoCompiler::setCustomOutputPath(std::string path)
    {
        customOutputPath = path;
    }

    void CryoCompiler::setCustomOutputPath(std::string path, bool isImport)
    {
        customOutputPath = path;
        isImporting = isImport;
        isPreprocessing = false;
    }

    void CryoCompiler::setPreprocessOutputPath(std::string path)
    {
        customOutputPath = path;
        isPreprocessing = true;
    }

    void CryoCompiler::initDependencies()
    {
        // Get the dependency module
        std::cout << "@initDependencies Initializing Dependencies" << std::endl;
        CryoContext::getInstance().mergeModule(GetCXXLinker()->initMainModule());
    }

    void CryoCompiler::linkDependencies(void)
    {
        std::cout << "@linkDependencies Linking Dependencies" << std::endl;
        // CryoContext::getInstance().mergeModule(std::move(GetCXXLinker()->appendDependenciesToRoot()));
        DEBUG_BREAKPOINT;
    }

    void CryoContext::printStructTypesMap(void)
    {
        std::cout << "\n";
        std::cout << "+ ---------- Struct Types Map ---------- +" << std::endl;
        for (auto &pair : structTypes)
        {
            std::cout << pair.first << std::endl;
        }
        std::cout << "+ -------------------------------------- +" << std::endl;
        std::cout << "\n";
    }
} // namespace Cryo
