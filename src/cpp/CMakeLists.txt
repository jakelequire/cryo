cmake_minimum_required(VERSION 3.10)

# Project name
project(CPPProject)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find LLVM
find_package(LLVM REQUIRED CONFIG)
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(HandleLLVMOptions)

# Include directories
# Include LLVM
include_directories(include ${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
# Include cJSON
include_directories(lib/cJSON)

# Source files
file(GLOB SOURCES "src/*.cpp")

# Set output directory for executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Executable
add_executable(CPPProject cppmain.cpp ${SOURCES})

# Link LLVM
llvm_map_components_to_libnames(llvm_libs core support)
target_link_libraries(CPPProject ${llvm_libs})

# Enable warnings but do not treat them as errors
if (MSVC)
    target_compile_options(CPPProject PRIVATE /W4 /MD /D_ITERATOR_DEBUG_LEVEL=0)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
else()
    target_compile_options(CPPProject PRIVATE -Wall -Wextra -Wpedantic)
endif()


