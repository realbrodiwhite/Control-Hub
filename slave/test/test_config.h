#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H

// Test Thresholds and Parameters
#define TEST_LATENCY_THRESHOLD_US        1000    // 1ms maximum input latency
#define TEST_FRAME_TIME_THRESHOLD_US     16667   // 60fps minimum (16.67ms)
#define TEST_CPU_USAGE_THRESHOLD         90      // 90% maximum CPU usage
#define TEST_MEMORY_USAGE_THRESHOLD      80      // 80% maximum memory usage
#define TEST_TEMPERATURE_THRESHOLD_C     80      // 80°C maximum temperature
#define TEST_POWER_DRAW_THRESHOLD_MA     2500    // 2.5A maximum current draw

// Test Durations
#define TEST_DURATION_QUICK_MS           5000    // 5 seconds
#define TEST_DURATION_NORMAL_MS          30000   // 30 seconds
#define TEST_DURATION_EXTENDED_MS        300000  // 5 minutes
#define TEST_DURATION_STRESS_MS          3600000 // 1 hour

// Test Iterations
#define TEST_ITERATIONS_QUICK            100     // Quick test iterations
#define TEST_ITERATIONS_NORMAL           1000    // Normal test iterations
#define TEST_ITERATIONS_EXTENDED         10000   // Extended test iterations

// Performance Test Parameters
#define PERF_TEST_BUFFER_SIZE           4096    // Test buffer size
#define PERF_TEST_DMA_THRESHOLD         512     // Minimum size for DMA
#define PERF_TEST_NEON_THRESHOLD        128     // Minimum size for NEON
#define PERF_TEST_CACHE_LINE_SIZE       64      // Cache line size

// USB Test Parameters
#define USB_TEST_PACKET_SIZE            64      // USB packet size
#define USB_TEST_PACKETS_PER_FRAME      8       // Packets per frame
#define USB_TEST_ERROR_THRESHOLD        0.001   // 0.1% maximum error rate

// Controller Test Parameters
#define CONTROLLER_TEST_INPUTS          16      // Number of inputs to test
#define CONTROLLER_TEST_REPEAT          10      // Input test repetitions
#define CONTROLLER_CALIBRATION_SAMPLES  100     // Calibration samples

// LED Test Patterns (microseconds)
#define LED_TEST_PATTERN_SHORT          100000  // 100ms
#define LED_TEST_PATTERN_MEDIUM         500000  // 500ms
#define LED_TEST_PATTERN_LONG           1000000 // 1s

// Error Recovery Parameters
#define ERROR_RECOVERY_ATTEMPTS         3       // Recovery attempts
#define ERROR_RECOVERY_DELAY_MS         1000    // Delay between attempts

// Test Categories
typedef enum {
    TEST_CAT_CRITICAL,      // Must pass for operation
    TEST_CAT_PERFORMANCE,   // Performance metrics
    TEST_CAT_STABILITY,     // Long-term stability
    TEST_CAT_FEATURES,      // Feature verification
    TEST_CAT_STRESS        // Stress testing
} test_category_t;

// Test Priority
typedef enum {
    TEST_PRIO_LOW,         // Optional tests
    TEST_PRIO_MEDIUM,      // Important tests
    TEST_PRIO_HIGH,        // Critical tests
    TEST_PRIO_MANDATORY    // Must-run tests
} test_priority_t;

// Test Configuration Structure
typedef struct {
    const char* name;           // Test name
    test_category_t category;   // Test category
    test_priority_t priority;   // Test priority
    uint32_t duration_ms;       // Test duration
    uint32_t iterations;        // Test iterations
    float threshold;            // Pass/fail threshold
    const char* description;    // Test description
} test_config_t;

// Standard Test Configurations
static const test_config_t TEST_CONFIGS[] = {
    {
        "input_latency",
        TEST_CAT_CRITICAL,
        TEST_PRIO_MANDATORY,
        TEST_DURATION_QUICK_MS,
        TEST_ITERATIONS_NORMAL,
        TEST_LATENCY_THRESHOLD_US,
        "Measures input processing latency"
    },
    {
        "stability",
        TEST_CAT_STABILITY,
        TEST_PRIO_HIGH,
        TEST_DURATION_EXTENDED_MS,
        TEST_ITERATIONS_EXTENDED,
        0.99f, // 99% uptime
        "Tests long-term system stability"
    },
    {
        "performance",
        TEST_CAT_PERFORMANCE,
        TEST_PRIO_HIGH,
        TEST_DURATION_NORMAL_MS,
        TEST_ITERATIONS_NORMAL,
        60.0f, // 60 FPS minimum
        "Measures system performance metrics"
    },
    {
        "thermal",
        TEST_CAT_CRITICAL,
        TEST_PRIO_MANDATORY,
        TEST_DURATION_NORMAL_MS,
        TEST_ITERATIONS_QUICK,
        TEST_TEMPERATURE_THRESHOLD_C,
        "Monitors system temperature under load"
    },
    {
        "power",
        TEST_CAT_CRITICAL,
        TEST_PRIO_MANDATORY,
        TEST_DURATION_NORMAL_MS,
        TEST_ITERATIONS_QUICK,
        TEST_POWER_DRAW_THRESHOLD_MA,
        "Monitors power consumption"
    },
    {
        "stress",
        TEST_CAT_STRESS,
        TEST_PRIO_MEDIUM,
        TEST_DURATION_STRESS_MS,
        TEST_ITERATIONS_EXTENDED,
        0.95f, // 95% pass rate
        "Full system stress test"
    }
};

#endif // TEST_CONFIG_H
