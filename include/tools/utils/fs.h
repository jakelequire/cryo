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
#ifndef FS_H
#define FS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <dirent.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdarg.h>

#include "tools/macros/debugMacros.h"
#include "tools/utils/c_logger.h"
#include "tools/utils/cWrappers.h"

#define INIT_FS() \
    initGlobalFS();

typedef struct CryoSrcLocations
{
    const char *rootDir;
} CryoSrcLocations;

#ifdef __cplusplus
extern "C"
{
#endif

    char *readFile(const char *path);
    bool fileExists(const char *path);

#ifdef __cplusplus
}
#endif

char *readFile(const char *path);
bool fileExists(const char *path);
int dirExists(const char *path);
int createDir(const char *path);
int createDirectory(const char *path);
void removeFile(const char *filePath);
const char *getSTDFilePath(const char *subModule);
const char *trimFilePath(const char *filePath);
char *getFileNameFromPathNoExt(const char *filePath);
char *getFileNameFromPath(const char *filePath);

const char *getCurRootDir(void);
const char *getCryoSrcLocation(void);
char *getCRuntimePath(void);

const char *appendStrings(const char *str1, const char *str2);
const char *appendExtensionToFileName(const char *fileName, const char *extension);
const char *appendPathToFileName(const char *path, const char *fileName, bool endingSlash);
const char *removeFileFromPath(const char *path);
const char *changeFileExtension(const char *fileName, const char *newExtension);

char *getPathFromCryoPath(void);
char *getPathFromEnvVar(void);
char *getCompilerBinPath(void);
char *getCompilerRootPath(void);

void createNewEmptyFile(const char *fileName, const char *ext, const char *path);
int createNewEmptyFileWpath(const char *fileWithPath);
const char *cleanFilePath(char *filePath);
const char *getFileName(const char *filePath);
const char *getFileExt(const char *filePath);

typedef struct jFS
{
    // `readFile` reads the contents of a file into a buffer.
    _NEW_METHOD(char *, readFile, const char *path);
    // `fileExists` checks if a file exists at the given path.
    _NEW_METHOD(bool, fileExists, const char *path);
    // `dirExists` checks if a directory exists at the given path.
    _NEW_METHOD(int, dirExists, const char *path);
    // `createDir` creates a directory at the given path.
    _NEW_METHOD(int, createDir, const char *path);
    // `createDirectory` creates a directory at the given path.
    _NEW_METHOD(int, createDirectory, const char *path);
    // `removeFile` removes a file at the given path.
    _NEW_METHOD(void, removeFile, const char *filePath);
    // `getSTDFilePath` gets the path to a standard file.
    _NEW_METHOD(const char *, getSTDFilePath, const char *subModule);
    // `trimFilePath` trims the file path.
    _NEW_METHOD(const char *, trimFilePath, const char *filePath);
    // `getFileNameFromPathNoExt` extracts the filename from a file path without the extension.
    _NEW_METHOD(char *, getFileNameFromPathNoExt, const char *filePath);
    // `getFileNameFromPath` extracts the filename from a file path.
    _NEW_METHOD(char *, getFileNameFromPath, const char *filePath);
    // `getCurRootDir` gets the current root directory.
    _NEW_METHOD(const char *, getCurRootDir, void);
    // `getCryoSrcLocation` gets the Cryo source location.
    _NEW_METHOD(const char *, getCryoSrcLocation, void);
    // `getCRuntimePath` gets the C runtime path.
    _NEW_METHOD(char *, getCRuntimePath, void);
    // `appendStrings` appends two strings together.
    _NEW_METHOD(const char *, appendStrings, const char *str1, const char *str2);
    // `appendExtensionToFileName` appends an extension to a file name.
    _NEW_METHOD(const char *, appendExtensionToFileName, const char *fileName, const char *extension);
    // `appendPathToFileName` appends a path to a file name.
    _NEW_METHOD(const char *, appendPathToFileName, const char *path, const char *fileName, bool endingSlash);
    // `removeFileFromPath` removes a file from a path.
    _NEW_METHOD(const char *, removeFileFromPath, const char *path);
    // `changeFileExtension` changes the file extension.
    _NEW_METHOD(const char *, changeFileExtension, const char *fileName, const char *newExtension);
    // `getPathFromCryoPath` gets the path from the Cryo path.
    _NEW_METHOD(char *, getPathFromCryoPath, void);
    // `getPathFromEnvVar` gets the path from an environment variable.
    _NEW_METHOD(char *, getPathFromEnvVar, void);
    // `getCompilerBinPath` gets the compiler binary path.
    _NEW_METHOD(char *, getCompilerBinPath, void);
    // `getCompilerRootPath` gets the compiler root path.
    _NEW_METHOD(char *, getCompilerRootPath, void);
    // `createNewEmptyFile` creates a new empty file.
    _NEW_METHOD(void, createNewEmptyFile, const char *fileName, const char *ext, const char *path);
    // `createNewEmptyFileWpath` creates a new empty file with a path.
    _NEW_METHOD(int, createNewEmptyFileWpath, const char *fileWithPath);
    // `cleanFilePath` cleans the file path.
    _NEW_METHOD(const char *, cleanFilePath, char *filePath);
    // `getFileName` gets the file name.
    _NEW_METHOD(const char *, getFileName, const char *filePath);
    // `getFileExt` gets the file extension.
    _NEW_METHOD(const char *, getFileExt, const char *filePath);
} jFS;

jFS *initFS(void);
void initGlobalFS(void);

extern jFS *fs;

#define CUR_ROOT_DIR_ABS
#define CUR_ROOT_DIR_REL
#endif
