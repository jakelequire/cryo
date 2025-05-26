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
#include "linker/linker.hpp"
#include "diagnostics/diagnostics.h"

namespace Cryo
{

    void Linker::logDirectoryInfo(DirectoryInfo *dirInfo)
    {
        __STACK_FRAME__
        std::cout << "Root Directory: " << dirInfo->rootDir << std::endl;
        std::cout << "Build Directory: " << dirInfo->buildDir << std::endl;
        std::cout << "Output Directory: " << dirInfo->outDir << std::endl;
        std::cout << "Dependency Directory: " << dirInfo->depDir << std::endl;
        std::cout << "Runtime Directory: " << dirInfo->runtimeDir << std::endl;
    }

    DirectoryInfo *Linker::createDirectoryInfo(std::string rootDir)
    {
        __STACK_FRAME__
        DirectoryInfo *dirInfo = new DirectoryInfo();
        dirInfo->rootDir = rootDir;
        dirInfo->buildDir = rootDir + "/build";
        dirInfo->outDir = rootDir + "/build/out";
        dirInfo->depDir = rootDir + "/build/out/deps";
        dirInfo->runtimeDir = rootDir + "/build/out/runtime";

        return dirInfo;
    }

} // namespace Cryo
