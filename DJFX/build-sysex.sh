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

# Copy all files and subdirectories to ../PatchSource/ only if newer or missing
echo "Copying updated files from $DIR_NAME to ../PatchSource/ directory..."
cp -rf * ../PatchSource/ 2>/dev/null || true

# Run make command
cd ..
make sysex PATCHNAME="$DIR_NAME" PLATFORM=OWL1
