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
#include "linker/linker.hpp"
#include "linker/init.h"
#include "diagnostics/diagnostics.h"

/// This files purpose is to initialize the Cryo Core and the Cryo Runtime.
/// In the root directory of the Cryo Compiler, there is a `cryolib` directory.
/// Inside this directory is `{COMPILER_DIR}/cryo/Std/Core/core.cryo` and
/// `{COMPILER_DIR}/cryo/runtime/runtime.c`.
///
/// The `core.cryo` file is the baseline for the Cryo Standard Library, equivalent
/// to C's CRT0. The `runtime.c` file is to be compiled along with the Cryo Core.
/// ```
///     A[Phase 1: C/C++ Implementation] --> B[Core Runtime]
///     A --> C[Basic Standard Library]
///     B --> D[Phase 2: Cryo Implementation]
///     C --> D
///     D --> E[Full Standard Library in Cryo]
/// ```

namespace Cryo
{

    void Linker::initCryoCore(const char *compilerRootPath, const char *buildDir,
                              CompilerState *state, CryoGlobalSymbolTable *globalTable)
    {
        __STACK_FRAME__
        // Create the llvm::Module for the `runtime.c` file.
        _initCRuntime_();

        if (!this->runtimeModuleInitialized)
        {
            fprintf(stderr, "[Linker] Error: Failed to initialize the runtime module\n");
            CONDITION_FAILED;
        }

        // Process the core.cryo
        const char *corePath = fs->appendStrings(compilerRootPath, "/cryo/Std/Core/core.cryo");
        if (!corePath)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to allocate memory for definitions path\n");
            CONDITION_FAILED;
        }

