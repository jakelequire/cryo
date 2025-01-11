#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e
# Set the IFS to only split on newlines and tabs
IFS=$'\n\t'
# Set the shell options
shopt -s nullglob
# Set the trap to cleanup on termination
trap EXIT

# Console Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
TEAL='\033[0;36m'
YELLOW='\033[0;33m'
BOLD='\033[1m'
COLOR_RESET='\033[0m'

echo " "
echo "              Cryo Programming Language Installer"
echo " "
echo "This script will install the Cryo Programming Language on your system."
echo "It will install the following components:"
echo " "
echo "1. Cryo CLI"
echo "2. Cryo Language Server Protocol Monitor"
echo "3. Cryo Compiler"
echo " "
echo "In the installation process, the Cryo Compiler will be built from the source code."
echo "After the compilation, it will also link the Cryo CLI to the global path."
echo " "
echo "This script will also install the following dependencies if they are not already installed:"
echo " "
echo "1. LLVM 18"
echo "2. Clang 18"
echo "3. Make"
echo " "

# Check if script is run with sudo
# if [ "$EUID" -ne 0 ]; then
#     echo "Please run with sudo"
#     exit 1
# fi

# Get confirmation from the user
read -p "Do you want to continue with the installation? (Y/n): " choice
if [ "$choice" != "Y" ] && [ "$choice" != "y" ]; then
    echo -e "$RED $BOLD Installation cancelled! $COLOR_RESET"
    exit 1
fi

# ================================================================================
# Cleanup

# Function to cleanup the installation
function errorHandler {
    local reason="$1"
    echo -e "$RED $BOLD Error: $reason $COLOR_RESET"
    echo -e "$RED $BOLD Something went wrong during the installation process. $COLOR_RESET"
    echo -e "$RED $BOLD Please check the error message and try again. $COLOR_RESET"
    exit 1
}

# ================================================================================
# Install Dependencies

# Make sure the user has Clang version 18 installed
# This has to be done 
function installClang {
    echo " "
    echo -e "$TEAL $BOLD Installing Clang... $COLOR_RESET"
    echo " "
    apt-get install clang-18
}

# Make sure the user has LLVM version 18 installed
# This has to be done
function installLLVM {
    echo " "
    echo -e "$TEAL $BOLD Installing LLVM... $COLOR_RESET"
    echo " "
    apt-get install llvm-18
}

# ================================================================================
# Dependency Check

# Check if the user has clang installed and it's above version 18
function checkClang {
    echo " "
    echo -e "$TEAL $BOLD Checking for Clang... $COLOR_RESET"
    echo " "
    # Check if clang is installed
    if ! command -v clang-18 &> /dev/null; then
        echo -e "$RED $BOLD Clang is not installed! $COLOR_RESET"
        echo -e "$RED $BOLD Please install Clang before proceeding with the installation. $COLOR_RESET"
        exit 1
    fi
    # Get the clang version
    clang_version=$(clang-18 --version | grep -oP '(?<=version )[0-9]+')
    # Check if the clang version is above 18
    if [ $clang_version -lt 18 ]; then
        echo -e "$RED $BOLD Clang version is below 18! $COLOR_RESET"
        echo -e "$RED $BOLD Current Version: $clang_version $COLOR_RESET"
        echo -e "$RED $BOLD Please install Clang version 18 or above before proceeding with the installation. $COLOR_RESET"
        read -p "Do you want to install Clang? (Y/n): " choice
        if [ "$choice" != "Y" ] && [ "$choice" != "y" ]; then
            echo -e "$RED $BOLD Installation cancelled! $COLOR_RESET"
            exit 1
        fi
        installClang
        # Run the check again
        checkClang
    fi
    echo -e "$GREEN $BOLD Clang is installed and the version is $clang_version $COLOR_RESET"
}

