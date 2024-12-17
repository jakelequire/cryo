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
#include "tools/cxx/IDGen.hpp"

extern "C"
{

    CryoIDGen CryoIDGen_Create(void)
    {
        try
        {
            auto idGen = new Cryo::IDGen();
            return reinterpret_cast<CryoIDGen>(idGen);
        }
        catch (...)
        {
            return nullptr;
        }
    }

    void CryoIDGen_Destroy(CryoIDGen idGen)
    {
        if (idGen)
        {
            delete reinterpret_cast<Cryo::IDGen *>(idGen);
        }
    }

    const char *CryoIDGen_Generate32BitID(CryoIDGen idGen)
    {
        if (idGen)
        {
            std::string id = reinterpret_cast<Cryo::IDGen *>(idGen)->generate32BitID();
            return id.c_str();
        }
        return nullptr;
    }

    int CryoIDGen_Generate32BitIntID(CryoIDGen idGen)
    {
        if (idGen)
        {
            return reinterpret_cast<Cryo::IDGen *>(idGen)->generate32BitIntID();
        }
        return 0;
    }

} // extern "C"

namespace Cryo
{

    std::string IDGen::generate32BitID(void)
    {
        std::string id = "";
        for (int i = 0; i < 32; i++)
        {
            id += std::to_string(rand() % 2);
        }
        return id;
    }

    int IDGen::generate32BitIntID(void)
    {
        int id = 0;
        for (int i = 0; i < 32; i++)
        {
            int multiplier = 1 << i;
            id += rand() % 2 * multiplier;
        }
        return id;
    }

    const char *IDGen::generate64BitHashID(const char *seed)
    {
        uint64_t hash = 0;
        size_t len = strlen(seed);
        for (size_t i = 0; i < len; ++i)
        {
            hash = (hash << 5) + hash + seed[i]; // hash * 33 + seed[i]
        }
        hash = hash & 0xFFFFFFFFFFFFFFFF; // Ensure it's 64-bit

        // Convert hash to string
        char *hashStr = (char *)std::malloc(17); // 16 digits + null terminator
        std::sprintf(hashStr, "%016lX", hash);
        return hashStr;
    }

} // namespace Cryo
