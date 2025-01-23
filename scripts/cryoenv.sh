#!/bin/bash

# Check if script is run with sudo
if [ "$EUID" -ne 0 ]; then
    echo "Please run with sudo"
    exit 1
fi

# Get the absolute path of the current directory
CURRENT_DIR=$(pwd)
COMPILER_PATH="${CURRENT_DIR}/bin/compiler"
ROOT_DIR_PATH="${CURRENT_DIR}"
VAR_NAME="CRYO_COMPILER"
ROOT_NAME="CRYO_ROOT"

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

# Handle both variables
for var in "${VAR_NAME}:${COMPILER_PATH}" "${ROOT_NAME}:${ROOT_DIR_PATH}"; do
    name="${var%%:*}"
    value="${var#*:}"
    
    if ! check_variable "$name"; then
        add_variable "$name" "$value"
    fi
    
    # Add to profile.d
    echo "export ${name}=${value}" >> "/etc/profile.d/compiler_path.sh"
    
    # Update current session
    export "${name}=${value}"
    
    # Update user's .bashrc
    SUDO_USER_HOME=$(eval echo ~${SUDO_USER})
    BASHRC="${SUDO_USER_HOME}/.bashrc"
    if [ -f "$BASHRC" ]; then
        sed -i "/^export ${name}=/d" "$BASHRC"
        echo "export ${name}=${value}" >> "$BASHRC"
    fi
done

chmod 644 "/etc/profile.d/compiler_path.sh"

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

echo "Environment variable ${VAR_NAME} & ${ROOT_NAME} has been:"
echo "1. Added to /etc/environment for system-wide persistence"
echo "2. Added to /etc/profile.d/compiler_path.sh for all users"
echo "3. Exported in current session"
echo "4. Added to ${SUDO_USER}'s .bashrc"
echo ""
echo "The variable is now available in your current session:"
echo "${VAR_NAME}=${COMPILER_PATH}"
echo "${ROOT_NAME}=${ROOT_DIR_PATH}"
echo ""
echo "You can verify it by running: echo \$${VAR_NAME} or echo \$${ROOT_NAME}"
