#include "test_framework.h"
#include "hardware_types.h"

// Test configuration
#define TEST_DURATION_SHORT  5000    // 5 seconds
#define TEST_DURATION_LONG   30000   // 30 seconds
#define TEST_ITERATIONS     1000     // Number of test iterations
#define LATENCY_THRESHOLD   1000     // 1ms maximum latency
#define TEMP_THRESHOLD      85       // 85°C maximum temperature
#define POWER_THRESHOLD     2500     // 2.5A maximum current draw
#define MAX_RETRIES        3        // Maximum test retries
#define TEST_TIMEOUT       60000    // Test timeout in ms
#define RECOVERY_DELAY     1000     // Delay between retries in ms

// Test dependencies
typedef struct {
    test_category_t category;
    test_category_t depends_on[3];
    uint8_t num_dependencies;
} test_dependency_t;

static const test_dependency_t test_dependencies[] = {
    {TEST_CONTROLLER, {TEST_USB}, 1},
    {TEST_PERFORMANCE, {TEST_THERMAL, TEST_POWER}, 2},
    {TEST_STABILITY, {TEST_USB, TEST_CONTROLLER, TEST_PERFORMANCE}, 3}
};

static test_result_t current_results = {0};
static uint32_t test_start_time = 0;

// Error recovery function
static void recover_from_error(const char* error_msg) {
    status_set_error();
    __builtin_snprintf(current_results.last_error, sizeof(current_results.last_error),
                     "Recovery attempt: %s", error_msg);
    
    // Reset subsystems
    hardware_init();
    optimize_init();
    status_init();
    usb_init();
    ps5_init();
    
    delay_microseconds(RECOVERY_DELAY * 1000);
}

// Check test dependencies
static int check_dependencies(test_category_t category) {
    for (size_t i = 0; i < sizeof(test_dependencies)/sizeof(test_dependency_t); i++) {
        if (test_dependencies[i].category == category) {
            for (uint8_t j = 0; j < test_dependencies[i].num_dependencies; j++) {
                test_result_t dep_result = {0};
                test_run_category(test_dependencies[i].depends_on[j]);
                test_get_results(&dep_result);
                if (dep_result.failed > 0) {
                    return 0;
                }
            }
            break;
        }
    }
    return 1;
}

// Check for test timeout
static int check_timeout(void) {
    if (get_system_time() - test_start_time > TEST_TIMEOUT * 1000) {
        __builtin_snprintf(current_results.last_error, sizeof(current_results.last_error),
                         "Test timeout exceeded");
        return 1;
    }
    return 0;
}

// Initialize test framework
int test_init(void) {
    int success = 1;
    // Reset results and timing
    __builtin_memset(&current_results, 0, sizeof(test_result_t));
    test_start_time = get_system_time();
    
    // Initialize all subsystems with error checking
    if (!hardware_init()) {
        recover_from_error("Hardware init failed");
        success = 0;
    }
    if (!optimize_init()) {
        recover_from_error("Optimization init failed");
        success = 0;
    }
    if (!status_init()) {
        recover_from_error("Status init failed");
        success = 0;
    }
    if (!usb_init()) {
        recover_from_error("USB init failed");
        success = 0;
    }
    if (!ps5_init()) {
        recover_from_error("PS5 init failed");
        success = 0;
    }
    
    return success;
}

