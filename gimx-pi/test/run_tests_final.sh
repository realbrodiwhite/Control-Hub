#!/bin/bash

# Exit on error
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}Building GIMX-Pi Tests...${NC}"

# Create build directory
mkdir -p build/test

# Compile test files
gcc -Wall -Wextra -I../src -o build/test/run_tests \
    run_tests.c \
    test_framework.c \
    test_script_gui.c \
    ../src/script_gui.c \
    ../src/script_lib.c \
    ../src/util.c \
    ../src/input.c \
    -DTEST_BUILD 2> build/test/build_errors.log

# Check for compilation errors
if [ $? -ne 0 ]; then
    echo -e "${RED}Build failed. Please check build_errors.log for details.${NC}"
    exit 1
fi

echo -e "${GREEN}Build successful${NC}"
echo -e "\n${BLUE}Running tests...${NC}\n"

# Run tests
./build/test/run_tests

# Store exit code
TEST_RESULT=$?

# Print final status
if [ $TEST_RESULT -eq 0 ]; then
    echo -e "\n${GREEN}All tests passed${NC}"
else
    echo -e "\n${RED}Some tests failed${NC}"
fi

exit $TEST_RESULT
