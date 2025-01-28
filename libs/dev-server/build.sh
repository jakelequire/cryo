#!/usr/bin/env bash

# Exit immediately if a command exits with a non-zero status
set -e
# Set the IFS to only split on newlines and tabs
IFS=$'\n\t'
# Set the shell options
shopt -s nullglob
# Set the trap to cleanup on termination
trap EXIT

# Bin directory
LIB_ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BIN_DIR="${LIB_ROOT_DIR}/bin"

# Create the bin directory if it doesn't exist
if [ ! -d "${BIN_DIR}" ]; then
  mkdir -p "${BIN_DIR}"
fi

function error() {
  echo "Error: $1"
  exit 1
}

CARGO_ARGS=""

# Build with cargo
cargo build --release $CARGO_ARGS || error "Failed to build the dev-server library"

# Take the binary and move it to the ./bin directory (no rename)
cp target/release/dev-server "${BIN_DIR}/dev-server"

