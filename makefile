#*******************************************************************************
#  Copyright 2025 Jacob LeQuire                                                *
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
#*******************************************************************************

#*******************************************************************************
# Cryo Compiler Makefile                                                       *
#*******************************************************************************

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
OPTIMIZATION =  -Og
NO_WARNINGS =   -w
DEBUG_FLAGS =   -D_CRT_SECURE_NO_WARNINGS  $(NO_WARNINGS)
C_STANDARD =    -std=c23
CXX_STANDARD =  -std=c++17

# Determine number of CPU cores
ifeq ($(OS), Windows_NT)
    NUM_CORES = $(NUMBER_OF_PROCESSORS)
else
    NUM_CORES = $(shell nproc)
endif

NUM_JOBS = $(shell expr $(NUM_CORES) + 1)

# OS-specific settings for compilers
ifeq ($(OS), Windows_NT)
# Windows settings
	C_COMPILER = C:/msys64/mingw64/bin/gcc
	CXX_COMPILER = C:/msys64/mingw64/bin/g++
else
# Linux settings
	C_COMPILER = clang-18
	CXX_COMPILER = clang++-18
endif

# >>=======--------------------------------------------------=======<< #
# >>=======                 Include Paths                    =======<< #
# >>=======--------------------------------------------------=======<< #

# Include paths
LINUX_INCLUDES =    -I./include -I./include/codegen -I./include/codegen/generation -I./include/codegen/IRSymTable \
                    -I./include/codegen/generation/codegen.hpp -I./include/codegen/IRSymTable/IRSymTable.hpp \

WIN_INCLUDES =      -I"C:/msys64/mingw64/include" -I./include -I./include/codegen -I./include/cli       \
                    -I./include/common -I./include/diagnostics -I./include/linker -I./include/settings  \
                    -I./include/tools -I./include/tools/macros -I./include/tools/utils                  \
                    -I./include/frontend

# >>=======--------------------------------------------------=======<< #
# >>=======       OS-specific settings for compilers         =======<< #
# >>=======--------------------------------------------------=======<< #

# OS-specific settings
ifeq ($(OS), Windows_NT)
    # Windows settings
    CC =            $(C_COMPILER) $(C_STANDARD) $(DEBUG_FLAGS) $(OPTIMIZATION)
    CXX =           $(CXX_COMPILER) $(CXX_STANDARD) $(DEBUG_FLAGS) $(OPTIMIZATION)
    CFLAGS =        $(WIN_INCLUDES) $(LLVM_CFLAGS) -fexceptions
    CXXFLAGS =      $(WIN_INCLUDES) $(LLVM_CXXFLAGS) -fexceptions
    LDFLAGS =       -L"C:/msys64/mingw64/lib" $(LLVM_LIBS) $(STDLIBS) -v
    LLVM_LIBS :=    -lLLVM 
    STDLIBS :=      -lmingw32 -lmingwex -lmsvcrt -lucrt -lpthread -lws2_32 -ladvapi32 -lshell32 -luser32 -lkernel32 -Wl,-subsystem,console
    MKDIR =         if not exist
    RMDIR =         rmdir /S /Q
    DEL =           del /Q
    BIN_SUFFIX =    .exe
else
    # Linux settings
    CC =            $(C_COMPILER) $(C_STANDARD) $(DEBUG_FLAGS) $(OPTIMIZATION)
    CXX =           $(CXX_COMPILER) $(CXX_STANDARD) $(DEBUG_FLAGS) $(OPTIMIZATION)
    CFLAGS =        $(LINUX_INCLUDES) $(LLVM_CFLAGS) -fexceptions
    CXXFLAGS =      $(LINUX_INCLUDES) $(LLVM_CXXFLAGS) -fexceptions
    LLVM_CONFIG =   llvm-config-18
    LLVM_CFLAGS =   $(shell $(LLVM_CONFIG) --cflags)
	LLVM_CXXFLAGS = $(shell $(LLVM_CONFIG) --cxxflags)
    LLVM_LDFLAGS =  $(shell $(LLVM_CONFIG) --ldflags) $(shell $(LLVM_CONFIG) --libs) $(shell $(LLVM_CONFIG) --system-libs)
    LDFLAGS =       $(LLVM_LDFLAGS) -lpthread -v
	STD_LIBS =      -lstdc++ -lm -lc -lgcc -lgcc_eh -lstdc++fs
    MKDIR =         mkdir -p
    RMDIR =         rm -rf
    DEL =           rm -f
    BIN_SUFFIX =
endif

# >>=======--------------------------------------------------=======<< #
# >>=======                  Define Paths                    =======<< #
# >>=======--------------------------------------------------=======<< #

# ---------------------------------------------
# Binary directory
BIN_DIR =       ./bin/
OBJ_DIR =       $(BIN_DIR).o/
DEBUG_BIN_DIR = $(BIN_DIR)debug/

# ---------------------------------------------
# Source directory
SRC_DIR =   ./src/
TOOLS_DIR =   ./tools/
TOOLS_INIT = $(TOOLS_DIR)initTools.sh
MAIN_FILE = $(SRC_DIR)main.c

# >>=======--------------------------------------------------=======<< #
# >>=======                 Compilation Rules                =======<< #
# >>=======--------------------------------------------------=======<< #

# ---------------------------------------------
# Main binary
MAIN_BIN = $(BIN_DIR)compiler$(BIN_SUFFIX)

# ---------------------------------------------
# Ensure OBJ_DIR exists
$(shell mkdir -p $(OBJ_DIR))

# ---------------------------------------------
# Define all source files
C_SRCS := $(shell find $(SRC_DIR) -name '*.c')
CPP_SRCS := $(shell find $(SRC_DIR) -name '*.cpp')