        ASTNode *defsNode = compileForASTNode(strdup(corePath), state, globalTable);
        if (!defsNode)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to compile definitions\n");
            CONDITION_FAILED;
        }

        DTM->symbolTable->importASTnode(DTM->symbolTable, defsNode);
        DTM->symbolTable->printTable(DTM->symbolTable);

        logMessage(LMI, "INFO", "DTM", "Definitions Path: %s", corePath);
        CompilationUnitDir dir = createCompilationUnitDir(corePath, buildDir, CRYO_DEPENDENCY);
        dir.print(dir);

        CompilationUnit *unit = createNewCompilationUnit(defsNode, dir);
        if (!unit)
        {
            logMessage(LMI, "ERROR", "CryoCompiler", "Failed to create CompilationUnit");
            CONDITION_FAILED;
            return;
        }
        if (unit->verify(unit) != 0)
        {
            logMessage(LMI, "ERROR", "CryoCompiler", "Failed to verify CompilationUnit");
            CONDITION_FAILED;
            return;
        }

        DEBUG_BREAKPOINT;
    }

    void Linker::_initCRuntime_(void)
    {
        logMessage(LMI, "INFO", "Linker", "Initializing C runtime module...");
        std::string compilerRootPath = this->dirInfo->compilerDir;
        const char *runtimePath = fs->appendStrings(compilerRootPath.c_str(), "/cryo/runtime/runtime.c");
        if (!runtimePath)
        {
            fprintf(stderr, "[Linker] Error: Failed to allocate memory for runtime path\n");
            CONDITION_FAILED;
        }

        // Check to see if the file exists
        if (!fs->fileExists(runtimePath))
        {
            fprintf(stderr, "[Linker] Error: Runtime file does not exist: %s\n", runtimePath);
            CONDITION_FAILED;
        }
        logMessage(LMI, "INFO", "Linker", "Runtime Path: %s", runtimePath);

        // We have verified that the file exists, now we need to convert it to LLVM IR

        // Create an LLVM context and module for the runtime
        llvm::LLVMContext *runtimeContext = new llvm::LLVMContext();
        this->runtimeModule = new llvm::Module("cryo_runtime", *runtimeContext);

        // Set up Clang to compile the C file to LLVM IR
        std::vector<const char *> args;

        // Basic compilation arguments
        args.push_back("clang");
        args.push_back("-c");
        args.push_back("-emit-llvm");
        args.push_back("-O2");       // Optimization level
        args.push_back("-fPIC");     // Position-Independent Code for shared library
        args.push_back("-x");        // Specify input language
        args.push_back("c");         // C language
        args.push_back(runtimePath); // Input file

        // Standard includes
        args.push_back("-I");
        args.push_back(fs->appendStrings(compilerRootPath.c_str(), "/include"));

        // Create temporary output file for IR
        std::string tempIRPath = std::string(this->dirInfo->buildDir) + "/runtime_temp.ll";
        args.push_back("-o");
        args.push_back(tempIRPath.c_str());

        // Create diagnostics engine for Clang
        llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> diagOpts = new clang::DiagnosticOptions();
        llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> diagID(new clang::DiagnosticIDs());
        clang::DiagnosticsEngine diags(diagID, &*diagOpts);

        // Create file system for compiler instance
        llvm::IntrusiveRefCntPtr<clang::FileManager> files(new clang::FileManager(clang::FileSystemOptions()));

        // Create compiler instance
        clang::CompilerInstance CI;
        CI.createDiagnostics();

        // Create compiler invocation
        std::shared_ptr<clang::CompilerInvocation> invocation(new clang::CompilerInvocation);
        clang::CompilerInvocation::CreateFromArgs(*invocation, args, diags);
        CI.setInvocation(invocation);

        // Create and execute the frontend action to emit LLVM IR
        clang::EmitLLVMOnlyAction action(runtimeContext);

        // Execute the action
        bool success = CI.ExecuteAction(action);
        if (!success)
        {
            fprintf(stderr, "[Linker] Error: Failed to compile runtime.c to LLVM IR\n");
            CONDITION_FAILED;
        }

        // Take ownership of the module
        std::unique_ptr<llvm::Module> module = action.takeModule();
        if (!module)
        {
            fprintf(stderr, "[Linker] Error: Failed to generate LLVM module from runtime.c\n");
            CONDITION_FAILED;
        }

        // Move the generated module to our class member
        this->runtimeModule = module.release();

        // Set up appropriate target triple and data layout
        auto targetTriple = llvm::Triple();
        this->runtimeModule->setTargetTriple(targetTriple.getTriple());

        // Initialize the target registry
        llvm::InitializeAllTargetInfos();
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmParsers();
        llvm::InitializeAllAsmPrinters();

        std::string error;
        const llvm::Target *target = llvm::TargetRegistry::lookupTarget(targetTriple.getTriple(), error);
        if (!target)
        {
            fprintf(stderr, "[Linker] Error: Failed to lookup target: %s\n", error.c_str());
            CONDITION_FAILED;
        }

        // Create target machine
        llvm::TargetOptions opt;
        auto RM = llvm::Optional<llvm::Reloc::Model>(llvm::Reloc::PIC_);
        llvm::TargetMachine *targetMachine = target->createTargetMachine(
            targetTriple.getTriple(), "generic", "", opt, RM);

        // Set data layout
        this->runtimeModule->setDataLayout(targetMachine->createDataLayout());

        // Mark runtime as initialized
        this->runtimeModuleInitialized = true;

        logMessage(LMI, "INFO", "Linker", "Successfully initialized C runtime module");

        DEBUG_BREAKPOINT;
    }

    void Linker::createStdSharedLib(const char *compilerRootPath)
    {
        __STACK_FRAME__

        logMessage(LMI, "INFO", "Linker", "Creating standard library shared objects...");

        // Ensure the runtime module is initialized
        if (!this->runtimeModuleInitialized)
        {
            fprintf(stderr, "[Linker] Error: Runtime module not initialized\n");
            CONDITION_FAILED;
            return;
        }

        // Create the bin directory if it doesn't exist
        std::string binDir = std::string(compilerRootPath) + "/cryo/bin";
        if (!fs->dirExists(binDir.c_str()))
        {
            if (!fs->createDirectory(binDir.c_str()))
            {
                fprintf(stderr, "[Linker] Error: Failed to create bin directory: %s\n", binDir.c_str());
                CONDITION_FAILED;
                return;
            }
        }

        // Output library path
        std::string libPath = binDir + "/libcryoruntime";
#ifdef _WIN32
        libPath += ".dll";
#else
        libPath += ".so";
#endif

        // Initialize LLVM target
        llvm::InitializeAllTargetInfos();
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmParsers();
        llvm::InitializeAllAsmPrinters();

        // Get the target triple
        auto targetTriple = llvm::Triple();
        this->runtimeModule->setTargetTriple(targetTriple.getTriple());

        // Look up the target
        std::string error;
        const llvm::Target *target = llvm::TargetRegistry::lookupTarget(targetTriple.getTriple(), error);
        if (!target)
        {
            fprintf(stderr, "[Linker] Error: Failed to lookup target: %s\n", error.c_str());
            CONDITION_FAILED;
            return;
        }

        // Create target machine
        llvm::TargetOptions opt;
        auto RM = llvm::Optional<llvm::Reloc::Model>(llvm::Reloc::PIC_);
        llvm::TargetMachine *targetMachine = target->createTargetMachine(
            targetTriple.getTriple(), "generic", "", opt, RM);

        // Set data layout
        this->runtimeModule->setDataLayout(targetMachine->createDataLayout());

        // Create output file
        std::error_code EC;
        llvm::raw_fd_ostream dest(libPath, EC, llvm::sys::fs::OF_None);
        if (EC)
        {
            fprintf(stderr, "[Linker] Error: Failed to open output file: %s\n", EC.message().c_str());
            CONDITION_FAILED;
            return;
        }

        // Create the pass manager for optimization
        llvm::legacy::PassManager pass;

        // Set file type for shared library
        auto fileType = llvm::CodeGenFileType::ObjectFile;

        // Add target-specific passes
        if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, fileType))
        {
            fprintf(stderr, "[Linker] Error: Target machine can't emit a file of this type\n");
            CONDITION_FAILED;
            return;
        }

        // Run passes to generate object file
        pass.run(*this->runtimeModule);
        dest.close();

// Now, we need to link the object file into a shared library
#ifdef _WIN32
        std::string linkCmd = "link.exe /DLL /OUT:" + libPath + " " + binDir + "/runtime.obj";
#else
        std::string objFile = binDir + "/runtime.o";
        // First, we need to make sure the LLVM-generated object file is saved
        std::string mvCmd = "mv " + libPath + " " + objFile;
        int mvResult = system(mvCmd.c_str());
        if (mvResult != 0)
        {
            fprintf(stderr, "[Linker] Error: Failed to move object file\n");
            CONDITION_FAILED;
            return;
        }

        std::string linkCmd = "gcc -shared -o " + libPath + " " + objFile + " -fPIC";
#endif

        // Execute the link command
        int linkResult = system(linkCmd.c_str());
        if (linkResult != 0)
        {
            fprintf(stderr, "[Linker] Error: Failed to link shared library\n");
            CONDITION_FAILED;
            return;
        }

        logMessage(LMI, "INFO", "Linker", "Successfully created standard library shared object: %s", libPath.c_str());

        DEBUG_BREAKPOINT;
    }

} // namespace Cryo
