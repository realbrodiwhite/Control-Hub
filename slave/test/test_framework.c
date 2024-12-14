#include "test_framework.h"
#include "test_script_gui.h"
#include "../src/util.h"

// Test registry
static struct {
    struct {
        const char* name;
        test_category_t category;
        test_type_t type;
        test_func_t func;
    } tests[MAX_TESTS];
    uint32_t test_count;
    
    struct {
        test_result_t results[MAX_RESULTS];
        uint32_t result_count;
        uint32_t passed_count;
        uint32_t failed_count;
        uint32_t skipped_count;
    } results;
    
    struct {
        uint32_t timeout_ms;
        uint64_t start_time;
        uint64_t end_time;
        const char* current_test;
        int test_failed;
        char failure_message[256];
    } state;
} test_framework = {0};

// Initialize test framework
void test_init(void) {
    test_framework.test_count = 0;
    test_framework.results.result_count = 0;
    test_framework.results.passed_count = 0;
    test_framework.results.failed_count = 0;
    test_framework.results.skipped_count = 0;
    test_framework.state.timeout_ms = 5000; // Default 5 second timeout
    test_framework.state.current_test = NULL;
}

// Add test to registry
void test_add(const char* name, test_category_t category, test_type_t type, test_func_t func) {
    if (test_framework.test_count >= MAX_TESTS) return;
    
    uint32_t index = test_framework.test_count++;
    test_framework.tests[index].name = name;
    test_framework.tests[index].category = category;
    test_framework.tests[index].type = type;
    test_framework.tests[index].func = func;
}

// Run single test
static void run_test(uint32_t test_index) {
    if (test_index >= test_framework.test_count) return;
    
    // Setup test
    test_framework.state.current_test = test_framework.tests[test_index].name;
    test_framework.state.test_failed = 0;
    test_framework.state.start_time = get_system_time();
    
    // Run test with timeout
    test_framework.tests[test_index].func();
    
    // Record result
    test_framework.state.end_time = get_system_time();
    uint32_t duration = (uint32_t)(test_framework.state.end_time - test_framework.state.start_time);
    
    if (test_framework.results.result_count < MAX_RESULTS) {
        test_result_t* result = &test_framework.results.results[test_framework.results.result_count++];
        result->name = test_framework.state.current_test;
        result->category = test_framework.tests[test_index].category;
        result->type = test_framework.tests[test_index].type;
        result->passed = !test_framework.state.test_failed;
        result->duration_us = duration;
        if (test_framework.state.test_failed) {
            str_copy(result->message, test_framework.state.failure_message, sizeof(result->message));
            test_framework.results.failed_count++;
        } else {
            test_framework.results.passed_count++;
        }
    }
}

// Run all tests
void test_run_all(void) {
    for (uint32_t i = 0; i < test_framework.test_count; i++) {
        run_test(i);
    }
}

// Run tests of specific category
void test_run_category(test_category_t category) {
    for (uint32_t i = 0; i < test_framework.test_count; i++) {
        if (test_framework.tests[i].category == category) {
            run_test(i);
        }
    }
}

// Run tests of specific type
void test_run_type(test_type_t type) {
    for (uint32_t i = 0; i < test_framework.test_count; i++) {
        if (test_framework.tests[i].type == type) {
            run_test(i);
        }
    }
}

// Assert condition
void test_assert(int condition, const char* message) {
    if (!condition) {
        test_framework.state.test_failed = 1;
        str_copy(test_framework.state.failure_message, message, sizeof(test_framework.state.failure_message));
    }
}

// Fail test
void test_fail(const char* message) {
    test_framework.state.test_failed = 1;
    str_copy(test_framework.state.failure_message, message, sizeof(test_framework.state.failure_message));
}

// Skip test
void test_skip(const char* message) {
    test_framework.results.skipped_count++;
    if (test_framework.results.result_count < MAX_RESULTS) {
        test_result_t* result = &test_framework.results.results[test_framework.results.result_count++];
        result->name = test_framework.state.current_test;
        result->passed = 1;
        result->duration_us = 0;
        str_copy(result->message, message, sizeof(result->message));
    }
}

// Set test timeout
void test_set_timeout(uint32_t timeout_ms) {
    test_framework.state.timeout_ms = timeout_ms;
}

// Get test results
void test_get_results(const test_result_t** results, uint32_t* count,
                     uint32_t* passed, uint32_t* failed, uint32_t* skipped) {
    if (results) *results = test_framework.results.results;
    if (count) *count = test_framework.results.result_count;
    if (passed) *passed = test_framework.results.passed_count;
    if (failed) *failed = test_framework.results.failed_count;
    if (skipped) *skipped = test_framework.results.skipped_count;
}

// Cleanup test framework
void test_cleanup(void) {
    test_framework.test_count = 0;
    test_framework.results.result_count = 0;
    test_framework.state.current_test = NULL;
}