// Test input latency with enhanced retry and recovery mechanism
void test_input_latency(void) {
    ps5_state_t state;
    uint64_t start, end;
    uint32_t max_latency = 0;
    uint32_t retry_count = 0;
    int test_failed = 0;
    uint32_t consecutive_failures = 0;
    uint32_t total_samples = 0;
    float avg_latency = 0;
    
    status_update(LED_STATE_INIT);
    
    while (retry_count < MAX_RETRIES && !test_failed) {
        if (check_timeout()) {
            test_failed = 1;
            __builtin_snprintf(current_results.last_error, sizeof(current_results.last_error),
                           "Input latency test timed out after %u retries", retry_count);
            break;
        }
    
        for (int i = 0; i < TEST_ITERATIONS && !test_failed; i++) {
            if (check_timeout()) {
                test_failed = 1;
                break;
            }
            
            // Warm up the cache before measurement
            optimize_process_input(&state);
            delay_microseconds(1000);
            
            start = get_system_time();
            if (!optimize_process_input(&state)) {
                consecutive_failures++;
                if (consecutive_failures >= 3) {
                    recover_from_error("Multiple input processing failures");
                    retry_count++;
                    break;
                }
                continue;
            }
            consecutive_failures = 0;
            end = get_system_time();
            
            uint32_t latency = (uint32_t)(end - start);
            
            // Update running average
            total_samples++;
            avg_latency = avg_latency + ((float)latency - avg_latency) / total_samples;
            
            // Track maximum latency
            if (latency > max_latency) {
                max_latency = latency;
            }
            
            // Check against dynamic threshold
            float dynamic_threshold = avg_latency * 2.0f;
            if (dynamic_threshold < LATENCY_THRESHOLD) {
                dynamic_threshold = LATENCY_THRESHOLD;
            }
            
            if (latency > dynamic_threshold) {
                if (retry_count < MAX_RETRIES - 1) {
                    retry_count++;
                    recover_from_error("Latency threshold exceeded");
                    break;
                } else {
                    current_results.failed++;
                    __builtin_snprintf(current_results.last_error, sizeof(current_results.last_error),
                                   "Latency threshold exceeded after retries: %uus (avg: %.2f)", 
                                   latency, avg_latency);
                }
            } else {
                current_results.passed++;
            }
        }
        
        if (current_results.failed == 0) {
            break;
        }
    }
    
    current_results.latency_us = max_latency;
    current_results.retry_count = retry_count;
}

// Test USB stability
void test_usb_stability(void) {
    uint32_t disconnects = 0;
    uint64_t start = get_system_time();
    
    status_update(LED_STATE_PS5_WAIT);
    
    while (get_system_time() - start < TEST_DURATION_LONG * 1000) {
        if (!usb_detect_device(USB_DEVICE_PS5) || !usb_detect_device(USB_DEVICE_CONTROLLER)) {
            disconnects++;
            status_set_error();
            delay_microseconds(1000000); // Wait 1s before retry
        }
    }
    
    if (disconnects > 0) {
        current_results.failed++;
        __builtin_snprintf(current_results.last_error, sizeof(current_results.last_error),
                         "USB disconnects detected: %u", disconnects);
    } else {
        current_results.passed++;
    }
}

// Test thermal management with enhanced monitoring
void test_thermal_throttling(void) {
    performance_stats_t stats;
    uint32_t over_temp_count = 0;
    uint32_t consecutive_over_temp = 0;
    float avg_temp = 0;
    uint32_t samples = 0;
    uint32_t critical_temp_count = 0;
    
    status_update(LED_STATE_ACTIVE);
    
    // Run system at full load
    optimize_set_mode(PROCESS_MODE_FAST);
    uint64_t start = get_system_time();
    uint64_t last_check = start;
    
    while (get_system_time() - start < TEST_DURATION_LONG * 1000) {
        if (check_timeout()) {
            __builtin_snprintf(current_results.last_error, sizeof(current_results.last_error),
                           "Thermal test timed out");
            break;
        }
        
        optimize_get_stats(&stats);
        
        // Update running average
        samples++;
        avg_temp += (stats.temperature - avg_temp) / samples;
        
        // Check temperature thresholds
        if (stats.temperature > TEMP_THRESHOLD) {
            over_temp_count++;
            consecutive_over_temp++;
            
            // Critical temperature check (10°C over threshold)
            if (stats.temperature > TEMP_THRESHOLD + 10) {
                critical_temp_count++;
                if (critical_temp_count >= 3) {
                    current_results.failed++;
                    __builtin_snprintf(current_results.last_error, sizeof(current_results.last_error),
                                   "Critical temperature detected: %.1f°C", stats.temperature);
                    break;
                }
            }
            
            // Check for sustained high temperature
            if (consecutive_over_temp >= 5) {
                recover_from_error("Sustained high temperature");
                delay_microseconds(5000000); // 5s cooldown
                consecutive_over_temp = 0;
            }
        } else {
            consecutive_over_temp = 0;
            critical_temp_count = 0;
        }
        
        // Simulate full load with periodic checks
        ps5_state_t state;
        optimize_process_input(&state);
        
        // Check system health every second
        if (get_system_time() - last_check > 1000000) {
            if (!hardware_init() || !optimize_init()) {
                recover_from_error("System instability detected");
                break;
            }
            last_check = get_system_time();
        }
        
        delay_microseconds(1000);
    }
    
    // Evaluate test results
    float temp_threshold_percent = (float)over_temp_count / samples * 100;
    if (temp_threshold_percent > 20.0f) { // More than 20% of samples over threshold
        current_results.failed++;
        __builtin_snprintf(current_results.last_error, sizeof(current_results.last_error),
                         "Temperature threshold exceeded %.1f%% of time (avg: %.1f°C)",
                         temp_threshold_percent, avg_temp);
    } else {
        current_results.passed++;
    }
    
    current_results.max_temp = stats.temperature;
}