# Check if the user has LLVM installed and it's above version 18
function checkLLVM {
    echo " "
    echo -e "$TEAL $BOLD Checking for LLVM... $COLOR_RESET"
    echo " "
    # Check if llvm is installed
    if ! command -v llvm-config-18 &> /dev/null; then
        echo -e "$RED $BOLD LLVM is not installed! $COLOR_RESET"
        echo -e "$RED $BOLD Please install LLVM before proceeding with the installation. $COLOR_RESET"
        exit 1
    fi
    # Get the llvm version
    llvm_version=$(llvm-config-18 --version | grep -oP '[0-9]+')
    # Check if the llvm version is above 18
    if [ $llvm_version -lt 18 ]; then
        echo -e "$RED $BOLD LLVM version is below 18! $COLOR_RESET"
        echo -e "$RED $BOLD Current Version: $llvm_version $COLOR_RESET"
        echo -e "$RED $BOLD Please install LLVM version 18 or above before proceeding with the installation. $COLOR_RESET"
        read -p "Do you want to install LLVM? (Y/n): " choice
        if [ "$choice" != "Y" ] && [ "$choice" != "y" ]; then
            echo -e "$RED $BOLD Installation cancelled! $COLOR_RESET"
            exit 1
        fi
        installLLVM
        # Run the check again
        checkLLVM
    fi
    echo -e "$GREEN $BOLD LLVM is installed and the version is $llvm_version $COLOR_RESET"
}

# Check if the user has make installed
function checkMake {
    echo " "
    echo -e "$TEAL $BOLD Checking for Make... $COLOR_RESET"
    echo " "
    # Check if make is installed
    if ! command -v make &> /dev/null; then
        echo -e "$RED $BOLD Make is not installed! $COLOR_RESET"
        echo -e "$RED $BOLD Please install Make before proceeding with the installation. $COLOR_RESET"
        exit 1
    fi
    echo -e "$GREEN $BOLD Make is installed $COLOR_RESET"
}

# Check if the user has the required dependencies installed
checkClang || errorHandler "Clang is not installed"
checkLLVM || errorHandler "LLVM is not installed"
checkMake || errorHandler "Make is not installed"

# ================================================================================
# Build the Cryo Project 

echo " "
echo -e "$TEAL $BOLD Building the Cryo Project... $COLOR_RESET"
echo " "
sleep 1

# Build the Cryo Compiler
function buildProject {
    echo " "
    echo -e "$GREEN $BOLD Building the Cryo Compiler... $COLOR_RESET"
    echo " "
    make all || echo -e "$RED $BOLD Failed to build the Cryo Compiler $COLOR_RESET"
}

# Build the Cryo Project
buildProject || errorHandler "Failed to build the Cryo Project"

# ================================================================================
# Link the Cryo CLI to the global path

echo " "
echo -e "$TEAL $BOLD Linking the Cryo CLI to the global path... $COLOR_RESET"
echo " "
sleep 1

echo -e "The next step requires$BOLD$YELLOW sudo permissions$COLOR_RESET to link the Cryo CLI to the global path."
read -p "Do you want to continue? (Y/n): " choice
if [ "$choice" != "Y" ] && [ "$choice" != "y" ]; then
    echo -e "$RED $BOLD Installation cancelled! $COLOR_RESET"
    exit 1
fi

# Get the absolute path of the current directory
CURRENT_DIR=$(pwd)
echo "Current directory: $CURRENT_DIR"
SCRIPTS_DIR="${CURRENT_DIR}/scripts"

# The script to link the binaries globally: global_setup.sh
GLOBAL_SETUP_SCRIPT="${SCRIPTS_DIR}/global_setup.sh"

# Check if the global setup script exists
if [ -f "$GLOBAL_SETUP_SCRIPT" ]; then
    echo "The global setup script exists"
    # Run the global setup script
    sudo bash $GLOBAL_SETUP_SCRIPT
else
    echo -e "$RED $BOLD The global setup script does not exist $COLOR_RESET"
    echo -e "$RED $BOLD Attempted path: $GLOBAL_SETUP_SCRIPT $COLOR_RESET"
    errorHandler "The global setup script does not exist"
fi

# ================================================================================
# Installation Complete

clear

echo " "
echo -e "$GREEN$BOLD Installation Complete! $COLOR_RESET"
echo " "
echo "The Cryo Programming Language has been successfully installed on your system."
echo "You can now start using the Cryo CLI to compile and run Cryo programs."
echo " "
echo "To get started, you can run the following command:"
echo " "
echo "cryo --help"
echo " "
echo "This will display the help menu for the Cryo CLI."
echo "I hope you enjoy using this passion project of mine."
echo "Please feel free to reach out to me if you have any questions or feedback!"
echo " "
echo "Happy Coding with Cryo! 🚀"
echo " "

# ================================================================================
# End of Script
