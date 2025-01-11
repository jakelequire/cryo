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

# Console Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
TEAL='\033[0;36m'
BOLD='\033[1m'
COLOR_RESET='\033[0m'

INIT_LIBS_SIG="$TEAL$BOLD[INIT_LIBS]$COLOR_RESET"

# Function to build and move binaries for each library
function buildAndMoveLib {
    LIB_ROOT=$1
    LIB_NAME=$(basename $LIB_ROOT)
    echo -e "$INIT_LIBS_SIG Building the $LIB_NAME library"
    cd $LIB_ROOT
    make all || echo -e "$INIT_LIBS_SIG $RED $BOLD Failed to build the $LIB_NAME library $COLOR_RESET"
    if [ -f "bin/$LIB_NAME" ]; then
        echo -e "$INIT_LIBS_SIG $RED $BOLD Moving the $LIB_NAME binary to the root bin directory $COLOR_RESET"
        mv "bin/$LIB_NAME" $ROOT_BIN_DIR
        if [ "$LIB_NAME" == "cli" ]; then
            echo -e "$INIT_LIBS_SIG $GREEN $BOLD Renaming the $LIB_NAME binary to cryo $COLOR_RESET"
            mv "$ROOT_BIN_DIR/$LIB_NAME" "$ROOT_BIN_DIR/cryo"
        fi
        if [ "$LIB_NAME" == "lsp-monitor" ]; then
            echo -e "$INIT_LIBS_SIG $GREEN $BOLD Renaming the $LIB_NAME binary to lspmonitor $COLOR_RESET"
            mv "$ROOT_BIN_DIR/$LIB_NAME" "$ROOT_BIN_DIR/lspmonitor"
        fi
        if [ "$LIB_NAME" == "cryo_path" ]; then
            echo -e "$INIT_LIBS_SIG $GREEN $BOLD Renaming the $LIB_NAME binary to cryo-path $COLOR_RESET"
            mv "$ROOT_BIN_DIR/$LIB_NAME" "$ROOT_BIN_DIR/cryo-path"
        fi
    else
        echo -e "$INIT_LIBS_SIG $RED $BOLD No binaries found to move for $LIB_NAME $COLOR_RESET"
        echo -e "$INIT_LIBS_SIG Attempted path: $LIB_ROOT/bin/$LIB_NAME"
    fi
    cd ..
}

# Function to cleanup each library
function cleanupLib {
    LIB_ROOT=$1
    LIB_NAME=$(basename $LIB_ROOT)
    echo -e "$INIT_LIBS_SIG Cleaning up the $LIB_NAME library"
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
        echo -e "$INIT_LIBS_SIG Processing the $lib library"
        buildAndMoveLib "./libs/$lib"
    else
        echo -e "$INIT_LIBS_SIG Library $lib does not exist"
        echo -e "$INIT_LIBS_SIG Attempted path: ./libs/$lib"
    fi
done
