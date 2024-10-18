#*******************************************************************************
#  Copyright 2024 Jacob LeQuire                                                *
#  SPDX-License-Identifier: Apache-2.0                                         *
#    Licensed under the Apache License, Version 2.0 (the "License");           *
#    you may not use this file except in compliance with the License.          *
#    You may obtain a copy of the License at                                   *
#                                                                              *
#    http://www.apache.org/licenses/LICENSE-2.0                                *
#                                                                              *
#    Unless required by applicable law or agreed to in writing, software       *
#    distributed under the License is distributed on an "AS IS" BASIS,         *
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
#    See the License for the specific language governing permissions and       *
#    limitations under the License.                                            *
#                                                                              *
#*******************************************************************************/

# --------------------------------------------- #
# `-O0`    - No optimization					#
# `-O1`    - Basic optimization					#
# `-O2`    - Further optimization				#
# `-O3`    - Maximum optimization				#
# `-Og`    - Optimize debugging experience		#
# `-Os`    - Optimize for size					#
# `-Ofast` - Optimize for speed					#
# `-Oz`    - Optimize for size					#
# --------------------------------------------- #
OPTIMIZATION = -O1
DEBUG_FLAGS =  -v -D_CRT_SECURE_NO_WARNINGS 
# C23
C_STANDARD = -std=c23
CXX_STANDARD = -std=c++23

# OS-specific settings for compilers
ifeq ($(OS), Windows_NT)
# Windows settings
	C_COMPILER = C:/msys64/mingw64/bin/gcc
	CXX_COMPILER = C:/msys64/mingw64/bin/g++
else
# Linux settings
	C_COMPILER = clang
	CXX_COMPILER = clang++
endif

# >>=======--------------------------------------------------=======<<
# >>=======       OS-specific settings for compilers         =======<<
# >>=======--------------------------------------------------=======<<

# OS-specific settings
ifeq ($(OS), Windows_NT)
    # Windows settings
    CC = $(C_COMPILER) $(C_STANDARD) $(DEBUG_FLAGS) $(OPTIMIZATION)
    CXX = $(CXX_COMPILER) $(CXX_STANDARD) $(DEBUG_FLAGS) $(OPTIMIZATION)
    CFLAGS = -I"C:/msys64/mingw64/include" -I./src/include -I./src/include/runtime -I./src/include/cli \
			-I./src/include/compiler -I./src/include/utils -I./src/include/tests
    CXXFLAGS = -I"C:/msys64/mingw64/include" -I./src/include -I./src/include/runtime -I./src/include/cli \
			-I./src/include/compiler -I./src/include/utils -I./src/include/tests
    LDFLAGS = -L"C:/msys64/mingw64/lib" $(LLVM_LIBS) $(STDLIBS) -v
    LLVM_LIBS := -lLLVM 
    STDLIBS := -lmingw32 -lmingwex -lmsvcrt -lucrt -lpthread -lws2_32 -ladvapi32 -lshell32 -luser32 -lkernel32 -Wl,-subsystem,console
    MKDIR = if not exist
    RMDIR = rmdir /S /Q
    DEL = del /Q
    BIN_SUFFIX = .exe
else
    # Linux settings
    CC = $(C_COMPILER) $(DEBUG_FLAGS) $(OPTIMIZATION)
    CXX = $(CXX_COMPILER) $(DEBUG_FLAGS) $(OPTIMIZATION)
    CFLAGS = -I./src/include -I./src/include/runtime -I./src/include/cli -I./src/include/compiler \
			-I./src/include/utils -I./src/include/tests
    CXXFLAGS = -I./src/include -I./src/include/runtime -I./src/include/cli -I./src/include/compiler \
			-I./src/include/utils -I./src/include/tests $(LLVM_CXXFLAGS) -fexceptions
    LLVM_CONFIG = llvm-config
    LLVM_CFLAGS = $(shell $(LLVM_CONFIG) --cflags)
	LLVM_CXXFLAGS = $(shell $(LLVM_CONFIG) --cxxflags)
    LLVM_LDFLAGS = $(shell $(LLVM_CONFIG) --ldflags) $(shell $(LLVM_CONFIG) --libs) $(shell $(LLVM_CONFIG) --system-libs)
    LDFLAGS = $(LLVM_LDFLAGS) -lpthread -v
	STD_LIBS = -lstdc++ -lm -lc -lgcc -lgcc_eh -lstdc++fs
    MKDIR = mkdir -p
    RMDIR = rm -rf
    DEL = rm -f
    BIN_SUFFIX =
endif

# >>=======--------------------------------------------------=======<<
# >>=======                  Define Paths                    =======<<
# >>=======--------------------------------------------------=======<<

# ---------------------------------------------
# Binary directory
BIN_DIR =       ./bin/
OBJ_DIR =       $(BIN_DIR).o/
DEBUG_BIN_DIR = $(BIN_DIR)debug/

# ---------------------------------------------
# Source directory
SRC_DIR =   ./src/
MAIN_FILE = $(SRC_DIR)main.c

# ---------------------------------------------
# Frontend directory
FRONTEND_DIR =  $(SRC_DIR)frontend/
AST_DIR =       $(FRONTEND_DIR)AST/
LEXER_DIR =     $(FRONTEND_DIR)lexer/
PARSER_DIR =    $(FRONTEND_DIR)parser/
SYMTABLE_DIR =  $(FRONTEND_DIR)symtable/
TOKENS_DIR =    $(FRONTEND_DIR)tokens/
SEMANTICS_DIR = $(FRONTEND_DIR)semantics/

