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
#include "linker/linkerv2.hpp"

// This file containes the C Implementation for the Cryo Linker class.

namespace Cryo
{
    CryoLinker *CryoLinker_Create(void)
    {
        try
        {
            auto linker = new CryoLinker();
            return reinterpret_cast<CryoLinker *>(linker);
        }
        catch (...)
        {
            logMessage(LMI, "ERROR", "CryoLinker", "Failed to create linker");
            return nullptr;
        }
    }

    void CryoLinker_Destroy(CryoLinker *linker)
    {
        if (linker)
        {
            logMessage(LMI, "INFO", "CryoLinker", "Destroying linker");
            delete reinterpret_cast<CryoLinker *>(linker);
        }
    }

    void CryoLinker_SetRootDir(CryoLinker *linker, const char *rootDir)
    {
        if (linker)
        {
            reinterpret_cast<CryoLinker *>(linker)->setRootDir(rootDir);
        }
    }

} // namespace Cryo
