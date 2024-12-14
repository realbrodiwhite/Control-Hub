#ifndef TEST_REPORT_H
#define TEST_REPORT_H

#include "test_framework.h"
#include "test_config.h"

// Report Types
typedef enum {
    REPORT_SUMMARY,     // Basic pass/fail summary
    REPORT_DETAILED,    // Detailed test results
    REPORT_PERFORMANCE, // Performance metrics
    REPORT_FULL        // Complete report with all sections
} report_type_t;

// Report Format
typedef enum {
    FORMAT_PLAIN,      // Plain text
    FORMAT_COLOR,      // Text with ANSI colors
    FORMAT_MINIMAL     // Minimal output for embedded display
} report_format_t;

// Report Destination
typedef enum {
    OUTPUT_UART,       // Output to UART
    OUTPUT_LED,        // Output via LED patterns
    OUTPUT_MEMORY      // Store in memory buffer
} report_output_t;

// Report Configuration
typedef struct {
    report_type_t type;
    report_format_t format;
    report_output_t output;
    void* output_buffer;
    uint32_t buffer_size;
} report_config_t;

// Function Prototypes
void report_init(const report_config_t* config);
void report_generate(const test_result_t* results);
void report_performance_metrics(const test_result_t* results);
void report_optimization_suggestions(const test_result_t* results);
void report_led_status(const test_result_t* results);
uint32_t report_get_summary_code(const test_result_t* results);

// LED Pattern Codes for Test Results
#define LED_PATTERN_ALL_PASS    0x01    // Steady on
#define LED_PATTERN_PERF_PASS   0x02    // Double blink
#define LED_PATTERN_PARTIAL     0x03    // Triple blink
#define LED_PATTERN_FAIL        0x04    // Rapid blink
#define LED_PATTERN_ERROR       0x05    // SOS pattern

// Performance Grade Thresholds (percentage)
#define GRADE_A_PLUS   95
#define GRADE_A        90
#define GRADE_B        80
#define GRADE_C        70
#define GRADE_D        60
#define GRADE_F        0

// Result Codes
#define RESULT_PASS              0x00
#define RESULT_FAIL_LATENCY      0x01
#define RESULT_FAIL_STABILITY    0x02
#define RESULT_FAIL_THERMAL      0x04
#define RESULT_FAIL_POWER        0x08
#define RESULT_FAIL_FEATURES     0x10
#define RESULT_ERROR_HARDWARE    0x20
#define RESULT_ERROR_USB         0x40
#define RESULT_ERROR_SYSTEM      0x80

#endif // TEST_REPORT_H
