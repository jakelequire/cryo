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
///
/// attributes.h | #include "/tools/utils/attributes.h"
///
/// This file contains utility attributes for C. These are used to help with
/// compiler optimizations and other things. They are not required, but they
/// can be helpful in some cases.
///
#ifndef ATTRIBUTES_H
#define ATTRIBUTES_H

#define __C_CONSTRUCTOR__ \
    __attribute__((constructor))

#define __C_DESTRUCTOR__ \
    __attribute__((destructor))

#define __C_UNUSED__ \
    __attribute__((unused))

#define __C_NORETURN__ \
    __attribute__((noreturn))

#endif // ATTRIBUTES_H
