#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdint.h>

// Test Categories
typedef enum {
    TEST_LATENCY,
    TEST_STABILITY,
    TEST_THERMAL,
    TEST_POWER,
    TEST_USB,
    TEST_HDMI,
    TEST_CONTROLLER,
    TEST_PERFORMANCE
} test_category_t;

// Test Result
typedef struct {
    uint32_t total_tests;
    uint32_t passed;
    uint32_t failed;
    uint32_t latency_us;
    uint32_t max_temp;
    uint32_t power_draw_mA;
    uint32_t memory_usage;
    uint32_t cpu_usage;
    uint32_t retry_count;
    char last_error[256];
} test_result_t;

// Function return types for subsystem initialization
typedef int (*init_func_t)(void);

// Test Functions
int test_init(void);
void test_run_all(void);
void test_run_category(test_category_t category);
void test_get_results(test_result_t* results);
void test_stress_system(uint32_t duration_ms);
void test_benchmark(void);
void test_calibrate(void);

// Specific Test Suites
void test_input_latency(void);
void test_usb_stability(void);
void test_thermal_throttling(void);
void test_power_consumption(void);
void test_hdmi_sync(void);
void test_controller_features(void);
void test_performance_modes(void);
void test_error_recovery(void);

#endif // TEST_FRAMEWORK_H
