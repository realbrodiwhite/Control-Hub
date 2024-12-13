#!/bin/bash

# Exit on error
set -e

echo "Preparing SD card for GIMX-Pi bare metal..."

# Check if running with sudo
if [ "$EUID" -ne 0 ]; then 
    echo "Please run as root (sudo)"
    exit 1
fi

# Create temporary directory for firmware files
TEMP_DIR=$(mktemp -d)
cd "$TEMP_DIR"

echo "Downloading Raspberry Pi firmware files..."
wget https://github.com/raspberrypi/firmware/raw/master/boot/bootcode.bin
wget https://github.com/raspberrypi/firmware/raw/master/boot/start.elf
wget https://github.com/raspberrypi/firmware/raw/master/boot/fixup.dat

# Check if kernel.img exists
if [ ! -f "../build/kernel.img" ]; then
    echo "Error: kernel.img not found. Please build the project first."
    exit 1
fi

# Ask for SD card device
echo "Please enter your SD card device (e.g., /dev/sdb): "
read SD_DEVICE

if [ ! -b "$SD_DEVICE" ]; then
    echo "Error: Invalid block device"
    exit 1
fi

echo "WARNING: This will erase all data on $SD_DEVICE"
echo "Are you sure you want to continue? (y/n)"
read CONFIRM

if [ "$CONFIRM" != "y" ]; then
    echo "Aborted"
    exit 1
fi

echo "Formatting SD card..."
mkfs.vfat -F 32 "$SD_DEVICE"1

echo "Creating mount point..."
MOUNT_POINT=$(mktemp -d)
mount "$SD_DEVICE"1 "$MOUNT_POINT"

echo "Copying files to SD card..."
cp bootcode.bin start.elf fixup.dat "$MOUNT_POINT/"
cp ../build/kernel.img "$MOUNT_POINT/"

echo "Creating config.txt..."
cat > "$MOUNT_POINT/config.txt" << EOL
# GIMX-Pi bare metal configuration
kernel_old=1
disable_commandline_tags=1
enable_uart=1
EOL

echo "Syncing and unmounting..."
sync
umount "$MOUNT_POINT"

echo "Cleaning up..."
rm -rf "$TEMP_DIR" "$MOUNT_POINT"

echo "SD card preparation complete!"
echo "You can now insert the SD card into your Raspberry Pi 3B"
echo "The program will start automatically when power is applied"
