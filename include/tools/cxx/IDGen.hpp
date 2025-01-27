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
#ifndef IDGEN_H
#define IDGEN_H

// C API
#ifdef __cplusplus
extern "C"
{
#endif

    // Opaque pointer type for C
    typedef struct CryoIDGen_t *CryoIDGen;

    // C API functions
    CryoIDGen CryoIDGen_Create(void);
    void CryoIDGen_Destroy(CryoIDGen idGen);
    const char *CryoIDGen_Generate64BitHashID(CryoIDGen idGen, const char *seed);
#define Generate64BitHashID(seed) Cryo_Generate64BitHashID(seed)

#ifdef __cplusplus
} // C API

#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <functional>
#include <string.h>

namespace Cryo
{

    class IDGen
    {
    public:
        /// @brief Generate a 64-bit hash ID from a seed string.
        /// This is a *deterministic* hash ID generator.
        static const char *generate64BitHashID(const char *seed);

        /// @brief Generate a 32-bit hash ID from a seed string.
        /// This is a *deterministic* hash ID generator.
        const char *generate32BitHashID(const char *seed);

        /// @brief Generate a 16-bit hash ID from a seed string.
        /// This is a *deterministic* hash ID generator.
        const char *generate16BitHashID(const char *seed);

    private:
        std::string id;
    };

} // namespace Cryo

// Implementation of the new function
inline const char *Cryo_Generate64BitHashID(const char *seed)
{
    return Cryo::IDGen::generate64BitHashID(seed);
}

#endif // __cplusplus
#endif // IDGEN_H
