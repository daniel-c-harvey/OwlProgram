#!/bin/bash

# Get the directory where this script is located
SCRIPT_DIR=$(dirname "${BASH_SOURCE[0]}")
DIR_NAME=$(basename "$SCRIPT_DIR")
echo "Script directory: $DIR_NAME"

# Change to the script's directory
cd "$SCRIPT_DIR"

# Create ../PatchSource/ directory if it doesn't exist
echo "Creating ../PatchSource/ directory..."
mkdir -p ../PatchSource/

# Copy all files and subdirectories to ../PatchSource/ with overwrite
echo "Copying all files from $DIR_NAME to ../PatchSource/ directory..."
cp -rf * ../PatchSource/ 2>/dev/null || true

# Run make command
echo "Running make clean web PATCHNAME=$DIR_NAME ..."
cd ..
make clean web PATCHNAME="$DIR_NAME"
