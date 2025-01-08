#!/bin/bash

# Check if script is run with sudo
if [ "$EUID" -ne 0 ]; then
    echo "Please run with sudo"
    exit 1
fi

# Get the absolute path of the current directory
CURRENT_DIR=$(pwd)
COMPILER_PATH="${CURRENT_DIR}/bin/compiler"
VAR_NAME="CRYO_COMPILER"

# Check if compiler exists
if [ ! -f "$COMPILER_PATH" ]; then
    echo "Error: Compiler not found at ${COMPILER_PATH}"
    exit 1
fi

# Function to check if variable exists in /etc/environment
check_variable() {
    local var_name=$1
    grep -q "^${var_name}=" /etc/environment
    return $?
}

# Function to add variable to /etc/environment
add_variable() {
    local var_name=$1
    local var_value=$2
    echo "${var_name}=${var_value}" >> /etc/environment
    echo "Added ${var_name}=${var_value} to /etc/environment"
}

# Check if variable already exists
if ! check_variable "$VAR_NAME"; then
    # Add to /etc/environment for persistence
    add_variable "$VAR_NAME" "$COMPILER_PATH"
fi

# Add to /etc/profile.d for immediate effect
PROFILE_SCRIPT="/etc/profile.d/compiler_path.sh"
echo "export ${VAR_NAME}=${COMPILER_PATH}" > "$PROFILE_SCRIPT"
chmod 644 "$PROFILE_SCRIPT"

# Update current session
export "${VAR_NAME}=${COMPILER_PATH}"

# Update environment for the user who ran sudo
SUDO_USER_HOME=$(eval echo ~${SUDO_USER})
BASHRC="${SUDO_USER_HOME}/.bashrc"

if [ -f "$BASHRC" ]; then
    # Remove any existing COMPILER_PATH export if it exists
    sed -i "/^export ${VAR_NAME}=/d" "$BASHRC"
    # Add the export
    echo "export ${VAR_NAME}=${COMPILER_PATH}" >> "$BASHRC"
fi

echo "Environment variable ${VAR_NAME} has been:"
echo "1. Added to /etc/environment for system-wide persistence"
echo "2. Added to /etc/profile.d/compiler_path.sh for all users"
echo "3. Exported in current session"
echo "4. Added to ${SUDO_USER}'s .bashrc"
echo ""
echo "The variable is now available in your current session:"
echo "${VAR_NAME}=${COMPILER_PATH}"
echo ""
echo "You can verify it by running: echo \$${VAR_NAME}"
