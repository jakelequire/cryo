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
#include "tools/utils/env.h"
#include "tools/logger/logger_config.h"

#define PATH_MAX 4096

const char *ENV_VECTOR[] = {
    "CRYO_ROOT",
    "CRYO_COMPILER"};

/*
build path: {CRYO_ROOT}/build/
runtime files: {CRYO_ROOT}/cryo/
compiler deps: {CRYO_ROOT}/build/out/deps/
cryo compiler: {CRYO_ROOT}/bin/compiler
*/

char *get_cryo_root(const char *path)
{
    const char *cryo_dir = "/cryo/";
    char *found = strstr((char *)path, cryo_dir);

    if (!found)
    {
        return NULL; // "cryo" not found in path
    }

    // Calculate length up to and including "/cryo/"
    size_t length = (found - path) + strlen(cryo_dir); // Changed to use full cryo_dir length

    // Allocate space for the new string
    char *result = (char *)malloc(length + 1); // +1 for null terminator
    if (!result)
    {
        return NULL;
    }

    // Copy the path up to and including "/cryo/"
    strncpy(result, path, length);
    result[length] = '\0'; // Place null terminator right after the slash

    return result;
}

char *getParentDir(const char *path)
{
    char *parent = strdup(path);
    if (!parent)
    {
        return NULL;
    }

    char *dir = dirname(parent);
    char *resultStr = strdup(dir);
    char *result = get_cryo_root(resultStr);
    free(parent);
    free(resultStr);
    return result;
}

char *getCompilerRootPath(const char *argv0)
{
    char resolved_path[PATH_MAX];
    if (!realpath(argv0, resolved_path))
    {
        return NULL;
    }

    return getParentDir(resolved_path);
}

// Helper function to check if directory exists
static int dir_exists(const char *path)
{
    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

// Helper to create path string
static char *create_path(const char *base, const char *suffix)
{
    char *result = (char *)malloc(PATH_MAX);
    if (!result)
        return NULL;

    // Check if base ends with '/' and suffix starts with '/'
    if (base[strlen(base) - 1] == '/' && suffix[0] == '/')
        snprintf(result, PATH_MAX, "%s%s", base, suffix + 1); // Skip the first '/' in suffix
    else if (base[strlen(base) - 1] == '/' || suffix[0] == '/')
        snprintf(result, PATH_MAX, "%s%s", base, suffix); // Use paths as-is when one has '/'
    else
        snprintf(result, PATH_MAX, "%s/%s", base, suffix); // Add '/' when neither has it

    return result;
}

int initEnvVars(const char *cryo_dir)
{
    if (!cryo_dir || !dir_exists(cryo_dir))
    {
        fprintf(stderr, "Invalid CRYO_ROOT directory: %s\n", cryo_dir);
        return -1;
    }

    // Set CRYO_ROOT
    if (ensureEnvVar("CRYO_ROOT", cryo_dir) != 0)
    {
        return -1;
    }

    // Create and ensure directory paths exist
    char *build_path = create_path(cryo_dir, "build");
    char *runtime_path = create_path(cryo_dir, "cryo");
    char *deps_path = create_path(cryo_dir, "build/out/deps");
    char *compiler_path = create_path(cryo_dir, "bin/compiler");

    if (!build_path || !runtime_path || !deps_path || !compiler_path)
    {
        fprintf(stderr, "Failed to allocate path strings\n");
        free(build_path);
        free(runtime_path);
        free(deps_path);
        free(compiler_path);
        return -1;
    }

    // Create directories if they don't exist
    mkdir(build_path, 0755);
    mkdir(runtime_path, 0755);
    mkdir(deps_path, 0755);

    // Ensure parent directory for compiler exists
    char *bin_dir = create_path(cryo_dir, "bin");
    if (bin_dir)
    {
        mkdir(bin_dir, 0755);
        free(bin_dir);
    }

    // Set CRYO_COMPILER path
    if (ensureEnvVar("CRYO_COMPILER", compiler_path) != 0)
    {
        fprintf(stderr, "Failed to set CRYO_COMPILER\n");
        free(build_path);
        free(runtime_path);
        free(deps_path);
        free(compiler_path);
        return -1;
    }

    // Additional environment variables can be set here

    // Clean up
    free(build_path);
    free(runtime_path);
    free(deps_path);
    free(compiler_path);

    return 0;
}

int ensureEnvVar(const char *env_name, const char *env_value)
{
    if (!env_name || !env_value)
    {
        fprintf(stderr, "Invalid environment variable name or value\n");
        return -1;
    }

    char *value = getenv(env_name);

    // If env var doesn't exist or is empty, set it
    if (value == NULL || strlen(value) == 0)
    {
        if (setenv(env_name, env_value, 1) != 0)
        {
            perror("Failed to set environment variable");
            return -1;
        }

        DEBUG_PRINT_FILTER({
            printf("%s%sSet %s=%s%s\n", LIGHT_RED, BOLD, env_name, env_value, COLOR_RESET);
            return 0;
        });
        // without color formatting
        printf("Set %s=%s\n", env_name, env_value);
    }
    else
    {
        DEBUG_PRINT_FILTER({
            printf("%s%s%s is already set to: %s%s\n", LIGHT_GREEN, BOLD, env_name, value, COLOR_RESET);
            return 0;
        });
        // without color formatting
        printf("%s is already set to: %s\n", env_name, value);
    }

    return 0;
}

// Add this to verify environment is properly set up
int verifyEnvSetup(void)
{
    for (size_t i = 0; i < sizeof(ENV_VECTOR) / sizeof(ENV_VECTOR[0]); i++)
    {
        char *value = getenv(ENV_VECTOR[i]);
        if (!value || strlen(value) == 0)
        {
            fprintf(stderr, "Required environment variable %s is not set\n", ENV_VECTOR[i]);
            return -1;
        }

        // For path variables, verify the paths exist
        if (strstr(ENV_VECTOR[i], "PATH") ||
            strcmp(ENV_VECTOR[i], "CRYO_ROOT") == 0 ||
            strcmp(ENV_VECTOR[i], "CRYO_COMPILER") == 0)
        {
            if (!dir_exists(value))
            {
                fprintf(stderr, "Directory for %s does not exist: %s\n", ENV_VECTOR[i], value);
                return -1;
            }
        }
    }
    return 0;
}

// ----------------------------------------------------------------

char *getCryoRootPath(void)
{
    char *cryoRoot = getenv("CRYO_ROOT");
    if (!cryoRoot)
    {
        fprintf(stderr, "Error: CRYO_ROOT environment variable not set\n");
        return NULL;
    }
    return strdup(cryoRoot);
}

char *getCryoCompilerPath(void)
{
    char *compilerPath = getenv("CRYO_COMPILER");
    if (!compilerPath)
    {
        fprintf(stderr, "Error: CRYO_COMPILER environment variable not set\n");
        return NULL;
    }
    return strdup(compilerPath);
}
