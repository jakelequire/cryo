#!/usr/bin/env bash

# Exit immediately if a command exits with a non-zero status
set -e

# Variables
BASE_FILE="./tests/main.cryo"
SRC_FILE="output.ll"
LIB_OBJ="./src/bin/.o/cryolib.o"
BUILD_DIR="build"
OUTPUT_FILE="output"
OBJ_FILE="output.o"
COMPILER_EXE="./src/bin/main"

# Functions
function log {
    echo "[INFO] $1"
}

function error {
    echo "[ERROR] $1" >&2
    exit 1
}

# Compile the project
log "Compiling the project..."
$COMPILER_EXE $BASE_FILE || error "Compilation failed"


# Build the project
log "Building the project..."

# Create the build directory if it doesn't exist
mkdir -p $BUILD_DIR

# Copy the source file & object file to the build directory
log "Copying files to the build directory..."
cp $SRC_FILE $BUILD_DIR || error "Failed to copy $SRC_FILE"
cp $LIB_OBJ $BUILD_DIR || error "Failed to copy $LIB_OBJ"

# Change to the build directory
cd $BUILD_DIR

# Compile the source file
log "Compiling the source file..."
llc -filetype=obj $SRC_FILE -o $OBJ_FILE || error "Compilation failed"

ld -relocatable $(basename $LIB_OBJ) $OBJ_FILE  -o bin.o || error "Compilation failed"

# Link the object files
log "Linking the object files..."
clang++  $OBJ_FILE -o $OUTPUT_FILE || error "Linking failed"

# Run the output file
log "Running the output file..."
./$OUTPUT_FILE || error "Execution failed"

log "Build and execution completed successfully."
