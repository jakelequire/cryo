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
#ifndef C_WRAPPERS_H
#define C_WRAPPERS_H
#include <stdio.h>
#include <stdlib.h>

/// @brief A macro to define a new method
/// @param type The return type of the method
/// @param name The name of the method
/// @param ... The arguments of the method
#define _NEW_METHOD(type, name, ...) \
    type (*name)(__VA_ARGS__)

#endif // C_WRAPPERS_H