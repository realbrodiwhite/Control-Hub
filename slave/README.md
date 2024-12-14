# ControlHub Slave

A bare metal implementation of ControlHub Slave designed to run directly on Raspberry Pi 3B hardware without an operating system.

## Requirements

- Raspberry Pi 3B
- SD Card (minimum 8GB recommended)
- HDMI cable and display (separate input from PS5)
- PS5 console (connected via USB)
- PS5 controller (connected via USB)
- Power supply (2.5A recommended)

## Connections

1. HDMI:
   - Connect Raspberry Pi to display via HDMI
   - Use a different HDMI input from the PS5

2. USB:
   - Port 1: Connect PS5 console
   - Port 2: Connect PS5 controller

3. Power:
   - Connect 2.5A power supply to Raspberry Pi

## LED Status Patterns

The onboard LED indicates system status through different blink patterns:

1. Single Blink (Fast)
   - System initializing
   - Firmware boot in progress

2. Double Blink
   - Waiting for HDMI connection
   - Ensure display is connected and powered on

3. Triple Blink
   - Waiting for PS5 connection
   - Check USB connection to PS5 console

4. Quad Blink
   - Waiting for controller connection
   - Check USB connection to PS5 controller

5. Steady On
   - All devices connected
   - System ready

6. Pulsing
   - System active
   - Processing controller input

7. Rapid Blink
   - Error state
   - Check all connections

## Building

1. Install the ARM cross-compiler toolchain:
```bash
# On Ubuntu/Debian
sudo apt-get install gcc-arm-none-eabi
# On macOS
brew install arm-none-eabi-gcc
# On Windows
# Download and install from ARM website
```

2. Build the bare metal binary:
```bash
chmod +x build.sh
./build.sh
```

This will create `build/kernel.img` which is the bare metal binary.

## Deployment

1. Prepare the SD card:
```bash
chmod +x prepare_sd.sh
sudo ./prepare_sd.sh
```

2. Insert the SD card into your Raspberry Pi 3B

3. Make connections:
   - Connect HDMI to display
   - Connect PS5 to USB Port 1
   - Connect controller to USB Port 2
   - Connect power last

The program will start automatically when power is applied.

## Troubleshooting

1. LED stays in single blink:
   - Check SD card is properly formatted
   - Verify kernel.img is present
   - Try reflashing SD card

2. LED stays in double blink:
   - Check HDMI connection
   - Ensure display is powered on
   - Try different HDMI input

3. LED stays in triple blink:
   - Check USB connection to PS5
   - Verify PS5 is powered on
   - Try different USB cable

4. LED stays in quad blink:
   - Check controller USB connection
   - Try different USB cable
   - Try different controller

5. Rapid blinking:
   - Check all connections
   - Power cycle all devices
   - Try reconnecting in this order:
     1. HDMI
     2. PS5
     3. Controller
     4. Power to Raspberry Pi

## Contributing

Contributions welcome! Please read the contributing guidelines first.

## License

This project is licensed under the GPL v3 License - see the LICENSE file for details.

## Acknowledgments

- Raspberry Pi Foundation for hardware documentation
- ARM for architecture reference
- Sony for PS5 hardware specifications
