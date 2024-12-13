#include "optimize.h"
#include "status.h"

// Performance tuning parameters
#define MIN_BUFFER_SIZE_MS    1
#define MAX_BUFFER_SIZE_MS    32
#define DEFAULT_BUFFER_SIZE_MS 4

#define CPU_FREQ_MIN         600000000   // 600 MHz
#define CPU_FREQ_MAX         1200000000  // 1.2 GHz
#define CPU_FREQ_TURBO       1400000000  // 1.4 GHz

// Memory-mapped CPU control registers
#define CPU_CONTROL_BASE     0x3F100000
#define CPU_FREQ_REG        ((volatile uint32_t*)(CPU_CONTROL_BASE + 0x08))
#define CPU_THROTTLE_REG    ((volatile uint32_t*)(CPU_CONTROL_BASE + 0x0C))
#define CPU_TEMP_REG        ((volatile uint32_t*)(CPU_CONTROL_BASE + 0x10))

// Static configuration
static struct {
    process_mode_t mode;
    uint32_t features;
    uint32_t input_buffer_ms;
    uint32_t output_buffer_ms;
    uint32_t cpu_freq;
    performance_stats_t stats;
} config = {
    .mode = PROCESS_MODE_NORMAL,
    .features = OPT_NEON_ENABLED | OPT_DMA_ENABLED | OPT_CACHE_ENABLED,
    .input_buffer_ms = DEFAULT_BUFFER_SIZE_MS,
    .output_buffer_ms = DEFAULT_BUFFER_SIZE_MS,
    .cpu_freq = CPU_FREQ_MAX
};

// Initialize optimization subsystem
int optimize_init(void) {
    // Initialize hardware acceleration
    hardware_init();
    
    // Enable default features
    if (config.features & OPT_NEON_ENABLED) {
        enable_neon();
    }
    if (config.features & OPT_GPU_ENABLED) {
        enable_gpu();
    }
    if (config.features & OPT_DMA_ENABLED) {
        enable_dma();
    }
    
    // Set initial CPU frequency
    *CPU_FREQ_REG = config.cpu_freq;
    
    // Lock memory to prevent paging
    optimize_lock_memory();
    
    return 1; // Return success
}

// Set processing mode with verification
void optimize_set_mode(process_mode_t mode) {
    config.mode = mode;
    
    switch (mode) {
        case PROCESS_MODE_SAFE:
            // Safe mode with minimal features
            config.input_buffer_ms = MAX_BUFFER_SIZE_MS;
            config.output_buffer_ms = MAX_BUFFER_SIZE_MS;
            config.cpu_freq = CPU_FREQ_MIN;
            config.features &= ~(OPT_GPU_ENABLED | OPT_DMA_ENABLED); // Disable advanced features
            *CPU_FREQ_REG = config.cpu_freq;
            break;
            
        case PROCESS_MODE_FAST:
            // Minimize latency
            config.input_buffer_ms = MIN_BUFFER_SIZE_MS;
            config.output_buffer_ms = MIN_BUFFER_SIZE_MS;
            config.cpu_freq = CPU_FREQ_TURBO;
            *CPU_FREQ_REG = config.cpu_freq;
            break;
            
        case PROCESS_MODE_ACCURATE:
            // Maximize accuracy
            config.input_buffer_ms = MAX_BUFFER_SIZE_MS;
            config.output_buffer_ms = DEFAULT_BUFFER_SIZE_MS;
            config.cpu_freq = CPU_FREQ_MAX;
            *CPU_FREQ_REG = config.cpu_freq;
            break;
            
        default:
            // Balanced mode
            config.input_buffer_ms = DEFAULT_BUFFER_SIZE_MS;
            config.output_buffer_ms = DEFAULT_BUFFER_SIZE_MS;
            config.cpu_freq = CPU_FREQ_MAX;
            *CPU_FREQ_REG = config.cpu_freq;
            break;
    }
}

// Input validation bounds
#define MAX_ANALOG_VALUE 255
#define MIN_ANALOG_VALUE 0
#define MAX_TRIGGER_VALUE 255
#define MIN_TRIGGER_VALUE 0
#define MAX_GYRO_VALUE 32767
#define MIN_GYRO_VALUE -32768
#define MAX_TOUCH_X 1920
#define MAX_TOUCH_Y 1080

