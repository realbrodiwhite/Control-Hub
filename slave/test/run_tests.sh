#!/bin/bash

# Exit on error
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}Building ControlHub Slave Tests...${NC}"

# Create build directory
mkdir -p build
cd build

# Configure and build tests
cmake .. -DBUILD_TESTS=ON
make

# Run different test categories
echo -e "\n${BLUE}Running Unit Tests...${NC}"
./test_runner --type=unit

echo -e "\n${BLUE}Running Integration Tests...${NC}"
./test_runner --type=integration

echo -e "\n${BLUE}Running System Tests...${NC}"
./test_runner --type=system

echo -e "\n${BLUE}Running GUI Tests...${NC}"
./test_runner --type=gui

echo -e "\n${BLUE}Running Performance Tests...${NC}"
./test_runner --type=performance

# Check if any test category failed
if [ $? -eq 0 ]; then
    echo -e "\n${GREEN}All tests passed successfully!${NC}"
    exit 0
else
    echo -e "\n${RED}Some tests failed. Check the logs above for details.${NC}"
    exit 1
fi
