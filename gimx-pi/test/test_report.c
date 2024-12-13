#include "test_report.h"
#include "../src/status.h"

// Static buffer for report generation
static char report_buffer[4096];
static uint32_t buffer_pos = 0;
static report_config_t current_config;

// Simple string utilities for bare metal
static void append_str(const char* str) {
    while (*str && buffer_pos < sizeof(report_buffer) - 1) {
        report_buffer[buffer_pos++] = *str++;
    }
    report_buffer[buffer_pos] = '\0';
}

static void append_num(uint32_t num) {
    char temp[12];
    int pos = 0;
    
    // Convert number to string
    do {
        temp[pos++] = '0' + (num % 10);
        num /= 10;
    } while (num > 0);
    
    // Reverse and append
    while (pos > 0 && buffer_pos < sizeof(report_buffer) - 1) {
        report_buffer[buffer_pos++] = temp[--pos];
    }
    report_buffer[buffer_pos] = '\0';
}

static void append_float(float num, int decimals) {
    // Convert to fixed point
    int32_t fixed = (int32_t)(num * (decimals == 1 ? 10 : 100));
    
    // Integer part
    append_num(fixed / (decimals == 1 ? 10 : 100));
    append_str(".");
    
    // Decimal part
    fixed = fixed % (decimals == 1 ? 10 : 100);
    if (decimals == 2 && fixed < 10) {
        append_str("0");
    }
    append_num(fixed);
}

// Initialize report generator
void report_init(const report_config_t* config) {
    buffer_pos = 0;
    report_buffer[0] = '\0';
    if (config) {
        current_config = *config;
    }
}

// Generate performance grade
static char get_performance_grade(float percentage) {
    if (percentage >= GRADE_A_PLUS) return 'A';
    if (percentage >= GRADE_A) return 'A';
    if (percentage >= GRADE_B) return 'B';
    if (percentage >= GRADE_C) return 'C';
    if (percentage >= GRADE_D) return 'D';
    return 'F';
}

// Generate result code based on test results
uint32_t report_get_summary_code(const test_result_t* results) {
    uint32_t code = RESULT_PASS;
    
    if (results->latency_us > TEST_LATENCY_THRESHOLD_US) {
        code |= RESULT_FAIL_LATENCY;
    }
    if (results->max_temp > TEST_TEMPERATURE_THRESHOLD_C) {
        code |= RESULT_FAIL_THERMAL;
    }
    if (results->power_draw_mA > TEST_POWER_DRAW_THRESHOLD_MA) {
        code |= RESULT_FAIL_POWER;
    }
    if (results->failed > 0) {
        code |= RESULT_FAIL_STABILITY;
    }
    
    return code;
}

// Generate performance metrics report
void report_performance_metrics(const test_result_t* results) {
    append_str("Performance Metrics\n");
    append_str("==================\n");
    
    append_str("Input Latency: ");
    append_float((float)results->latency_us / 1000.0f, 2);
    append_str(" ms\n");
    
    append_str("Temperature: ");
    append_num(results->max_temp);
    append_str("C\n");
    
    append_str("Power Draw: ");
    append_float((float)results->power_draw_mA / 1000.0f, 2);
    append_str(" A\n");
    
    append_str("CPU Usage: ");
    append_float(results->cpu_usage, 1);
    append_str("%\n");
    
    append_str("Memory Usage: ");
    append_float(results->memory_usage, 1);
    append_str("%\n");
}

// Generate optimization suggestions
void report_optimization_suggestions(const test_result_t* results) {
    append_str("\nOptimization Suggestions\n");
    append_str("======================\n");
    
    if (results->latency_us > TEST_LATENCY_THRESHOLD_US) {
        append_str("- High latency detected: Enable fast mode\n");
    }
    
    if (results->max_temp > TEST_TEMPERATURE_THRESHOLD_C) {
        append_str("- High temperature: Check cooling\n");
    }
    
    if (results->power_draw_mA > TEST_POWER_DRAW_THRESHOLD_MA) {
        append_str("- High power draw: Enable power saving\n");
    }
    
    if (results->cpu_usage > TEST_CPU_USAGE_THRESHOLD) {
        append_str("- High CPU usage: Enable NEON/DMA\n");
    }
}

// Update LED status based on test results
void report_led_status(const test_result_t* results) {
    uint32_t code = report_get_summary_code(results);
    
    if (code == RESULT_PASS) {
        status_update(LED_STATE_READY);
    } else if (code & (RESULT_FAIL_LATENCY | RESULT_FAIL_STABILITY)) {
        status_update(LED_STATE_ERROR);
    } else if (code & (RESULT_FAIL_THERMAL | RESULT_FAIL_POWER)) {
        status_update(LED_STATE_INIT);
    } else {
        status_update(LED_STATE_ACTIVE);
    }
}

// Generate complete test report
void report_generate(const test_result_t* results) {
    // Reset buffer
    buffer_pos = 0;
    
    // Generate appropriate sections based on config
    if (current_config.type == REPORT_FULL || 
        current_config.type == REPORT_PERFORMANCE) {
        report_performance_metrics(results);
    }
    
    if (current_config.type == REPORT_FULL || 
        current_config.type == REPORT_DETAILED) {
        report_optimization_suggestions(results);
    }
    
    // Update LED status
    if (current_config.output == OUTPUT_LED) {
        report_led_status(results);
    }
    
    // Store report in output buffer if provided
    if (current_config.output == OUTPUT_MEMORY && 
        current_config.output_buffer && 
        current_config.buffer_size > 0) {
        uint32_t copy_size = buffer_pos < current_config.buffer_size ? 
                            buffer_pos : current_config.buffer_size - 1;
        for (uint32_t i = 0; i < copy_size; i++) {
            ((char*)current_config.output_buffer)[i] = report_buffer[i];
        }
        ((char*)current_config.output_buffer)[copy_size] = '\0';
    }
}
