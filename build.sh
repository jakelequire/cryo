#!/usr/bin/env bash

# Exit immediately if a command exits with a non-zero status
set -e
# Set the IFS to only split on newlines and tabs
IFS=$'\n\t'
# Set the shell options
shopt -s nullglob
# Set the trap to cleanup on termination
trap cleanup EXIT

## Variables

# BASE_FILE is the main file of the project
BASE_FILE="./tests/main.cryo"
# SRC_FILE is produced by the compiler at the top level of the project
SRC_FILE="output.ll"
# The object file for the standard library
LIB_OBJ="./bin/.o/cryolib.o"
# The build directory
BUILD_DIR="build"
# The object file
OBJ_FILE="output.o"
# The out directory
OUT_DIR="$BUILD_DIR/out"
# The compiler executable
COMPILER_EXE="./bin/compiler"

# Cryo Compiler Arguments
compiler_args=()
# Debug Level
DEBUG_LEVEL=0
# Enable Specific Logs
ENABLE_LOGS=""


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
    echo -e "\e[36m\e[1m\x1b#3══════════════════════════════════════════════════════════════════════════\e[0m\x1b#5"
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
    echo "AST Dump: $0 --ast-dump"
    echo "Options:"
    echo "  -f, --file          The input file to compile"
    echo "  -d, --debug-level   The debug level for the compiler"
    echo "  -L, --enable-logs   Enable specific logs"
    echo "  -o, --output        The output file"
    echo "  --ast-dump          Dump the AST"
    echo "  -h, --help          Display this help and exit"
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

function startTime {
    # Use date command with nanosecond precision
    START_TIME=$(date +%s.%N)
}

function endTime {
    # Get end time with nanosecond precision
    END_TIME=$(date +%s.%N)
    
    # Calculate duration with bc for floating-point arithmetic
    # Scale set to 3 for millisecond precision
    TIME_TAKEN=$(echo "scale=3; $END_TIME - $START_TIME" | bc)
    
    echo " "
    echo "<#> ----------------------------------- <#>"
    echo " "
    printf "     Build time: %.3f seconds\n" $TIME_TAKEN
    echo " "
    echo "<#> ----------------------------------- <#>"
    echo " "
}

# Parsing arguments
while [[ "$#" -gt 0 ]]; do
    case $1 in
        -f|--file)
            if [[ -n "$2" && "$2" != -* ]]; then
                setFileName $2
                compiler_args+=("-f" "$2")
                shift 2
            else
                error "Argument for $1 is missing or invalid"
            fi
            ;;
        -d|--debug-level)
            if [[ -n "$2" && "$2" != -* ]]; then
                DEBUG_LEVEL=$2
                compiler_args+=("-d" "$DEBUG_LEVEL")
                shift 2
            else
                error "Argument for $1 is missing or invalid"
            fi
            ;;
        -L|--enable-logs)
            if [[ -n "$2" && "$2" != -* ]]; then
                ENABLE_LOGS=$2
                compiler_args+=("-L" "$ENABLE_LOGS")
                shift 2
            else
                error "Argument for $1 is missing or invalid"
            fi
            ;;
        -o|--output)
            if [[ -n "$2" && "$2" != -* ]]; then
                OUTPUT_FILE=$2
                compiler_args+=("-o" "$OUTPUT_FILE")
                shift 2
            else
                error "Argument for $1 is missing or invalid"
            fi
            ;;
        --ast-dump)
            compiler_args+=("--ast-dump")
            shift
            ;;
        -h|--help)
            usage
            ;;
        *)
            ;;
    esac
done

# Check if the file exists
if [ ! -f $INPUT_FILE ]; then
    error "The file $INPUT_FILE does not exist"
fi


# ============================================================ #
# Building the project

# Check the build directory
# checkBuildDir 

startTime # Start the timer

make all || error "Failed to build the project"
clear

endTime # End the timer
sleep 1 # Sleep for a second

# ============================================================ #


# Create the necessary directories if they don't exist
mkdir -p $BUILD_DIR
mkdir -p $OUT_DIR

echo " "
echo " "

# For debugging, you can print the arguments like this:
echo "Compiler Args:" "${compiler_args[@]}"

