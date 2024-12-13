#!/bin/bash

# Exit on error
set -e

echo "Building GIMX-Pi bare metal binary..."

# Check for required tools
if ! command -v arm-none-eabi-gcc &> /dev/null; then
    echo "Error: arm-none-eabi-gcc not found. Please install the ARM toolchain."
    exit 1
fi

if ! command -v cmake &> /dev/null; then
    echo "Error: cmake not found. Please install cmake."
    exit 1
fi

# Create and enter build directory
mkdir -p build
cd build

# Configure and build
cmake ..
make

echo "Build complete! kernel.img has been created."
echo ""
echo "To deploy:"
echo "1. Run prepare_sd.sh to prepare your SD card"
echo "2. Insert the SD card into your Raspberry Pi 3B"
echo "3. Power on the Raspberry Pi"
echo ""
echo "LED Status Indicators:"
echo "- Fast blink: System initializing"
echo "- Slow blink: Ready, waiting for controller"
echo "- Rapid blink: Controller connected"
