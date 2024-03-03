#!/bin/bash

# Function to print usage
print_usage() {
    echo "Usage: $0 [outdir]"
    echo "  outdir: The location on the filesystem where the output files should be placed."
    echo "          If not specified, /tmp/aeld will be used."
}

# Default outdir if not provided
outdir="${1:-/tmp/aeld}"
outdir=$(realpath "$outdir")

# Create outdir if it doesn't exist
if [ ! -d "$outdir" ]; then
    mkdir -p "$outdir" || { echo "Failed to create directory $outdir"; exit 1; }
fi

# Change directory to outdir
cd "$outdir" || { echo "Failed to change directory to $outdir"; exit 1; }

# Build kernel
echo "Building kernel..."
if [ ! -d "linux" ]; then
    # Clone linux kernel source tree
    git clone --depth 1 https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git || { echo "Failed to clone Linux kernel source"; exit 1; }
fi

# Change directory to linux source
cd linux || { echo "Failed to change directory to linux source"; exit 1; }

# Checkout specific tag
echo "Checking out specific tag..."
git checkout <tag_name> || { echo "Failed to checkout specific tag"; exit 1; }

# Additional build steps here (e.g., make, make install, etc.)
# TODO: Add additional build steps as needed

echo "Kernel built successfully"

