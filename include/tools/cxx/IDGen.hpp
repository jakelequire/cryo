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
    const char *CryoIDGen_Generate32BitID(CryoIDGen idGen);

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

namespace Cryo
{

    class IDGen
    {
    public:
        static std::string generate32BitID(void);

    private:
        std::string id;
    };

} // namespace Cryo

#endif // __cplusplus
#endif // IDGEN_H
