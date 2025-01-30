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
PURPLE='\033[0;35m'
BLUE='\033[0;34m'
YELLOW='\033[0;33m'
GREY='\033[0;37m'
BOLD='\033[1m'
ITALIC='\033[3m'
UNDERLINE='\033[4m'
COLOR_RESET='\033[0m'
NEW_LINE=$'\n'

echo -e "$TEAL"
echo "               ██████╗██████╗ ██╗   ██╗ ██████╗ "
echo "              ██╔════╝██╔══██╗╚██╗ ██╔╝██╔═══██╗"
echo "              ██║     ██████╔╝ ╚████╔╝ ██║   ██║"
echo "              ██║     ██╔══██╗  ╚██╔╝  ██║   ██║"
echo "              ╚██████╗██║  ██║   ██║   ╚██████╔╝"
echo "               ╚═════╝╚═╝  ╚═╝   ╚═╝    ╚═════╝ "
echo -e "$COLOR_RESET"
echo -e "$TEAL$BOLD              Cryo Programming Language Installer $COLOR_RESET"
echo " "
echo "This script will install the Cryo Programming Language on your system."
echo "It will install/compile the following components:"
echo " "
echo -e "$BLUE$BOLD  1. Cryo CLI$COLOR_RESET"
echo -e "$BLUE$BOLD  2. Cryo Compiler$COLOR_RESET"
echo -e "$BLUE$BOLD  3. cryo-path$COLOR_RESET"
echo -e "$BLUE$BOLD  4. LSP Debug Server$COLOR_RESET"
echo " "
echo "In the installation process, the Cryo Compiler will be built from the source code."
echo "After the compilation, it will also link the Cryo CLI to the global path."
echo " "
echo "This script will also install the following dependencies if they are not already installed:"
echo " "
echo -e "$GREEN$BOLD  1. LLVM 18$COLOR_RESET"
echo -e "$GREEN$BOLD  2. Clang 18$COLOR_RESET"
echo -e "$GREEN$BOLD  3. Make$COLOR_RESET"
echo " "
echo -e "Please note, this script will only work on $YELLOW$BOLD$UNDERLINE*Debian-based systems*$COLOR_RESET. It has been developed"
echo "and tested on Ubuntu. If you are using a different system, I cannot guarantee that this"
echo "script will work for you."
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
    echo " "
    echo " "
    echo -e "$RED $BOLD Error: $reason $COLOR_RESET"
    echo -e "$RED $BOLD Something went wrong during the installation process. $COLOR_RESET"
    echo -e "$RED $BOLD Please check the error message and try again. $COLOR_RESET"
    echo " "
    echo " "
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
    # Download the LLVM installation script
    wget https://apt.llvm.org/llvm.sh
    chmod +x llvm.sh
    # Install LLVM
    sudo ./llvm.sh 18
    # Cleanup the installation script
    rm llvm.sh
}

# ================================================================================
# Dependency Check

