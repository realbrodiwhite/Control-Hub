#ifndef TEST_ANALYSIS_H
#define TEST_ANALYSIS_H

#include "test_framework.h"
#include "test_config.h"

// Analysis Categories
typedef enum {
    ANALYSIS_LATENCY,      // Input latency analysis
    ANALYSIS_STABILITY,    // System stability analysis
    ANALYSIS_THERMAL,      // Thermal performance analysis
    ANALYSIS_POWER,        // Power consumption analysis
    ANALYSIS_MEMORY,       // Memory usage analysis
    ANALYSIS_CPU,          // CPU utilization analysis
    ANALYSIS_USB,          // USB performance analysis
    ANALYSIS_FULL         // Complete system analysis
} analysis_type_t;

// Performance Trend
typedef enum {
    TREND_IMPROVING,      // Metrics improving over time
    TREND_STABLE,         // Metrics stable
    TREND_DEGRADING,      // Metrics degrading over time
    TREND_UNKNOWN        // Not enough data
} performance_trend_t;

// Statistical Data
typedef struct {
    float min;            // Minimum value
    float max;            // Maximum value
    float avg;            // Average value
    float std_dev;        // Standard deviation
    float percentile_95;  // 95th percentile
    performance_trend_t trend;
} stat_data_t;

// Performance Analysis Results
typedef struct {
    stat_data_t latency;          // Latency statistics
    stat_data_t temperature;      // Temperature statistics
    stat_data_t power;            // Power consumption statistics
    stat_data_t cpu_usage;        // CPU usage statistics
    stat_data_t memory_usage;     // Memory usage statistics
    stat_data_t usb_errors;       // USB error statistics
    uint32_t anomalies_detected;  // Number of anomalies
    uint32_t optimization_flags;  // Suggested optimizations
} analysis_result_t;

// Optimization Flags
#define OPT_SUGGEST_NEON      (1 << 0)  // Enable NEON optimization
#define OPT_SUGGEST_DMA       (1 << 1)  // Enable DMA transfers
#define OPT_SUGGEST_CACHE     (1 << 2)  // Enable cache optimization
#define OPT_SUGGEST_FREQ      (1 << 3)  // Adjust CPU frequency
#define OPT_SUGGEST_COOLING   (1 << 4)  // Improve cooling
#define OPT_SUGGEST_POWER     (1 << 5)  // Power optimization
#define OPT_SUGGEST_USB       (1 << 6)  // USB optimization
#define OPT_SUGGEST_MEMORY    (1 << 7)  // Memory optimization

// Function Prototypes
void analysis_init(void);
void analysis_process_results(const test_result_t* results);
void analysis_get_stats(analysis_result_t* analysis);
uint32_t analysis_get_optimization_suggestions(void);
performance_trend_t analysis_get_trend(analysis_type_t type);
float analysis_get_reliability_score(void);
float analysis_get_performance_score(void);
float analysis_get_efficiency_score(void);
void analysis_detect_anomalies(void);
void analysis_predict_failures(void);
void analysis_generate_recommendations(char* buffer, uint32_t size);

// Analysis Thresholds
#define ANOMALY_THRESHOLD     2.0f    // Standard deviations for anomaly
#define TREND_THRESHOLD       0.1f    // Minimum change for trend
#define RELIABILITY_THRESHOLD 0.95f   // 95% reliability target
#define PERFORMANCE_TARGET    0.90f   // 90% performance target
#define EFFICIENCY_TARGET     0.85f   // 85% efficiency target

#endif // TEST_ANALYSIS_H