// Test power consumption with enhanced monitoring
void test_power_consumption(void) {
    performance_stats_t stats;
    uint32_t over_power_count = 0;
    uint32_t consecutive_over_power = 0;
    float avg_power = 0;
    uint32_t samples = 0;
    uint32_t mode_transitions = 0;
    
    // Test different power states with transition monitoring
    for (int mode = PROCESS_MODE_FAST; mode <= PROCESS_MODE_ACCURATE; mode++) {
        optimize_set_mode(mode);
        delay_microseconds(100000); // Allow system to stabilize after mode change
        
        uint64_t start = get_system_time();
        uint64_t last_check = start;
        float mode_avg_power = 0;
        uint32_t mode_samples = 0;
        
        while (get_system_time() - start < TEST_DURATION_SHORT * 1000) {
            if (check_timeout()) {
                __builtin_snprintf(current_results.last_error, sizeof(current_results.last_error),
                               "Power test timed out in mode %d", mode);
                break;
            }
            
            optimize_get_stats(&stats);
            float current_power = stats.cpu_usage * 25; // Approximate power from CPU usage
            
            // Update running averages
            samples++;
            mode_samples++;
            avg_power += (current_power - avg_power) / samples;
            mode_avg_power += (current_power - mode_avg_power) / mode_samples;
            
            // Check power thresholds
            if (current_power > POWER_THRESHOLD) {
                over_power_count++;
                consecutive_over_power++;
                
                // Check for sustained high power draw
                if (consecutive_over_power >= 10) {
                    recover_from_error("Sustained high power consumption");
                    mode_transitions++;
                    if (mode_transitions >= 3) {
                        __builtin_snprintf(current_results.last_error, sizeof(current_results.last_error),
                                       "Excessive power state transitions required");
                        break;
                    }
                    break; // Exit current mode early
                }
            } else {
                consecutive_over_power = 0;
            }
            
            // Check system stability every second
            if (get_system_time() - last_check > 1000000) {
                if (!optimize_init()) {
                    recover_from_error("Power instability detected");
                    break;
                }
                last_check = get_system_time();
            }
            
            delay_microseconds(1000);
        }
        
        // Log mode-specific results
        if (mode_avg_power > POWER_THRESHOLD) {
            __builtin_snprintf(current_results.last_error, sizeof(current_results.last_error),
                           "Mode %d average power (%.1fmA) exceeded threshold", 
                           mode, mode_avg_power);
        }
    }
    
    // Evaluate overall results
    float power_threshold_percent = (float)over_power_count / samples * 100;
    if (power_threshold_percent > 10.0f) { // More than 10% of samples over threshold
        current_results.failed++;
        __builtin_snprintf(current_results.last_error, sizeof(current_results.last_error),
                         "Power threshold exceeded %.1f%% of time (avg: %.1fmA)",
                         power_threshold_percent, avg_power);
    } else {
        current_results.passed++;
    }
    
    current_results.power_draw_mA = (uint32_t)avg_power;
}

// Run all tests with dependency and error handling
void test_run_all(void) {
    if (!test_init()) {
        __builtin_snprintf(current_results.last_error, sizeof(current_results.last_error),
                         "Test framework initialization failed");
        return;
    }
    
    // Run test suites in dependency order
    test_category_t test_order[] = {
        TEST_USB,           // Base hardware test
        TEST_THERMAL,       // System stability tests
        TEST_POWER,
        TEST_CONTROLLER,    // Device-specific tests
        TEST_LATENCY,
        TEST_STABILITY,     // Integration tests
        TEST_PERFORMANCE
    };
    
    for (size_t i = 0; i < sizeof(test_order)/sizeof(test_category_t); i++) {
        if (!check_dependencies(test_order[i])) {
            __builtin_snprintf(current_results.last_error, sizeof(current_results.last_error),
                             "Dependencies not met for test category %d", test_order[i]);
            continue;
        }
        
        // Run test with timeout protection
        uint64_t category_start = get_system_time();
        test_run_category(test_order[i]);
        
        if (get_system_time() - category_start > TEST_TIMEOUT * 1000) {
            __builtin_snprintf(current_results.last_error, sizeof(current_results.last_error),
                             "Test category %d timed out", test_order[i]);
            recover_from_error("Test category timeout");
        }
    }
    
    current_results.total_tests = current_results.passed + current_results.failed;
}