# Check if the user has clang installed and it's above version 18
function checkClang {
    echo " "
    echo -e "$GREY Checking for Clang... $COLOR_RESET"
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
    echo -e "$GREY Checking for LLVM... $COLOR_RESET"
    echo " "
    # Check if llvm is installed
    if ! command -v llvm-config-18 &> /dev/null; then
        echo -e "$RED $BOLD LLVM is not installed! $COLOR_RESET"
        echo -e "$RED $BOLD Please install LLVM before proceeding with the installation. $COLOR_RESET"
        # Ask the user if they want to install LLVM
        read -p "Do you want to install LLVM? (Y/n): " choice
        if [ "$choice" != "Y" ] && [ "$choice" != "y" ]; then
            echo -e "$RED $BOLD Installation cancelled! $COLOR_RESET"
            exit 1
        fi
        installLLVM
    fi
    # Get the llvm version
    llvm_version=$(llvm-config-18 --version | awk '{print $1}' | cut -d'.' -f1)
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
    echo -e "$GREY Checking for Make... $COLOR_RESET"
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
# File Icon Installation

function installFileIcon {
    echo " "
    echo -e "$TEAL $BOLD Installing Cryo file icons... $COLOR_RESET"
    echo " "
    
    # Create necessary directories if they don't exist
    sudo mkdir -p /usr/share/mime/packages
    sudo mkdir -p /usr/share/icons/hicolor/scalable/mimetypes
    
    # Create the MIME type definition file
    cat > cryo-mime.xml << EOF
<?xml version="1.0" encoding="UTF-8"?>
<mime-info xmlns="http://www.freedesktop.org/standards/shared-mime-info">
    <mime-type type="text/x-cryo">
        <comment>Cryo source code</comment>
        <glob pattern="*.cryo"/>
    </mime-type>
</mime-info>
EOF
    
    # Install the MIME type definition
    sudo cp cryo-mime.xml /usr/share/mime/packages/
    
    # Create an SVG icon for Cryo files
    cat > cryo-file.svg << EOF
<?xml version="1.0" encoding="UTF-8"?>
<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="70" height="70" viewBox="10 10 180 180">
  <image x="15" y="9" width="170" height="184" xlink:href="data:img/png;base64,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"/>
</svg>
EOF
    
    # Install the icon
    sudo cp cryo-file.svg /usr/share/icons/hicolor/scalable/mimetypes/text-x-cryo.svg
    
    # Update the MIME and icon caches
    sudo update-mime-database /usr/share/mime
    sudo gtk-update-icon-cache /usr/share/icons/hicolor/
    
    # Cleanup temporary files
    rm cryo-mime.xml cryo-file.svg
}

# ================================================================================
# Env Variables

function installEnvVars {
    echo "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-"
    echo " "
    echo -e "$TEAL $BOLD Installing Environment Variables... $COLOR_RESET"
    echo " "
    sleep 1

    # Get the absolute path of the current directory
    CURRENT_DIR=$(pwd)
    echo "Current directory: $CURRENT_DIR"
    SCRIPTS_DIR="${CURRENT_DIR}/scripts"

    # The script to install the environment variables: env_vars.sh
    ENV_VARS_SCRIPT="${SCRIPTS_DIR}/cryoenv.sh"

    # Check if the env_vars script exists
    if [ -f "$ENV_VARS_SCRIPT" ]; then
        echo "The cryoenv script exists"
        echo " "
        # Run the env_vars script
        sudo bash $ENV_VARS_SCRIPT
    else
        echo -e "$RED $BOLD The env_vars script does not exist $COLOR_RESET"
        echo -e "$RED $BOLD Attempted path: $ENV_VARS_SCRIPT $COLOR_RESET"
        errorHandler "The cryoenv script does not exist"
    fi
}


# ================================================================================
# Build the Cryo Project 

echo " "
echo "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-"
echo " "
echo -e "$GREY$BOLD Building the Cryo Project... $COLOR_RESET"
echo " "
sleep 1

# Build the Cryo Compiler
function buildProject {
    echo " "
    echo -e "$GREEN$BOLD Building the Cryo Compiler... $COLOR_RESET"
    echo " "
    make all || echo -e "$RED $BOLD Failed to build the Cryo Compiler $COLOR_RESET"
}

# Pause Execution, this will take a while if this is the first time building the project
echo " "
echo -e "$TEAL $BOLD"
read -n 1 -s -r -p "If this is the first time building the compiler, this may take a while. ${NEW_LINE}Press any key to continue..."
echo -e "$COLOR_RESET"

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


# Next, we will install the environment variables for Cryo
echo " "
echo " "
echo "Next, we will install the environment variables for Cryo."
echo "This will set up the CRYO_COMPILER environment variable."
echo "It is required for many components of the compiler and its tools."
echo " "

installEnvVars || errorHandler "Failed to install the environment variables for Cryo"


# Pause Execution, let the user read the output and press any key to continue
echo " "
echo -e "$TEAL $BOLD"
read -n 1 -s -r -p "Press any key to continue..."
echo -e "$COLOR_RESET"


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
echo " "
echo "I hope you enjoy using this passion project of mine."
echo "This is not a full-fledged programming language, but it's a start!"
echo "You can find documentation and examples on the GitHub repository."
echo " "
echo -e "$TEAL$BOLD https://github.com/jakelequire/cryo $COLOR_RESET"
echo " "
echo "Please feel free to reach out to me if you have any questions or feedback!"
echo " "
echo "Happy Coding with Cryo! ❄️"
echo " "
# ================================================================================
# End of Script