// Validate input state
static int validate_input_state(ps5_state_t* state) {
    if (!state) return 0;
    
    // Validate analog sticks
    if (state->sticks.lx < MIN_ANALOG_VALUE || state->sticks.lx > MAX_ANALOG_VALUE ||
        state->sticks.ly < MIN_ANALOG_VALUE || state->sticks.ly > MAX_ANALOG_VALUE ||
        state->sticks.rx < MIN_ANALOG_VALUE || state->sticks.rx > MAX_ANALOG_VALUE ||
        state->sticks.ry < MIN_ANALOG_VALUE || state->sticks.ry > MAX_ANALOG_VALUE) {
        return 0;
    }
    
    // Validate triggers
    if (state->triggers.l2 < MIN_TRIGGER_VALUE || state->triggers.l2 > MAX_TRIGGER_VALUE ||
        state->triggers.r2 < MIN_TRIGGER_VALUE || state->triggers.r2 > MAX_TRIGGER_VALUE) {
        return 0;
    }
    
    // Validate motion sensors
    if (state->motion.accel_x < MIN_GYRO_VALUE || state->motion.accel_x > MAX_GYRO_VALUE ||
        state->motion.accel_y < MIN_GYRO_VALUE || state->motion.accel_y > MAX_GYRO_VALUE ||
        state->motion.accel_z < MIN_GYRO_VALUE || state->motion.accel_z > MAX_GYRO_VALUE ||
        state->motion.gyro_x < MIN_GYRO_VALUE || state->motion.gyro_x > MAX_GYRO_VALUE ||
        state->motion.gyro_y < MIN_GYRO_VALUE || state->motion.gyro_y > MAX_GYRO_VALUE ||
        state->motion.gyro_z < MIN_GYRO_VALUE || state->motion.gyro_z > MAX_GYRO_VALUE) {
        return 0;
    }
    
    // Validate touch points
    for (int i = 0; i < 2; i++) {
        if (state->touch[i].active) {
            if (state->touch[i].x > MAX_TOUCH_X || state->touch[i].y > MAX_TOUCH_Y) {
                return 0;
            }
        }
    }
    
    // Validate battery level and temperature
    if (state->battery_level > 100 || state->temperature > 100) {
        return 0;
    }
    
    return 1;
}

// Process input with optimizations
int optimize_process_input(ps5_state_t* state) {
    uint64_t start_time = get_system_time();
    int result = 0;
    
    // Input validation
    if (!validate_input_state(state)) {
        config.stats.input_errors++;
        config.stats.frames_dropped++;
        config.stats.error_count++;
        return 0;
    }
    
    // Prefetch next input buffer
    if (config.features & OPT_CACHE_ENABLED) {
        optimize_prefetch_data(state, sizeof(ps5_state_t));
    }
    
    // Process input based on mode
    switch (config.mode) {
        case PROCESS_MODE_SAFE:
            // Basic processing with bounds checking
            if (validate_input_state(state)) {
                result = ps5_process_input(state);
            }
            break;
            
        case PROCESS_MODE_FAST:
            // Direct processing, no buffering
            result = ps5_process_input(state);
            break;
            
        case PROCESS_MODE_ACCURATE:
            // Use NEON for input smoothing
            if (config.features & OPT_NEON_ENABLED) {
                static ps5_state_t prev_state;
                ps5_process_input(state);
                neon_process_input(state, &prev_state, sizeof(ps5_state_t));
                prev_state = *state;
                result = 1;
            } else {
                result = ps5_process_input(state);
            }
            break;
            
        default:
            // Standard processing
            result = ps5_process_input(state);
            break;
    }
    
    // Update statistics
    config.stats.input_latency_us = (uint32_t)(get_system_time() - start_time);
    if (result) {
        config.stats.frames_processed++;
    } else {
        config.stats.frames_dropped++;
    }
    
    return result;
}

// Output validation bounds
#define MAX_LED_VALUE 255
#define MAX_HAPTIC_VALUE 255
#define MAX_TRIGGER_FORCE 255
#define MAX_VOLUME 100

