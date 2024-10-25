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
#include "support_lib.h"

// ----------------------------

template <typename T>
void testGeneric(T value)
{
    // Test what type of value is passed in
    if (std::is_same<T, int>::value)
    {
        std::cout << "This is an int" << std::endl;
    }
    else if (std::is_same<T, std::string>::value)
    {
        std::cout << "This is a string" << std::endl;
    }
    else if (std::is_same<T, const char *>::value)
    {
        std::cout << "This is a const char*" << std::endl;
    }
    else
    {
        std::cout << "This is something else" << std::endl;
        std::cout << "Type Received: " << typeid(value).name() << std::endl;
    }
}

__EXTERN_C__ void Generic(void *value)
{
    testGeneric(value);
}

__EXTERN_C__ void derefVoidPtr(void *value)
{
    int *val = static_cast<int *>(value);
    std::cout << *val << std::endl;
}

#define PRINT_TY_NAME(x) std::cout << #x << " - " << typeid(x).name() << '\n'

__EXTERN_C__ void printInt(int value)
{
    printf("%d\n", value);
}

__EXTERN_C__ void printIntPtr(int *value)
{
    printf("%d\n", *value);
}

__EXTERN_C__ void printStr(char *value)
{
    printf("%s\n", value);
}

__EXTERN_C__ void printTypeIDs()
{
    std::cout << "\n---------------------------" << std::endl;
    std::cout << "Type IDs:" << std::endl;
    std::cout << "---------------------------" << std::endl;
    PRINT_TY_NAME(char);
    PRINT_TY_NAME(signed char);
    PRINT_TY_NAME(unsigned char);
    PRINT_TY_NAME(short);
    PRINT_TY_NAME(unsigned short);
    PRINT_TY_NAME(int);
    PRINT_TY_NAME(unsigned int);
    PRINT_TY_NAME(long);
    PRINT_TY_NAME(unsigned long);
    PRINT_TY_NAME(float);
    PRINT_TY_NAME(double);
    PRINT_TY_NAME(long double);
    PRINT_TY_NAME(char *);
    PRINT_TY_NAME(const char *);
    PRINT_TY_NAME(void);
    PRINT_TY_NAME(void *);
    PRINT_TY_NAME(void **);
    PRINT_TY_NAME(int *);
    PRINT_TY_NAME(const int *);
    PRINT_TY_NAME(std::string);
    PRINT_TY_NAME(std::vector<int>);
    PRINT_TY_NAME(std::vector<std::string>);
    std::cout << "---------------------------" << std::endl;
}
