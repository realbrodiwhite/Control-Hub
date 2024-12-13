cmake_minimum_required(VERSION 3.10)
project(gimx_pi_bare_metal C)

# Set ARM toolchain
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)

# Compiler flags for bare metal
set(CMAKE_C_FLAGS "\
    -mcpu=cortex-a53 \
    -fpic \
    -ffreestanding \
    -nostdlib \
    -nostartfiles \
    -Wall \
    -Wextra \
    -O2 \
    -D__BARE_METAL__ \
")

# Create bare metal executable
add_executable(kernel.elf 
    src/main.c
    src/usb.c
)

# Link with our custom linker script
target_link_options(kernel.elf PRIVATE
    -T${CMAKE_SOURCE_DIR}/src/rpi3b.ld
    -Wl,--build-id=none
)

# Custom command to create binary
add_custom_command(
    TARGET kernel.elf POST_BUILD
    COMMAND arm-none-eabi-objcopy -O binary kernel.elf kernel.img
    COMMAND ${CMAKE_COMMAND} -E copy kernel.img ${CMAKE_BINARY_DIR}/
    COMMENT "Creating bare metal binary"
)

# Install targets
install(FILES
    ${CMAKE_BINARY_DIR}/kernel.img
    DESTINATION .
)