// Validate output state
static int validate_output_state(const ps5_output_t* output) {
    if (!output) return 0;
    
    // Validate LED values
    if (output->led_r > MAX_LED_VALUE || 
        output->led_g > MAX_LED_VALUE || 
        output->led_b > MAX_LED_VALUE) {
        return 0;
    }
    
    // Validate haptic values
    if (output->haptic_right_intensity > MAX_HAPTIC_VALUE || 
        output->haptic_left_intensity > MAX_HAPTIC_VALUE) {
        return 0;
    }
    
    // Validate trigger values
    if (output->trigger_right_force > MAX_TRIGGER_FORCE || 
        output->trigger_left_force > MAX_TRIGGER_FORCE) {
        return 0;
    }
    
    // Validate audio values
    if (output->speaker_volume > MAX_VOLUME || 
        output->mic_volume > MAX_VOLUME) {
        return 0;
    }
    
    return 1;
}

// Process output with optimizations
int optimize_process_output(const ps5_output_t* output) {
    uint64_t start_time = get_system_time();
    int result = 0;
    
    // Output validation
    if (!validate_output_state(output)) {
        config.stats.output_errors++;
        config.stats.frames_dropped++;
        config.stats.error_count++;
        return 0;
    }
    
    // Process output based on mode
    switch (config.mode) {
        case PROCESS_MODE_SAFE:
            // Basic processing with validation
            if (validate_output_state(output)) {
                result = ps5_send_output(output);
            }
            break;
            
        case PROCESS_MODE_FAST:
            // Direct output with DMA if available
            if (config.features & OPT_DMA_ENABLED) {
                dma_memcpy((void*)output, output, sizeof(ps5_output_t));
                result = ps5_send_output(output);
            } else {
                result = ps5_send_output(output);
            }
            break;
            
        default:
            // Standard processing
            result = ps5_send_output(output);
            break;
    }
    
    // Update statistics
    config.stats.output_latency_us = (uint32_t)(get_system_time() - start_time);
    if (!result) {
        config.stats.frames_dropped++;
    }
    
    return result;
}

// System voltage register
#define VOLTAGE_REG ((volatile uint32_t*)(CPU_CONTROL_BASE + 0x14))

// Get performance statistics
void optimize_get_stats(performance_stats_t* stats) {
    if (stats) {
        static uint64_t start_time = 0;
        uint64_t current_time = get_system_time();
        
        if (start_time == 0) {
            start_time = current_time;
        }
        
        // Update latency metrics
        config.stats.total_latency_us = config.stats.input_latency_us + config.stats.output_latency_us;
        if (config.stats.total_latency_us < config.stats.min_latency_us || config.stats.min_latency_us == 0) {
            config.stats.min_latency_us = config.stats.total_latency_us;
        }
        if (config.stats.total_latency_us > config.stats.max_latency_us) {
            config.stats.max_latency_us = config.stats.total_latency_us;
        }
        
        // Update system metrics
        config.stats.temperature = *CPU_TEMP_REG;
        config.stats.cpu_usage = (*CPU_THROTTLE_REG & 0xFF) / 255.0f * 100.0f;
        config.stats.voltage_mv = (*VOLTAGE_REG & 0xFFF) * 1000 / 4096; // Convert to millivolts
        
        // Update buffer metrics
        uint32_t total_buffer = config.input_buffer_ms + config.output_buffer_ms;
        config.stats.buffer_usage = (total_buffer * 100) / (2 * MAX_BUFFER_SIZE_MS);
        
        // Update uptime
        config.stats.uptime_ms = (uint32_t)((current_time - start_time) / 1000);
        
        // Copy stats to output
        *stats = config.stats;
    }
}

// Performance thresholds
#define CRITICAL_TEMP_THRESHOLD   85
#define HIGH_TEMP_THRESHOLD      75
#define NORMAL_TEMP_THRESHOLD    65
#define HIGH_CPU_THRESHOLD       90.0f
#define NORMAL_CPU_THRESHOLD     70.0f
#define ERROR_RATE_THRESHOLD     0.1f    // 10% error rate
#define TARGET_LATENCY_US        2000    // 2ms target latency
#define FREQ_STEP_SIZE          100000000 // 100MHz steps

