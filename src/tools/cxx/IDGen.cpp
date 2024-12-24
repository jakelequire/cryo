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

} // extern "C"

namespace Cryo
{
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

    const char *IDGen::reverse64BitHashID(const char *hash)
    {
        uint64_t reversed = 0;
        std::sscanf(hash, "%016lX", &reversed);
        char *reversedStr = (char *)std::malloc(17); // 16 digits + null terminator
        std::sprintf(reversedStr, "%016lX", reversed);
        return reversedStr;
    }

    const char *IDGen::generate32BitHashID(const char *seed)
    {
        uint32_t hash = 0;
        size_t len = strlen(seed);
        for (size_t i = 0; i < len; ++i)
        {
            hash = (hash << 5) + hash + seed[i]; // hash * 33 + seed[i]
        }
        hash = hash & 0xFFFFFFFF; // Ensure it's 32-bit

        // Convert hash to string
        char *hashStr = (char *)std::malloc(9); // 8 digits + null terminator
        std::sprintf(hashStr, "%08X", hash);
        return hashStr;
    }

    const char *IDGen::generate16BitHashID(const char *seed)
    {
        uint16_t hash = 0;
        size_t len = strlen(seed);
        for (size_t i = 0; i < len; ++i)
        {
            hash = (hash << 5) + hash + seed[i]; // hash * 33 + seed[i]
        }
        hash = hash & 0xFFFF; // Ensure it's 16-bit

        // Convert hash to string
        char *hashStr = (char *)std::malloc(5); // 4 digits + null terminator
        std::sprintf(hashStr, "%04X", hash);
        return hashStr;
    }

} // namespace Cryo
