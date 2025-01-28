#!/usr/bin/env bash

# Exit immediately if a command exits with a non-zero status
set -e
# Set the IFS to only split on newlines and tabs
IFS=$'\n\t'
# Set the shell options
shopt -s nullglob
# Set the trap to cleanup on termination
trap cleanup EXIT

# Bin directory
LIB_ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BIN_DIR="${LIB_ROOT_DIR}/bin"

function error() {
  echo "Error: $1"
  exit 1
}

# Build with cargo
cargo build --release || error "Failed to build"

# Take the binary 

