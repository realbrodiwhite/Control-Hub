#!/bin/bash

# Exit on error
set -e

echo "Installing GIMX-Pi for Raspberry Pi 3B..."

# Check if running on Raspberry Pi
if ! grep -q "Raspberry Pi" /proc/cpuinfo; then
    echo "This script must be run on a Raspberry Pi!"
    exit 1
fi

# Update package lists
echo "Updating package lists..."
apt-get update

# Install required dependencies
echo "Installing dependencies..."
apt-get install -y \
    build-essential \
    cmake \
    git \
    libusb-1.0-0-dev \
    libbluetooth-dev \
    libgtk-3-dev \
    libxml2-dev \
    libglib2.0-dev \
    libdbus-1-dev \
    libudev-dev \
    libncurses5-dev \
    libevdev-dev \
    pkg-config

# Create necessary directories
echo "Creating directories..."
mkdir -p /etc/gimx-pi
mkdir -p /usr/local/share/gimx-pi

# Install configuration files
echo "Installing configuration files..."
cp config/config.xml /etc/gimx-pi/
cp config/99-gimx-pi.rules /etc/udev/rules.d/
cp config/gimx-pi.service /etc/systemd/system/

# Set up udev rules
echo "Setting up udev rules..."
usermod -a -G plugdev $SUDO_USER
udevadm control --reload-rules
udevadm trigger

# Build GIMX-Pi
echo "Building GIMX-Pi..."
mkdir -p build
cd build
cmake ..
make -j$(nproc)
make install

# Configure auto-login and auto-start X server using raspi-config
echo "Configuring auto-login and GUI boot..."
raspi-config nonint do_boot_behaviour B4

# Enable rc.local if not already enabled
systemctl enable rc-local.service

# Add startup script to rc.local
echo "Configuring startup script..."
if ! grep -q "gimx-pi" /etc/rc.local; then
    sed -i '/^exit 0/i \
# Start GIMX-Pi\
su pi -c "startx /usr/local/bin/gimx-pi"\
' /etc/rc.local
fi

# Create desktop shortcut
echo "Creating desktop shortcut..."
cat > /usr/share/applications/gimx-pi.desktop << EOL
[Desktop Entry]
Name=GIMX-Pi
Comment=PS5 Controller Interface
Exec=/usr/local/bin/gimx-pi
Icon=/usr/local/share/gimx-pi/icon.png
Terminal=false
Type=Application
Categories=Game;Utility;
EOL

echo "Installation complete!"
echo "Please reboot your Raspberry Pi to ensure all changes take effect."
echo "After reboot, you can start GIMX-Pi from the desktop menu or run 'gimx-pi' in terminal."
