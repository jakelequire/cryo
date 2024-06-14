cmake_minimum_required(VERSION 3.10)
project(cryo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_CRT_SECURE_NO_WARNINGS /MDd")

# Add LLVM to the project
find_package(LLVM REQUIRED CONFIG)

# Include LLVM directories
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Include directories for your project
include_directories(${CMAKE_SOURCE_DIR}/src/cpp/include)

# Define your source files
set(SOURCES
    src/cpp/cppmain.cpp
    src/cpp/src/cppASTNode.cpp
    src/cpp/src/cppCodeGen.cpp
)

# Add the executable
add_executable(cpp ${SOURCES})

# Link against LLVM libraries
target_link_libraries(cpp LLVMCore LLVMSupport LLVMIRReader LLVMAsmParser LLVMBitReader LLVMAnalysis)
