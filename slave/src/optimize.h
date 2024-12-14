#ifndef OPTIMIZE_H
#define OPTIMIZE_H

#include <stdint.h>
#include "ps5.h"
#include "hardware.h"

// Performance Optimization Flags
#define OPT_NEON_ENABLED      (1 << 0)
#define OPT_GPU_ENABLED       (1 << 1)
#define OPT_DMA_ENABLED       (1 << 2)
#define OPT_CACHE_ENABLED     (1 << 3)
#define OPT_LOW_LATENCY       (1 << 4)

// Input Processing Modes
typedef enum {
    PROCESS_MODE_SAFE,        // Safe mode with minimal features
    PROCESS_MODE_NORMAL,      // Standard processing
    PROCESS_MODE_FAST,        // Minimal processing for lowest latency
    PROCESS_MODE_ACCURATE     // Full processing with interpolation
} process_mode_t;

// Performance Statistics
typedef struct {
    // Latency metrics
    uint32_t input_latency_us;     // Input processing latency
    uint32_t output_latency_us;    // Output processing latency
    uint32_t total_latency_us;     // Total round-trip latency
    uint32_t min_latency_us;       // Minimum observed latency
    uint32_t max_latency_us;       // Maximum observed latency
    
    // Frame metrics
    uint32_t frames_processed;     // Number of frames processed
    uint32_t frames_dropped;       // Number of frames dropped
    uint32_t input_errors;         // Number of invalid inputs
    uint32_t output_errors;        // Number of invalid outputs
    
    // Buffer metrics
    uint32_t buffer_overruns;      // Number of buffer overruns
    uint32_t buffer_underruns;     // Number of buffer underruns
    uint32_t buffer_usage;         // Current buffer usage percentage
    
    // System metrics
    float cpu_usage;               // CPU usage percentage
    float memory_usage;            // Memory usage percentage
    uint32_t temperature;          // SoC temperature
    uint32_t voltage_mv;           // System voltage in millivolts
    
    // Recovery metrics
    uint32_t error_count;          // Total error count
    uint32_t recovery_attempts;    // Number of recovery attempts
    uint32_t last_error_time;      // Timestamp of last error
    uint32_t uptime_ms;            // System uptime in milliseconds
} performance_stats_t;

// Function Prototypes
void optimize_init(void);
void optimize_set_mode(process_mode_t mode);
int optimize_verify_mode(process_mode_t mode);
void optimize_enable_features(uint32_t features);
int optimize_verify_features(uint32_t features);
int optimize_verify_stability(void);
void optimize_disable_features(uint32_t features);
int optimize_process_input(ps5_state_t* state);
int optimize_process_output(const ps5_output_t* output);
void optimize_get_stats(performance_stats_t* stats);
void optimize_calibrate(void);
void optimize_tune_performance(void);

// Advanced Features
void optimize_set_input_buffer(uint32_t size_ms);
void optimize_set_output_buffer(uint32_t size_ms);
void optimize_set_priority(uint32_t priority);
void optimize_set_cpu_governor(const char* governor);
void optimize_set_memory_policy(uint32_t policy);
void optimize_lock_memory(void);
void optimize_prefetch_data(const void* addr, size_t size);

#endif // OPTIMIZE_H
