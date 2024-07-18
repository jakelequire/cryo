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
#include "cli/version.h"


char* cryo_logo =
"                                                                     \n"
"                    ==                                               \n"
"                   ==++                                              \n"
"                  == ===                                             \n"
"        +===     =+======                                            \n"
"         = ===== == ==+=+==                                          \n"
"         =+= =============+==                                        \n"
"          ===+=+====++========+     :::::::::  :::   :::  ::::::::   \n"
"         ==+==== ==    ==+====+=+   :+:    :+: :+:   :+: :+:    :+:  \n"
"      === ==== ==        ========+  +:+    +:+  +:+ +:+  +:+    +:+  \n"
"   +=== =======                     +#++:++#:    +#++:   +#+    +:+  \n"
"      === ==+=+==        ========+  +#+    +#+    +#+    +#+    +#+  \n"
"         +=+====+==    ==+====+=+   #+#    #+#    #+#    #+#    #+#  \n"
"          ===+=+==+==+========+     ###    ###    ###     ########   \n"
"         === =+==+======+====                                        \n"
"         = ===== == ==+=+==                                          \n"
"        +====+   = ====+=                                            \n"
"                  == ===                                             \n"
"                   ==++                                              \n"
"                    ==                                               \n"
"                                                                     \n";



// <getVersion>
Version getVersion() {
    Version version;
    version.major = 0;
    version.minor = 1;
    version.patch = 0;

    return version;
}
// </getVersion>


// <executeVersionCmd>
void executeVersionCmd() {
    Version version = getVersion();

    printf("\n\n");
    printf("%s\n", cryo_logo);
    printf("Cryo Programming Language v%d.%d.%d\n", version.major, version.minor, version.patch);
    printf("Cryo CLI Tools v%d.%d.%d\n", version.major, version.minor, version.patch);
    printf("\n\n");

    exit(0);
}
// </executeVersionCmd>