// Run a specific test category
void test_run_category(test_category_t category) {
    switch (category) {
        case TEST_LATENCY:
            test_input_latency();
            break;
        case TEST_USB:
            test_usb_stability();
            break;
        case TEST_THERMAL:
            test_thermal_throttling();
            break;
        case TEST_POWER:
            test_power_consumption();
            break;
        case TEST_STABILITY:
            test_stress_system(TEST_DURATION_LONG);
            break;
        case TEST_PERFORMANCE:
            test_benchmark();
            break;
        default:
            __builtin_snprintf(current_results.last_error, sizeof(current_results.last_error),
                             "Unknown test category: %d", category);
            current_results.failed++;
            break;
    }
}

// Get test results with validation
void test_get_results(test_result_t* results) {
    if (!results) {
        return;
    }
    
    // Validate results before returning
    if (current_results.total_tests != current_results.passed + current_results.failed) {
        current_results.total_tests = current_results.passed + current_results.failed;
    }
    
    // Ensure latency is within reasonable bounds
    if (current_results.latency_us > TEST_TIMEOUT * 1000) {
        current_results.latency_us = TEST_TIMEOUT * 1000;
    }
    
    // Cap temperature and power readings
    if (current_results.max_temp > 100) {  // Max realistic CPU temp
        current_results.max_temp = 100;
    }
    if (current_results.power_draw_mA > 3000) {  // Max realistic power draw
        current_results.power_draw_mA = 3000;
    }
    
    *results = current_results;
}

// Enhanced stress test system
void test_stress_system(uint32_t duration_ms) {
    status_update(LED_STATE_ACTIVE);
    optimize_set_mode(PROCESS_MODE_FAST);
    
    uint64_t start = get_system_time();
    uint64_t last_check = start;
    ps5_state_t state;
    ps5_output_t output = {0};
    
    // Stress test metrics
    uint32_t total_operations = 0;
    uint32_t failed_operations = 0;
    float avg_temp = 0;
    float avg_cpu = 0;
    uint32_t samples = 0;
    uint32_t recovery_count = 0;
    uint32_t consecutive_failures = 0;
    
    while (get_system_time() - start < duration_ms * 1000) {
        if (check_timeout()) {
            __builtin_snprintf(current_results.last_error, sizeof(current_results.last_error),
                           "Stress test timed out");
            break;
        }
        
        // Maximum load on all subsystems
        total_operations++;
        
        if (!optimize_process_input(&state)) {
            failed_operations++;
            consecutive_failures++;
            
            if (consecutive_failures >= 3) {
                recover_from_error("Multiple processing failures");
                recovery_count++;
                if (recovery_count >= MAX_RETRIES) {
                    current_results.failed++;
                    __builtin_snprintf(current_results.last_error, sizeof(current_results.last_error),
                                   "Excessive recovery attempts required");
                    break;
                }
                consecutive_failures = 0;
                continue;
            }
        } else {
            consecutive_failures = 0;
        }
        
        optimize_process_output(&output);
        ps5_set_led_color(255, 0, 0);
        ps5_set_haptic_feedback(255, 255);
        
        performance_stats_t stats;
        optimize_get_stats(&stats);
        
        // Update running averages
        samples++;
        avg_temp += (stats.temperature - avg_temp) / samples;
        avg_cpu += (stats.cpu_usage - avg_cpu) / samples;
        
        // Check system health every second
        if (get_system_time() - last_check > 1000000) {
            // Critical temperature check
            if (stats.temperature > TEMP_THRESHOLD + 10) {
                current_results.failed++;
                __builtin_snprintf(current_results.last_error, sizeof(current_results.last_error),
                               "Critical temperature during stress: %.1f°C", stats.temperature);
                break;
            }
            
            // Check for sustained high temperature
            if (stats.temperature > TEMP_THRESHOLD) {
                recover_from_error("High temperature during stress test");
                delay_microseconds(5000000); // 5s cooldown
            }
            
            // Check system stability
            if (!hardware_init() || !optimize_init()) {
                recover_from_error("System instability during stress");
                recovery_count++;
                if (recovery_count >= MAX_RETRIES) {
                    break;
                }
            }
            
            last_check = get_system_time();
        }
    }
    
    // Evaluate stress test results
    float error_rate = (float)failed_operations / total_operations * 100;
    if (error_rate > 5.0f || recovery_count >= MAX_RETRIES) {
        current_results.failed++;
        __builtin_snprintf(current_results.last_error, sizeof(current_results.last_error),
                       "Stress test failed: %.1f%% errors, %u recoveries needed\n"
                       "Avg temp: %.1f°C, Avg CPU: %.1f%%",
                       error_rate, recovery_count, avg_temp, avg_cpu);
    } else {
        current_results.passed++;
    }
    
    // Update test metrics
    current_results.max_temp = (uint32_t)avg_temp;
    current_results.cpu_usage = (uint32_t)avg_cpu;
}

