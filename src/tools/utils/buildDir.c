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
#include "tools/utils/buildDir.h"
#include "diagnostics/diagnostics.h"

/*
{ROOT_DIR}
    - build
        - out
            - obj
            - runtime
            - deps
        {exeName}
*/

#define BUILD_DIR "build"
#define OUT_DIR "out"
#define RUNTIME_DIR "runtime"
#define DEPS_DIR "deps"
#define OBJ_DIR "obj"

void init_build_dir(const char *rootDir)
{
    __STACK_FRAME__
    printf("Hello from @init_build_dir\n");
    printf("Root Directory: %s\n", rootDir);

    // Create the build directory
    String *buildDir = Str(rootDir);
    buildDir->append(buildDir, "/");
    buildDir->append(buildDir, BUILD_DIR);
    printf("Build Directory: %s\n", buildDir->c_str(buildDir));

    // Create the out directory
    String *outDir = Str(buildDir->c_str(buildDir));
    outDir->append(outDir, "/");
    outDir->append(outDir, OUT_DIR);
    printf("Out Directory: %s\n", outDir->c_str(outDir));

    // Create the obj directory
    String *objDir = Str(buildDir->c_str(outDir));
    objDir->append(objDir, "/");
    objDir->append(objDir, OBJ_DIR);
    printf("Obj Directory: %s\n", objDir->c_str(objDir));

    // Create the runtime directory
    String *runtimeDir = Str(outDir->c_str(outDir));
    runtimeDir->append(runtimeDir, "/");
    runtimeDir->append(runtimeDir, RUNTIME_DIR);
    printf("Runtime Directory: %s\n", runtimeDir->c_str(runtimeDir));

    // Create the deps directory
    String *depsDir = Str(outDir->c_str(outDir));
    depsDir->append(depsDir, "/");
    depsDir->append(depsDir, DEPS_DIR);
    printf("Deps Directory: %s\n", depsDir->c_str(depsDir));

    // Create the directories
    fs->createDirectory(buildDir->c_str(buildDir));
    fs->createDirectory(outDir->c_str(outDir));
    fs->createDirectory(objDir->c_str(objDir));
    fs->createDirectory(runtimeDir->c_str(runtimeDir));
    fs->createDirectory(depsDir->c_str(depsDir));

    // Cleanup
    buildDir->destroy(buildDir);
    outDir->destroy(outDir);
    objDir->destroy(objDir);
    runtimeDir->destroy(runtimeDir);
    depsDir->destroy(depsDir);
}
