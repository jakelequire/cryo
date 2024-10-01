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
# The object file
OBJ_FILE="output.o"
# The out directory
OUT_DIR="$BUILD_DIR/out"
# The compiler executable
COMPILER_EXE="./src/bin/main"


# Base File
INPUT_FILE=""
# Output File
OUTPUT_FILE=""
# File Name
FILE_NAME=""

function checkBuildDir {
    # Check the build directory if it exists & has files
    if [ -d $BUILD_DIR ]; then
        # Check if the directory is empty
        if [ "$(ls -A $BUILD_DIR)" ]; then
            # The directory is not empty
            log "The build directory is not empty"

            rm -r $BUILD_DIR/*
            log "Deleted the contents of the build directory"

        else
            # The directory is empty
            log "The build directory is empty"
        fi
    else
        # The directory does not exist
        log "The build directory does not exist"
    fi
}


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
    if [ -f "$FILE_NAME.ll" ]; then
        # Remove the file
        rm $FILE_NAME.ll
    fi
}

function usage {
    # No arguments - compiles the project how it is
    # -libs - compiles the project libs
    echo "Usage: $0 [-f {filename}]"
    exit 1
}

function setFileName {
    # Set the input file
    INPUT_FILE=$1
    log "Setting the input file to $INPUT_FILE"
    # Set the output file
    OUTPUT_FILE=$(basename $INPUT_FILE)
    log "Setting the output file to $OUTPUT_FILE"
    # Remove the extension
    FILE_NAME="${OUTPUT_FILE%.*}"
    log "Setting the file name to $OUTPUT_FILE"
}

# Parse command-line arguments
while [[ "$#" -gt 0 ]]; do
    case $1 in
        -f|--file)
            if [[ -n "$2" && "$2" != -* ]]; then
                setFileName $2
                shift 2
            else
                error "Argument for $1 is missing or invalid"
            fi
            ;;
        -h|--help)
            usage
            ;;
        *)
            error "Unknown argument: $1"
            ;;
    esac
done

# Check if the file exists
if [ ! -f $INPUT_FILE ]; then
    error "The file $INPUT_FILE does not exist"
fi

# Check the build directory
# checkBuildDir 

# Build from make
make all || error "Failed to build the project"


# Create the necessary directories if they don't exist
mkdir -p $BUILD_DIR
mkdir -p $OUT_DIR

# Compile the project
log "Compiling the project..."
$COMPILER_EXE "-f" $INPUT_FILE || error "Compilation failed"

# Build the project
log "Building the project..."

# Copy the source file to the out directory
log "Copying files to the out directory..."
cp $FILE_NAME.ll $OUT_DIR/$FILE_NAME.ll || error "Failed to copy the source file to the out directory"

# Compile the standard library
clang -S -emit-llvm ./src/cryo/std.c -o $OUT_DIR/cryolib.ll || error "Failed to compile the standard library"

# Change to the out directory
cd $OUT_DIR

# Combine the `cryolib.ll` and `output.ll` files into one object file
llvm-link cryolib.ll $FILE_NAME.ll -S -o bin.ll

# Compile the object file
llc -filetype=obj -relocation-model=static bin.ll -o bin.o

# llc -filetype=asm bin.ll -o bin.s

# Change back to the original directory
cd ../../

# Link the object files and place the output in the build directory
clang++ -fno-pie -no-pie  $OUT_DIR/bin.o -o $BUILD_DIR/$FILE_NAME

# Turn it into an executable with no extension

# Cleanup
cleanup

log "Build completed successfully, running the output file..."
log ">===----------------<Output>----------------===<"
echo ""
echo ""
# Run the output file
$BUILD_DIR/$FILE_NAME


# Exit successfully
exit 0