// Enhanced benchmark system performance
void test_benchmark(void) {
    performance_stats_t stats;
    uint32_t frames = 0;
    uint32_t failed_frames = 0;
    float min_fps = UINT32_MAX;
    float max_fps = 0;
    float avg_fps = 0;
    float avg_cpu = 0;
    float avg_mem = 0;
    uint32_t samples = 0;
    uint32_t consecutive_failures = 0;
    
    uint64_t start = get_system_time();
    uint64_t last_fps_check = start;
    uint32_t frame_count = 0;
    
    // Warm up phase
    status_update(LED_STATE_INIT);
    for (int i = 0; i < 100; i++) {
        ps5_state_t state;
        optimize_process_input(&state);
        delay_microseconds(1000);
    }
    
    status_update(LED_STATE_ACTIVE);
    while (get_system_time() - start < TEST_DURATION_SHORT * 1000) {
        if (check_timeout()) {
            __builtin_snprintf(current_results.last_error, sizeof(current_results.last_error),
                           "Benchmark timed out");
            break;
        }
        
        ps5_state_t state;
        if (!optimize_process_input(&state)) {
            failed_frames++;
            consecutive_failures++;
            
            if (consecutive_failures >= 5) {
                recover_from_error("Multiple frame processing failures");
                if (consecutive_failures >= 10) {
                    current_results.failed++;
                    break;
                }
            }
            continue;
        }
        
        consecutive_failures = 0;
        frames++;
        frame_count++;
        
        // Calculate FPS every second
        uint64_t current = get_system_time();
        if (current - last_fps_check >= 1000000) {
            float fps = (float)frame_count * 1000000 / (current - last_fps_check);
            
            // Update FPS statistics
            if (fps < min_fps) min_fps = fps;
            if (fps > max_fps) max_fps = fps;
            
            samples++;
            avg_fps += (fps - avg_fps) / samples;
            
            // Get system stats
            optimize_get_stats(&stats);
            avg_cpu += (stats.cpu_usage - avg_cpu) / samples;
            avg_mem += (stats.memory_usage - avg_mem) / samples;
            
            // Check for performance degradation
            if (fps < avg_fps * 0.5f && samples > 10) {
                recover_from_error("Severe performance drop detected");
            }
            
            frame_count = 0;
            last_fps_check = current;
        }
    }
    
    // Evaluate benchmark results
    float error_rate = (float)failed_frames / (frames + failed_frames) * 100;
    float fps_stability = (max_fps - min_fps) / avg_fps * 100;
    
    if (error_rate > 5.0f || fps_stability > 50.0f) {
        current_results.failed++;
        __builtin_snprintf(current_results.last_error, sizeof(current_results.last_error),
                       "Benchmark failed:\n"
                       "Error rate: %.1f%%\n"
                       "FPS: %.1f avg (%.1f-%.1f, %.1f%% stability)\n"
                       "CPU: %.1f%%, Mem: %.1f%%",
                       error_rate, avg_fps, min_fps, max_fps, fps_stability,
                       avg_cpu, avg_mem);
    } else {
        current_results.passed++;
        __builtin_snprintf(current_results.last_error, sizeof(current_results.last_error),
                       "Benchmark passed:\n"
                       "FPS: %.1f avg (%.1f-%.1f)\n"
                       "CPU: %.1f%%, Mem: %.1f%%",
                       avg_fps, min_fps, max_fps, avg_cpu, avg_mem);
    }
    
    // Update test metrics
    current_results.cpu_usage = (uint32_t)avg_cpu;
    current_results.memory_usage = (uint32_t)avg_mem;
}
