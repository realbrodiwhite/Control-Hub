#!/bin/bash

# Exit on error
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}Starting GIMX-Pi Bare Metal Test Suite${NC}"
echo "================================================"

# Build test binary
echo -e "\n${YELLOW}Building test binary...${NC}"
cd ..
mkdir -p build
cd build
cmake .. -DBUILD_TESTS=ON
make

# Prepare test environment
echo -e "\n${YELLOW}Preparing test environment...${NC}"
echo "- Checking USB devices"
lsusb
echo "- Checking HDMI connection"
tvservice -s
echo "- Checking CPU frequency"
vcgencmd measure_clock arm
echo "- Checking temperature"
vcgencmd measure_temp

# Run tests
echo -e "\n${YELLOW}Running tests...${NC}"

# Function to run test and check result
run_test() {
    local test_name=$1
    local test_cmd=$2
    echo -e "\n${BLUE}Running $test_name test...${NC}"
    if $test_cmd; then
        echo -e "${GREEN}✓ $test_name test passed${NC}"
        return 0
    else
        echo -e "${RED}✗ $test_name test failed${NC}"
        return 1
    }
}

# Latency Tests
echo -e "\n${YELLOW}=== Latency Tests ===${NC}"
run_test "Input Processing" "./test_gimx --test=latency"
run_test "USB Communication" "./test_gimx --test=usb_latency"
run_test "End-to-End" "./test_gimx --test=e2e_latency"

# Stability Tests
echo -e "\n${YELLOW}=== Stability Tests ===${NC}"
run_test "Long Running" "./test_gimx --test=stability --duration=300"
run_test "Error Recovery" "./test_gimx --test=error_recovery"
run_test "Hot Plug" "./test_gimx --test=hot_plug"

# Performance Tests
echo -e "\n${YELLOW}=== Performance Tests ===${NC}"
run_test "CPU Usage" "./test_gimx --test=cpu_usage"
run_test "Memory Usage" "./test_gimx --test=memory_usage"
run_test "Temperature" "./test_gimx --test=temperature"

# Feature Tests
echo -e "\n${YELLOW}=== Feature Tests ===${NC}"
run_test "LED Patterns" "./test_gimx --test=led_patterns"
run_test "USB Detection" "./test_gimx --test=usb_detection"
run_test "HDMI Detection" "./test_gimx --test=hdmi_detection"
run_test "Controller Features" "./test_gimx --test=controller_features"

# Stress Tests
echo -e "\n${YELLOW}=== Stress Tests ===${NC}"
run_test "Maximum Load" "./test_gimx --test=stress --duration=60"
run_test "Thermal Throttling" "./test_gimx --test=thermal"
run_test "Power Management" "./test_gimx --test=power"

# Performance Modes
echo -e "\n${YELLOW}=== Performance Mode Tests ===${NC}"
run_test "Fast Mode" "./test_gimx --test=mode_fast"
run_test "Accurate Mode" "./test_gimx --test=mode_accurate"
run_test "Normal Mode" "./test_gimx --test=mode_normal"

# Generate report
echo -e "\n${YELLOW}Generating test report...${NC}"
./test_gimx --generate-report > test_report.txt

# Display summary
echo -e "\n${YELLOW}=== Test Summary ===${NC}"
echo "Full report saved to: test_report.txt"
echo -e "\nLatency Statistics:"
grep "Latency" test_report.txt
echo -e "\nStability Statistics:"
grep "Stability" test_report.txt
echo -e "\nPerformance Statistics:"
grep "Performance" test_report.txt

# Check for warnings
warnings=$(grep "WARNING" test_report.txt || true)
if [ ! -z "$warnings" ]; then
    echo -e "\n${YELLOW}Warnings detected:${NC}"
    echo "$warnings"
fi

# Check for optimizations
echo -e "\n${YELLOW}Optimization Opportunities:${NC}"
./test_gimx --analyze-optimization

echo -e "\n${BLUE}Test suite completed${NC}"
echo "================================================"
