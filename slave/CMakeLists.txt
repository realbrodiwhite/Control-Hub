cmake_minimum_required(VERSION 3.10)
project(controlhub_slave C)

# Option to build tests
option(BUILD_TESTS "Build test suite" OFF)

# Set ARM toolchain for main build
if(NOT BUILD_TESTS)
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
        src/hardware.c
        src/optimize.c
        src/ps5.c
        src/script.c
        src/script_gui.c
        src/script_lib.c
        src/status.c
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
else()
    # Test build configuration
    set(CMAKE_C_FLAGS "-Wall -Wextra -g -O0 -coverage")
    
    # Test executable
    add_executable(test_runner
        test/run_tests.c
        test/test_framework.c
        test/test_script_gui.c
        test/test_analysis.c
        src/script_gui.c
        src/script_lib.c
        src/optimize.c
        src/status.c
    )

    # Include directories
    target_include_directories(test_runner PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/test
    )

    # Link with required libraries for testing
    target_link_libraries(test_runner
        gcov
        pthread
    )

    # Enable testing
    enable_testing()
    add_test(NAME unit_tests COMMAND test_runner --type=unit)
    add_test(NAME integration_tests COMMAND test_runner --type=integration)
    add_test(NAME system_tests COMMAND test_runner --type=system)
    add_test(NAME gui_tests COMMAND test_runner --type=gui)
    add_test(NAME performance_tests COMMAND test_runner --type=performance)
endif()
