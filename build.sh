#!/usr/bin/env bash

# Exit immediately if a command exits with a non-zero status
set -e

# Set the IFS to only split on newlines and tabs
IFS=$'\n\t'

# Set the shell options
shopt -s nullglob

# Set the trap to cleanup on error
trap cleanup ERR

# Set the trap to cleanup on termination
trap cleanup SIGTERM

## Variables

# BASE_FILE is the main file of the project
BASE_FILE="./tests/main.cryo"
# SRC_FILE is produced by the compiler at the top level of the project
SRC_FILE="output.ll"
# The object file for the standard library
LIB_OBJ="./src/bin/.o/cryolib.o"
# The build directory
BUILD_DIR="build"
# The output file
OUTPUT_FILE="output"
# The object file
OBJ_FILE="output.o"
# The out directory
OUT_DIR="$BUILD_DIR/out"
# The compiler executable
COMPILER_EXE="./src/bin/main"
# Libs compiler executable
LIBS_COMPILER_EXE="./src/bin/visualDebug"

# Functions
function log {
    echo "[BUILD] $1"
}

function error {
    echo "[ERROR] $1" >&2
    exit 1
}

function cleanup {
    echo ""
    echo ""
    log "----------------------------------------"
    log "Cleaning up..."
    # If the file exists check
    if [ -f $SRC_FILE ]; then
        # Remove the file
        rm $SRC_FILE
    fi
}

function usage {
    # No arguments - compiles the project how it is
    # -libs - compiles the project libs
    echo "Usage: $0 [-libs]"
    exit 1
}

function compileLibs {
    # Compile the libraries
    log "Compiling the libraries..."
    make libs || error "Failed to compile the libraries"
    log "Libraries compiled successfully"

    # Run the library
    log "Running the library..."
    $LIBS_COMPILER_EXE $BASE_FILE || error "Failed to run the library"
    log "Library ran successfully"
}

# Parse command-line arguments
while [[ "$#" -gt 0 ]]; do
    case $1 in
        -libs) compileLibs ;;
        *) error "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done





# Build from make
make all || error "Failed to build the project"

# Clear the screen
clear

# Create the necessary directories if they don't exist
mkdir -p $BUILD_DIR
mkdir -p $OUT_DIR

# Compile the project
log "Compiling the project..."
$COMPILER_EXE $BASE_FILE || error "Compilation failed"

# Build the project
log "Building the project..."

# Copy the source file to the out directory
log "Copying files to the out directory..."
cp $SRC_FILE $OUT_DIR/output.ll || error "Failed to copy $SRC_FILE"

# Compile the standard library
clang -S -emit-llvm ./src/cryo/std.c -o $OUT_DIR/cryolib.ll

# Change to the out directory
cd $OUT_DIR

# Combine the `cryolib.ll` and `output.ll` files into one object file
llvm-link cryolib.ll output.ll -o bin.ll 

# Compile the object file
llc -filetype=obj -relocation-model=static bin.ll -o bin.o

# Change back to the original directory
cd ../../

# Link the object files and place the output in the build directory
clang++ -fno-pie -no-pie  $OUT_DIR/bin.o -o $BUILD_DIR/$OUTPUT_FILE

# Cleanup
cleanup

log "Build completed successfully, running the output file..."
log ">===----------------<Output>----------------===<"
echo ""
echo ""
# Run the output file
$BUILD_DIR/$OUTPUT_FILE


# Exit successfully
exit 0
