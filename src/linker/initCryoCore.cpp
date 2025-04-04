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
    }

    llvm::Module *Linker::_initCRuntime_(void)
    {
    }

} // namespace Cryo
