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

#include "cryo/cryolib.h"

void printInt(int value) {
    printf("Attempting to execute printInt function: \n");
    printf("%d\n", value);
}

void printFloat(float value) {
    printf("Attempting to execute printFloat function: \n");
    printf("%f\n", value);
}

void printStr(char* value) {
    printf("Attempting to execute printStr function: \n");
    printf("%s\n", value);
}