# ---------------------------------------------
# Codegen directory
CODEGEN_DIR =        $(SRC_DIR)codegen/
CXX_GENERATION_DIR = $(CODEGEN_DIR)generation/
CXX_TYPES_DIR =      $(CODEGEN_DIR)types/
CXX_SEMANTICS_DIR =  $(CODEGEN_DIR)semantics/

# ---------------------------------------------
# Settings directory
SETTINGS_DIR = $(SRC_DIR)settings/

# ---------------------------------------------
# CLI directory
CLI_DIR =       $(SRC_DIR)cli/
COMMANDS_DIR =  $(CLI_DIR)commands/

# ---------------------------------------------
# Common directory
COMMON_DIR = $(SRC_DIR)common/

# ---------------------------------------------
# Diagnostic directory
DIAGNOSTIC_DIR = $(SRC_DIR)diagnostic/

# ---------------------------------------------
# Linker directory
LINKER_DIR = $(SRC_DIR)linker/

# ---------------------------------------------
# Tools directory
TOOLS_DIR = $(SRC_DIR)tools/

# >>=======--------------------------------------------------=======<<
# >>=======                  Source Files                    =======<<
# >>=======--------------------------------------------------=======<<

# ---------------------------------------------
# Frontend source files
FRONTEND_SRC =  $(MAIN_FILE) $(LEXER_DIR)lexer.c $(PARSER_DIR)parser.c $(AST_DIR)ast.c \
                $(SEMANTICS_DIR)semantics.c $(SYMTABLE_DIR)symtable.c $(TOKENS_DIR)tokens.c

# ---------------------------------------------
# Codegen source files
CODEGEN_SRC =   $(CODEGEN_DIR)codegen.c $(CXX_GENERATION_DIR)generation.c $(CXX_TYPES_DIR)types.c \
                $(CXX_SEMANTICS_DIR)semantics.c

# ---------------------------------------------
# Settings source files
SETTINGS_SRC = $(SETTINGS_DIR)settings.c

# ---------------------------------------------
# CLI source files
CLI_SRC =   $(CLI_DIR)cli.c

# ---------------------------------------------
# Common source files
COMMON_SRC = $(COMMON_DIR)common.c

# ---------------------------------------------
# Diagnostic source files
DIAGNOSTIC_SRC = $(DIAGNOSTIC_DIR)diagnostic.c

# ---------------------------------------------
# Linker source files
LINKER_SRC = $(LINKER_DIR)linker.c

# ---------------------------------------------
# Tools source files
TOOLS_SRC = $(TOOLS_DIR)tools.c

# >>=======--------------------------------------------------=======<<
# >>=======                  Object Files                    =======<<
# >>=======--------------------------------------------------=======<<

# ---------------------------------------------
# Frontend object files
FRONTEND_OBJ =  $(OBJ_DIR)main.o $(OBJ_DIR)lexer.o $(OBJ_DIR)parser.o       \
                $(OBJ_DIR)ast.o $(OBJ_DIR)semantics.o $(OBJ_DIR)symtable.o  \
                $(OBJ_DIR)tokens.o

# ---------------------------------------------
# Codegen object files
CODEGEN_OBJ =   $(OBJ_DIR)codegen.o $(OBJ_DIR)generation.o $(OBJ_DIR)types.o \
                $(OBJ_DIR)semantics.o

# ---------------------------------------------
# Settings object files
SETTINGS_OBJ = $(OBJ_DIR)settings.o

# ---------------------------------------------
# CLI object files
CLI_OBJ =   $(OBJ_DIR)cli.o

# ---------------------------------------------
# Common object files
COMMON_OBJ = $(OBJ_DIR)common.o

# ---------------------------------------------
# Diagnostic object files
DIAGNOSTIC_OBJ = $(OBJ_DIR)diagnostic.o

# ---------------------------------------------
# Linker object files
LINKER_OBJ = $(OBJ_DIR)linker.o

# ---------------------------------------------
# Tools object files
TOOLS_OBJ = $(OBJ_DIR)tools.o

# >>=======--------------------------------------------------=======<<
# >>=======                 Compilation Rules                =======<<
# >>=======--------------------------------------------------=======<<


# >>=======--------------------------------------------------=======<<
# >>=======                   Link Binaries                  =======<<
# >>=======--------------------------------------------------=======<<

# ---------------------------------------------
# Link the main binary
$(MAIN_BIN): $(MAIN_OBJ) $(FRONTEND_OBJ) $(CODEGEN_OBJ) $(SETTINGS_OBJ) $(CLI_OBJ) $(COMMON_OBJ) $(DIAGNOSTIC_OBJ) $(LINKER_OBJ) $(TOOLS_OBJ)
    $(CXX) -o $@ $^ $(LDFLAGS)

# ---------------------------------------------
# Link the CLI binary
$(CLI_BIN): $(CLI_OBJ) $(COMMON_OBJ) $(DIAGNOSTIC_OBJ) $(TOOLS_OBJ)
    $(CXX) -o $@ $^ $(LDFLAGS)

# >>=======--------------------------------------------------=======<<
# >>=======                     Cleanup                      =======<<
# >>=======--------------------------------------------------=======<<
clean:
	python3 ./scripts/clean.py

.PHONY: all clean runlexer runparser runmain runcli runtest
