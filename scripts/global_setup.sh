#!/usr/bin/env bash

# /usr/local/bin/ is the directory where the symbolic link will be created
# for the Cryo Compiler, CLI, and cryo-path binaries.

# Exit immediately if a command exits with a non-zero status
set -e

echo "Starting process to link Cryo Binaries globally..."

# Get the absolute path of the current directory
CURRENT_DIR=$(pwd)
echo "Current directory: $CURRENT_DIR"
BIN_DIR="${CURRENT_DIR}/bin"

function unlinkBinaries {
    # Check if the cryo binary exists
    if [ -f "/usr/local/bin/cryo" ]; then
        echo "The cryo binary exists"
        # Remove the symbolic link
        rm /usr/local/bin/cryo
        echo "The cryo binary has been unlinked globally"
    else
        echo "The cryo binary does not exist"
    fi
    # Check if the cryo-path binary exists
    if [ -f "/usr/local/bin/cryo-path" ]; then
        echo "The cryo-path binary exists"
        # Remove the symbolic link
        rm /usr/local/bin/cryo-path
        echo "The cryo-path binary has been unlinked globally"
    else
        echo "The cryo-path binary does not exist"
    fi
}

unlinkBinaries

# The binaries that will be linked globally
# - cryo
# - cryo-path
function checkBinaries {
    # Check if the bin directory exists
    if [ -d "$BIN_DIR" ]; then
        # Check if the directory is empty
        if [ "$(ls -A $BIN_DIR)" ]; then
            # The directory is not empty
            echo "The bin directory is not empty"
            # Check if the cryo binary exists
            if [ -f "$BIN_DIR/cryo" ]; then
                echo "The cryo binary exists"
                # Create a symbolic link in /usr/local/bin
                ln -sf "$BIN_DIR/cryo" /usr/local/bin/cryo
                echo "The cryo binary has been linked globally"
            else
                echo "The cryo binary does not exist"
            fi
            # Check if the cryo-path binary exists
            if [ -f "$BIN_DIR/cryo-path" ]; then
                echo "The cryo-path binary exists"
                # Create a symbolic link in /usr/local/bin
                ln -sf "$BIN_DIR/cryo-path" /usr/local/bin/cryo-path
                echo "The cryo-path binary has been linked globally"
            else
                echo "The cryo-path binary does not exist"
            fi
            # Check if the cryoconfig binary exists
            if [ -f "$BIN_DIR/cryoconfig" ]; then
                echo "The cryoconfig binary exists"
                # Create a symbolic link in /usr/local/bin
                ln -sf "$BIN_DIR/cryoconfig" /usr/local/bin/cryoconfig
                echo "The cryoconfig binary has been linked globally"
            else
                echo "The cryoconfig binary does not exist"
            fi
        else
            # The directory is empty
            echo "The bin directory is empty"
        fi
    else
        # The directory does not exist
        echo "The bin directory does not exist"
    fi
}

checkBinaries

echo "Cryo Binaries have been linked globally"
