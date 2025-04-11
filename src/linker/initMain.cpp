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
#include "diagnostics/diagnostics.h"

namespace Cryo
{

    /// @brief This function will initialize the creation of the Cryo Runtime Module.
    /// This module is compiled along with a C runtime module to create the final
    /// runtime module that will be used in the Cryo Compiler before the main Cryo module.
    ///
    /// @return The Cryo Runtime Module
    ///
    /// @note this function is called from the C++ CodeGen API before code generation.
    llvm::Module *Linker::initMainModule(void)
    {
        __STACK_FRAME__
        std::cout << "Initializing Main Module before CodeGen..." << std::endl;

        // This is the new process for initializing the main module.
    }

} // namespace Cryo
