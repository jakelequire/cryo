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

namespace Cryo
{
    // ================================================================ //
    //             Linker Module Handler Debug Methods                  //
    // ================================================================ //

    void LinkerModule::logDirectoryInfo(DirectoryInfo *dirInfo)
    {
        std::cout << "Root Directory: " << dirInfo->rootDir << std::endl;
        std::cout << "Build Directory: " << dirInfo->buildDir << std::endl;
        std::cout << "Output Directory: " << dirInfo->outDir << std::endl;
        std::cout << "Dependency Directory: " << dirInfo->depDir << std::endl;
    }

    // ================================================================ //
    //                    Linker Debug Methods                          //
    // ================================================================ //

} // namespace Cryo
