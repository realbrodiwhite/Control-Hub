#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdint.h>

// Define maximum limits for tests and results
#define MAX_TESTS 256
#define MAX_RESULTS 1024

// Test Categories
typedef enum {
    TEST_LATENCY,      // Input latency analysis
    TEST_STABILITY,    // System stability
    TEST_THERMAL,      // Thermal management
    TEST_POWER,        // Power management
    TEST_USB,          // USB connectivity
    TEST_GUI,          // GUI functionality
    TEST_SCRIPTS,      // Script management
    TEST_CATALOG       // Script catalog
} test_category_t;

// Test Types
typedef enum {
    TEST_TYPE_UNIT,        // Unit tests
    TEST_TYPE_INTEGRATION, // Integration tests
    TEST_TYPE_SYSTEM,      // System tests
    TEST_TYPE_GUI,         // GUI tests
    TEST_TYPE_PERFORMANCE  // Performance tests
} test_type_t;

// Test Result
typedef struct {
    const char* name;
    test_category_t category;
    test_type_t type;
    int passed;
    uint32_t duration_us;
    char message[256];
} test_result_t;

// Test Function Type
typedef void (*test_func_t)(void);

// Test Framework Functions
void test_init(void);
void test_cleanup(void);
void test_run_all(void);
void test_run_category(test_category_t category);
void test_run_type(test_type_t type);
void test_add(const char* name, test_category_t category, test_type_t type, test_func_t func);
void test_assert(int condition, const char* message);
void test_fail(const char* message);
void test_skip(const char* message);
void test_set_timeout(uint32_t timeout_ms);

// Get test results
void test_get_results(const test_result_t** results, uint32_t* count,
                     uint32_t* passed, uint32_t* failed, uint32_t* skipped);

// Test Macros
#define TEST_ASSERT(condition) test_assert((condition), #condition)
#define TEST_FAIL(message) test_fail(message)
#define TEST_SKIP(message) test_skip(message)

// Test Registration Macro
#define TEST_ADD(func) test_add(#func, TEST_GUI, TEST_TYPE_GUI, func)

// GUI Test Helpers
void test_gui_init(void);
void test_gui_cleanup(void);
void test_gui_simulate_input(uint32_t buttons);
void test_gui_verify_state(gui_state_t expected_state);
void test_gui_verify_selection(uint32_t expected_item);
void test_gui_verify_message(const char* expected_message);
void test_gui_verify_progress(const char* operation, uint32_t progress);

// Script Test Helpers
void test_script_init(void);
void test_script_cleanup(void);
void test_script_load(const char* path);
void test_script_verify_loaded(const char* name);
void test_script_verify_active(const char* name);
void test_script_verify_output(const char* expected_output);

// Catalog Test Helpers
void test_catalog_init(void);
void test_catalog_cleanup(void);
void test_catalog_connect(void);
void test_catalog_verify_connection(void);
void test_catalog_verify_script_exists(const char* name);
void test_catalog_verify_download(const char* name);

#endif // TEST_FRAMEWORK_H
