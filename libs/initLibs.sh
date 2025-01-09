#!/usr/bin/env bash

# Exit immediately if a command exits with a non-zero status
set -e
# Set the IFS to only split on newlines and tabs
IFS=$'\n\t'
# Set the shell options
shopt -s nullglob
# Set the trap to cleanup on termination
trap cleanup EXIT

# Root bin directory
ROOT_BIN_DIR=$(pwd)/bin

# Function to build and move binaries for each library
function buildAndMoveLib {
    LIB_ROOT=$1
    LIB_NAME=$(basename $LIB_ROOT)
    echo "Building the $LIB_NAME library"
    cd $LIB_ROOT
    make all
    if [ -f "bin/$LIB_NAME" ]; then
        echo "Moving the $LIB_NAME binary to the root bin directory"
        mv "bin/$LIB_NAME" $ROOT_BIN_DIR
        if [ "$LIB_NAME" == "cli" ]; then
            echo "Renaming the $LIB_NAME binary to cryo"
            mv "$ROOT_BIN_DIR/$LIB_NAME" "$ROOT_BIN_DIR/cryo"
        fi
        if [ "$LIB_NAME" == "lsp-monitor" ]; then
            echo "Renaming the $LIB_NAME binary to lspmonitor"
            mv "$ROOT_BIN_DIR/$LIB_NAME" "$ROOT_BIN_DIR/lspmonitor"
        fi
    else
        echo "No binaries found to move for $LIB_NAME"
        echo "Attempted path: $LIB_ROOT/bin/$LIB_NAME"
    fi
    cd ..
}

# Function to cleanup each library
function cleanupLib {
    LIB_ROOT=$1
    LIB_NAME=$(basename $LIB_ROOT)
    echo "Cleaning up the $LIB_NAME library"
    cd $LIB_ROOT
    make clean
    cd ..
}

# Cleanup function to be called on script exit
function cleanup {
    for lib in "$@"; do
        if [ -d "./libs/$lib" ]; then
            cleanupLib "./libs/$lib"
        fi
    done
}

# Build and move binaries for each specified library
for lib in "$@"; do
    if [ -d "./libs/$lib" ]; then
        echo "Processing the $lib library"
        buildAndMoveLib "./libs/$lib"
    else
        echo "Library $lib does not exist"
        echo "Attempted path: ./libs/$lib"
    fi
done
