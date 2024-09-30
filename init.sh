#!/usr/bin/env bash

# Exit immediately if a command exits with a non-zero status
set -e

# Treat unset variables as an error when substituting
set -u

# Exit if any command in a pipeline fails
set -o pipefail

# Set the IFS to only split on newlines and tabs
IFS=$'\n\t'

# Function to check if script is run as root
check_root() {
    if [[ $EUID -ne 0 ]]; then
        echo "This script must be run as root" >&2
        exit 1
    fi
}

# Function to set compiler environment variables
set_compiler_env_var() {
    # Use the directory of the script as the root of the project
    local ROOT
    ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

    # Set the compiler executable
    CRYO_COMPILER="$ROOT/src/bin/main"

    # Set the libs compiler executable
    CRYO_LIBS="$ROOT/cryo/"

    # Create a temporary file with export commands
    local temp_file="/tmp/cryo_env_vars.sh"
    echo "export CRYO_COMPILER=\"$CRYO_COMPILER\"" > "$temp_file"
    echo "export CRYO_LIBS=\"$CRYO_LIBS\"" >> "$temp_file"

    # Make the temp file readable by the original user
    chown "$SUDO_USER:$SUDO_USER" "$temp_file"
    chmod 644 "$temp_file"

    # Output the commands for the user to see
    echo "Setting the following environment variables:"
    cat "$temp_file"

    # Source the temp file in the user's shell
    echo "source $temp_file" >> "/home/$SUDO_USER/.bashrc"
    
    echo "Environment variables have been set and will be available in new shell sessions."
    echo "To apply them to your current session, please run: source ~/.bashrc"
}

# Main function
main() {
    check_root
    set_compiler_env_var
}

# Run the main function
main