# Compile the project
log "Compiling the project..."
$COMPILER_EXE "${compiler_args[@]}"|| error "Failed to compile the project"

# Print the whole compiler command
log "Command: $COMPILER_EXE -f $INPUT_FILE $COMPILER_ARGS"

# Check if the /build/out/imports directory exists from the compiler
if [ -d $OUT_DIR/imports ]; then
    log "Processing the imports..."
    
    # Array to store valid .ll files
    valid_files=()

    # Process each .ll file individually
    for file in $OUT_DIR/imports/*.ll; do
        base_name=$(basename "$file" .ll)
        log "Processing $file..."

        # Check for external dependencies
        external_deps=$(grep -E "^declare|^@.*= external" "$file" | awk '{print $2}' | tr -d '@')
        if [ ! -z "$external_deps" ]; then
            log "External dependencies found in $file: $external_deps"
            # Here you might want to ensure these dependencies are available
            # For example, you could check if they're in a specific library or another .ll file
        fi
    done

    # Combine all the valid .ll files
    if [ ${#valid_files[@]} -gt 0 ]; then
        log "Combining the .ll files..."
        llvm-link "${valid_files[@]}" -S -o "$OUT_DIR/imports/combined.ll"

        # Optimize the combined IR
        log "Optimizing the combined IR..."
        opt -O3 "$OUT_DIR/imports/combined.ll" -S -o "$OUT_DIR/imports/optimized.ll"

        # Generate final object file
        log "Generating final object file..."
        llc  -filetype=obj -relocation-model=pic "$OUT_DIR/imports/optimized.ll" -o "$OUT_DIR/imports/combined.o"

        # Check if the combined object file was created successfully
        if [ -f "$OUT_DIR/imports/combined.o" ]; then
            log "Combined object file created successfully."
        else
            log "Failed to create combined object file. Please check the LLVM IR files and the compilation process."
            exit 1
        fi
    else
        log "No valid .ll files found to process."
        exit 1
    fi

    # Clean up temporary directory
    rm -rf "$TEMP_DIR"
fi
# Build the project
log "Building the project..."

# Copy the source file to the out directory
log "Copying files to the out directory..."

# Check if the compiler moved the file to the out directory
if [ -f $OUT_DIR/$FILE_NAME.ll ]; then
    log "The compiler moved the $FILE_NAME.ll file to the out directory"
    # Set the input file to the out directory
    INPUT_FILE=$OUT_DIR/$FILE_NAME.ll
else
    log "The compiler did not move the $FILE_NAME.ll file to the out directory"
    # Set the input file to the base file
    INPUT_FILE=$BASE_FILE
fi

# Compile the standard library
C_SUPPORT_IR="c_support.ll"
# clang -S -emit-llvm ./cryo/c_support.c -o $OUT_DIR/cryolib.ll || error "Failed to compile the standard library"
clang++ -S -emit-llvm ./cryo/cxx_support.cpp -o $OUT_DIR/$C_SUPPORT_IR || error "Failed to compile the standard library"


# Combine the `cryolib.ll` and `output.ll` files into one object file
llvm-link $OUT_DIR/$C_SUPPORT_IR $OUT_DIR/$FILE_NAME.ll -S -o $OUT_DIR/bin.ll

# Compile the object file
llc -filetype=obj -relocation-model=static $OUT_DIR/bin.ll -o $OUT_DIR/bin.o

# llc -filetype=asm bin.ll -o bin.s

# Link the object files and place the output in the build directory
clang++ -fno-pie -no-pie  $OUT_DIR/bin.o -o $BUILD_DIR/$FILE_NAME

# Turn it into an executable with no extension

# Cleanup

log "Build completed successfully, running the output file..."
echo ""
echo ""
echo -e "\e[36m\e[1m\x1b#3╔════════════════════════════════════════════════════════════════════════╗"
echo -e "║                          Cryo Program Output                           ║"
echo -e "╚════════════════════════════════════════════════════════════════════════╝\e[0m\x1b#5"
echo ""
echo ""
# Run the output file
$BUILD_DIR/$FILE_NAME

log "----------------------------------------"

# Exit successfully
exit 0