# ---------------------------------------------
# Define all object files
C_OBJS := $(patsubst $(SRC_DIR)%.c,$(OBJ_DIR)%.o,$(C_SRCS))
CPP_OBJS := $(patsubst $(SRC_DIR)%.cpp,$(OBJ_DIR)%.o,$(CPP_SRCS))

# ---------------------------------------------
# Combine all object files
ALL_OBJS := $(C_OBJS) $(CPP_OBJS)

# Add these directory rules
$(BIN_DIR) $(OBJ_DIR):
	@mkdir -p $@

# ---------------------------------------------
# Compile C source files
$(OBJ_DIR)%.o: $(SRC_DIR)%.c | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) -c $< -o $@

# ---------------------------------------------
# Compile C++ source files
$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# >>=======--------------------------------------------------=======<< #
# >>=======                   Link Binaries                  =======<< #
# >>=======--------------------------------------------------=======<< #

# Main target
$(MAIN_BIN): build-cryo-path build-cli build-lsp-monitor build-dev-server $(ALL_OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(ALL_OBJS) -o $@ $(LDFLAGS)

# Build the CLI library
build-cryo-path:
	@$(TOOLS_INIT) cryo_path
build-cli:
	@$(TOOLS_INIT) cli
build-lsp-monitor:
	@$(TOOLS_INIT) lsp-monitor
build-dev-server:
	@$(TOOLS_INIT) dev-server


# >>=======--------------------------------------------------=======<< #
# >>=======                     Commands                     =======<< #
# >>=======--------------------------------------------------=======<< #

cls:
	@clear


# Build timer script
BUILD_TIMER = ./scripts/build_timer.py

# Timed build target
.PHONY: timed-build
timed-build:
	@python3 $(BUILD_TIMER)

# Clean and timed build
.PHONY: rebuild
rebuild:
	@python3 $(BUILD_TIMER) --clean

# Timed build without tests
.PHONY: quick-build
quick-build:
	@python3 $(BUILD_TIMER) --no-tests

# Verbose timed build with compiler info
.PHONY: verbose-build
verbose-build:
	@python3 $(BUILD_TIMER) --verbose

.PHONY: all
all: 
	@$(MAKE) timed-build
	

.PHONY: build
build: $(MAIN_BIN)

libs:
	$(TOOLS_INIT) 

# Define the valid clean targets
CLEAN_TARGETS := codegen common compiler diagnostics frontend linker runtime settings symbolTable tools main

# Generic clean target for any component
clean-%:
	python3 ./scripts/custom_clean.py $*

# Clean all components
clean:
	python3 ./scripts/clean.py

.PHONY: debug clean all 
.NOTPARALLEL: clean clean-% libs

# >>=======--------------------------------------------------=======<< #
# >>=======                  Test Suite                      =======<< #
# >>=======--------------------------------------------------=======<< #

# Test Suite Configuration
TEST_SRC_DIR = ./tests/
TEST_FRAMEWORK_DIR = $(TEST_SRC_DIR)framework/
TEST_OBJ_DIR = $(OBJ_DIR)tests/
TEST_BIN_DIR = $(BIN_DIR)tests/

# Test binary names
CRYO_TEST_SUITE = $(TEST_BIN_DIR)cryoTestSuite$(BIN_SUFFIX)
AST_VERIFIER = $(TEST_BIN_DIR)astVerifier$(BIN_SUFFIX)

# Ensure test directories exist
$(shell mkdir -p $(TEST_OBJ_DIR)/framework $(TEST_BIN_DIR))

# Define test source files - explicitly list them to avoid issues
TEST_FRAMEWORK_SRCS = $(TEST_FRAMEWORK_DIR)test_runner.c $(TEST_FRAMEWORK_DIR)ast_verifier.c
TEST_SRCS = $(TEST_SRC_DIR)cryoTestSuite.c

# Define test object files
TEST_FRAMEWORK_OBJS = $(patsubst $(TEST_FRAMEWORK_DIR)%.c,$(TEST_OBJ_DIR)framework/%.o,$(notdir $(TEST_FRAMEWORK_SRCS)))
TEST_OBJS = $(patsubst $(TEST_SRC_DIR)%.c,$(TEST_OBJ_DIR)%.o,$(notdir $(TEST_SRCS)))

# Combined test objects
ALL_TEST_OBJS = $(TEST_FRAMEWORK_OBJS) $(TEST_OBJS)

# Compile test framework C files (explicitly defined)
$(TEST_OBJ_DIR)framework/test_runner.o: $(TEST_FRAMEWORK_DIR)test_runner.c
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TEST_OBJ_DIR)framework/ast_verifier.o: $(TEST_FRAMEWORK_DIR)ast_verifier.c
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile main test file
$(TEST_OBJ_DIR)cryoTestSuite.o: $(TEST_SRC_DIR)cryoTestSuite.c
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link test binaries - only make the main test suite for now
$(CRYO_TEST_SUITE): $(TEST_OBJ_DIR)cryoTestSuite.o $(TEST_OBJ_DIR)framework/test_runner.o $(TEST_OBJ_DIR)framework/ast_verifier.o
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $^ $(filter-out $(OBJ_DIR)main.o, $(ALL_OBJS)) -o $@ $(LDFLAGS) $(STD_LIBS)

# Let's simplify for now - we'll just build the main test suite
tests: $(CRYO_TEST_SUITE)

run-tests: tests
	@echo "Running Cryo Test Suite..."
	@$(CRYO_TEST_SUITE)

clean-tests:
	@echo "Cleaning test files..."
	@$(RMDIR) $(TEST_OBJ_DIR) 2>/dev/null || true
	@$(RMDIR) $(TEST_BIN_DIR) 2>/dev/null || true
	