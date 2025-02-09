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

// This file containes the C Implementation for the Cryo Linker class.

extern "C"
{
    CryoLinker *CryoLinker_Create(const char *buildDir)
    {
        __STACK_FRAME__
        try
        {
            auto linker = new Cryo::Linker(buildDir);
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
        __STACK_FRAME__
        if (linker)
        {
            logMessage(LMI, "INFO", "CryoLinker", "Destroying linker");
            delete reinterpret_cast<Cryo::Linker *>(linker);
        }
    }

    void CryoLinker_InitCRuntime(CryoLinker *linker)
    {
        __STACK_FRAME__
        if (linker)
        {
            logMessage(LMI, "INFO", "CryoLinker", "Initializing C Runtime");
            reinterpret_cast<Cryo::Linker *>(linker)->initCRuntime();
        }
    }

    void CryoLinker_LinkAll(CryoLinker *linker)
    {
        __STACK_FRAME__
        if (linker)
        {
            logMessage(LMI, "INFO", "CryoLinker", "Linking all modules");
            reinterpret_cast<Cryo::Linker *>(linker)->linkAll();
        }
    }

} // C API
