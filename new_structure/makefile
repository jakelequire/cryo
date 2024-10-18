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

# ---------------------------------------------
# Clean up - remove object files and executables
clean:
	python3 ./scripts/clean.py

.PHONY: all clean runlexer runparser runmain runcli runtest