// Auto-tune performance
void optimize_tune_performance(void) {
    // Calculate error rate
    float error_rate = 0;
    if (config.stats.frames_processed > 0) {
        error_rate = (float)config.stats.frames_dropped / config.stats.frames_processed;
    }
    
    // Check temperature first
    if (config.stats.temperature >= CRITICAL_TEMP_THRESHOLD) {
        // Critical temperature - switch to safe mode
        optimize_set_mode(PROCESS_MODE_SAFE);
        return;
    }
    
    // Determine if we need to adjust performance
    int need_more_performance = 0;
    int can_reduce_performance = 1;
    
    // Check various metrics
    if (error_rate > ERROR_RATE_THRESHOLD ||
        config.stats.total_latency_us > TARGET_LATENCY_US ||
        config.stats.buffer_overruns > 0) {
        need_more_performance = 1;
        can_reduce_performance = 0;
    }
    
    // Temperature-based throttling
    if (config.stats.temperature >= HIGH_TEMP_THRESHOLD) {
        can_reduce_performance = 0;
        if (config.cpu_freq > CPU_FREQ_MIN) {
            config.cpu_freq -= FREQ_STEP_SIZE;
            *CPU_FREQ_REG = config.cpu_freq;
        }
    } else if (config.stats.temperature < NORMAL_TEMP_THRESHOLD) {
        // Temperature is good, can increase if needed
        if (need_more_performance && config.cpu_freq < CPU_FREQ_TURBO) {
            config.cpu_freq += FREQ_STEP_SIZE;
            *CPU_FREQ_REG = config.cpu_freq;
        }
    }
    
    // CPU usage based tuning
    if (config.stats.cpu_usage >= HIGH_CPU_THRESHOLD) {
        can_reduce_performance = 0;
        // Try to optimize buffer sizes
        if (config.input_buffer_ms < MAX_BUFFER_SIZE_MS) {
            config.input_buffer_ms++;
        }
    } else if (config.stats.cpu_usage < NORMAL_CPU_THRESHOLD && can_reduce_performance == 1) {
        // CPU usage is low, can reduce frequency if latency is good
        if (config.stats.total_latency_us < TARGET_LATENCY_US / 2 &&
            config.cpu_freq > CPU_FREQ_MIN) {
            config.cpu_freq -= FREQ_STEP_SIZE;
            *CPU_FREQ_REG = config.cpu_freq;
        }
    }
    
    // Buffer tuning based on latency
    if (config.stats.buffer_underruns > 0) {
        if (config.input_buffer_ms < MAX_BUFFER_SIZE_MS) {
            config.input_buffer_ms++;
        }
    } else if (config.stats.total_latency_us < TARGET_LATENCY_US / 2 &&
               can_reduce_performance == 1) {
        if (config.input_buffer_ms > MIN_BUFFER_SIZE_MS) {
            config.input_buffer_ms--;
        }
    }
    
    // Reset error counters for next interval
    config.stats.frames_dropped = 0;
    config.stats.frames_processed = 0;
    config.stats.buffer_overruns = 0;
    config.stats.buffer_underruns = 0;
}

// Verify current processing mode
int optimize_verify_mode(process_mode_t mode) {
    return (config.mode == mode) ? 1 : 0;
}

// Verify enabled features
int optimize_verify_features(uint32_t features) {
    return ((config.features & features) == features) ? 1 : 0;
}

// Verify system stability
int optimize_verify_stability(void) {
    // Check temperature
    if (config.stats.temperature > 85) { // Critical temperature threshold
        return 0;
    }
    
    // Check error rates
    if (config.stats.frames_dropped > config.stats.frames_processed / 10) { // >10% drop rate
        return 0;
    }
    
    // Check buffer health
    if (config.stats.buffer_overruns > 5 || config.stats.buffer_underruns > 5) {
        return 0;
    }
    
    // Check CPU usage
    if (config.stats.cpu_usage > 90.0f) { // >90% CPU usage
        return 0;
    }
    
    return 1;
}

// Lock memory to prevent paging
void optimize_lock_memory(void) {
    // On bare metal, memory is already locked
    // This is just a placeholder for compatibility
}

// Prefetch data into cache
void optimize_prefetch_data(const void* addr, size_t size) {
    // Simple prefetch implementation without assembly
    volatile const uint8_t* p = (const uint8_t*)addr;
    for (size_t i = 0; i < size; i += 32) {
        (void)p[i]; // Force memory read
    }
}
