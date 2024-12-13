#!/bin/bash

# Exit on error
set -e

echo "Running GIMX-Pi tests..."

# Check if required files exist
check_files() {
    local files=(
        "/usr/local/bin/gimx-pi"
        "/etc/gimx-pi/config.xml"
        "/etc/udev/rules.d/99-gimx-pi.rules"
        "/etc/systemd/system/gimx-pi.service"
        "/usr/local/share/gimx-pi/icon.svg"
    )

    for file in "${files[@]}"; do
        if [ ! -f "$file" ]; then
            echo "ERROR: Missing file: $file"
            return 1
        fi
    done
    echo "All required files are present."
    return 0
}

# Check if services are running
check_services() {
    if ! systemctl is-active --quiet gimx-pi.service; then
        echo "ERROR: gimx-pi service is not running"
        return 1
    fi
    echo "GIMX-Pi service is running."
    return 0
}

# Check USB permissions
check_usb_permissions() {
    if ! groups | grep -q "plugdev"; then
        echo "ERROR: Current user is not in plugdev group"
        return 1
    fi
    echo "USB permissions are correctly set."
    return 0
}

# Check GTK dependencies
check_gtk() {
    if ! pkg-config --exists gtk+-3.0; then
        echo "ERROR: GTK3 development files are not installed"
        return 1
    fi
    echo "GTK3 dependencies are satisfied."
    return 0
}

# Run all checks
main() {
    local failed=0
    
    echo "1. Checking required files..."
    check_files || failed=1
    
    echo -e "\n2. Checking services..."
    check_services || failed=1
    
    echo -e "\n3. Checking USB permissions..."
    check_usb_permissions || failed=1
    
    echo -e "\n4. Checking GTK dependencies..."
    check_gtk || failed=1
    
    echo -e "\nTest Summary:"
    if [ $failed -eq 0 ]; then
        echo "✓ All tests passed successfully!"
        return 0
    else
        echo "✗ Some tests failed. Please check the errors above."
        return 1
    fi
}

# Run main function
main
