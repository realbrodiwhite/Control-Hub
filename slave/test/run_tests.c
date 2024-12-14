#include "test_framework.h"
#include "test_script_gui.h"
#include "../src/input.h"
#include "../src/util.h"

// Test result formatting
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_RESET   "\x1b[0m"

// Print test results
static void print_test_results(const test_result_t* result) {
    if (result->passed) {
        printf(COLOR_GREEN "PASS" COLOR_RESET " %s (%u us)\n", 
               result->name, result->duration_us);
    } else {
        printf(COLOR_RED "FAIL" COLOR_RESET " %s: %s\n", 
               result->name, result->message);
    }
}

// Print test summary
static void print_summary(uint32_t passed, uint32_t failed, uint32_t skipped) {
    printf("\nTest Summary:\n");
    printf(COLOR_GREEN "Passed: %u" COLOR_RESET "\n", passed);
    printf(COLOR_RED "Failed: %u" COLOR_RESET "\n", failed);
    printf(COLOR_YELLOW "Skipped: %u" COLOR_RESET "\n", skipped);
}

int main(void) {
    printf("Running ControlHub Slave Tests...\n\n");
    
    // Initialize test framework
    test_init();
    
    // Register all test categories
    register_gui_tests();
    register_usb_tests();
    register_hardware_tests();
    register_script_tests();
    register_performance_tests();
    
    // Run all tests
    test_run_all();
    
    // Print results
    const test_result_t* results;
    uint32_t count, passed, failed, skipped;
    test_get_results(&results, &count, &passed, &failed, &skipped);
    
    for (uint32_t i = 0; i < count; i++) {
        print_test_results(&results[i]);
    }
    
    print_summary(passed, failed, skipped);
    
    // Cleanup
    test_cleanup();
    
    return failed > 0 ? 1 : 0;
}
