#include <stdint.h>
#include "ps5.h"
#include "usb.h"
#include "status.h"
#include "hardware.h"
#include "optimize.h"

// System state and error handling
typedef struct {
    int hdmi_connected;
    int ps5_connected;
    int controller_connected;
    ps5_state_t controller_state;
    ps5_output_t controller_output;
    performance_stats_t perf_stats;
    uint32_t error_count;
    uint32_t recovery_attempts;
    uint64_t last_error_time;
    char error_message[256];
} system_state_t;

// Connection retry parameters
#define MAX_CONNECT_RETRIES 3
#define CONNECT_RETRY_DELAY_MS 1000
#define ERROR_COOLDOWN_MS 5000
#define WATCHDOG_TIMEOUT_MS 5000

static system_state_t state = {0};
static uint64_t watchdog_last_kick = 0;

// Error recovery function
static void system_recover(const char* error_msg) {
    uint64_t current_time = get_system_time();
    
    // Check error cooldown
    if (current_time - state.last_error_time < ERROR_COOLDOWN_MS * 1000) {
        state.error_count++;
        return;
    }
    
    // Log error
    state.error_count++;
    state.recovery_attempts++;
    state.last_error_time = current_time;
    snprintf(state.error_message, sizeof(state.error_message), "Error: %s, Count: %u, Recovery Attempts: %u", error_msg, state.error_count, state.recovery_attempts);
    
    // Reset subsystems
    status_set_error();
    hardware_init();
    optimize_init();
    usb_init();
    ps5_init();
    
    // Reset connection states
    state.ps5_connected = 0;
    state.controller_connected = 0;
}

// Watchdog kick function
static void kick_watchdog(void) {
    watchdog_last_kick = get_system_time();
}

// Check watchdog timeout
static int check_watchdog(void) {
    if (get_system_time() - watchdog_last_kick > WATCHDOG_TIMEOUT_MS * 1000) {
        system_recover("Watchdog timeout");
        return 1;
    }
    return 0;
}

// Initialize system with robust error handling
static int system_init(void) {
    int success = 1;
    
    // Initialize hardware optimizations with verification
    if (!optimize_init()) {
        status_set_error();
        success = 0;
    }
    
    // Set high-performance mode with validation
    optimize_set_mode(PROCESS_MODE_FAST);
    if (!optimize_verify_mode(PROCESS_MODE_FAST)) {
        system_recover("Failed to set performance mode");
        success = 0;
    }
    
    // Enable optimizations with feature verification
    uint32_t features = OPT_NEON_ENABLED | OPT_GPU_ENABLED | 
                       OPT_DMA_ENABLED | OPT_CACHE_ENABLED |
                       OPT_LOW_LATENCY;
    
    optimize_enable_features(features);
    if (!optimize_verify_features(features)) {
        system_recover("Failed to enable optimizations");
        success = 0;
    }
    
    // Initialize subsystems with error checking
    if (!status_init() || !usb_init() || !ps5_init()) {
        system_recover("Subsystem initialization failed");
        success = 0;
    }
    
    // Initialize watchdog
    kick_watchdog();
    
    // Initial LED pattern
    status_update(LED_STATE_INIT);
    
    return success;
}

// Main program entry with robust error handling
int main(void) {
    // Initialize system with retry
    int retry_count = 0;
    while (!system_init() && retry_count < MAX_CONNECT_RETRIES) {
        delay_microseconds(CONNECT_RETRY_DELAY_MS * 1000);
        retry_count++;
    }
    
    if (retry_count >= MAX_CONNECT_RETRIES) {
        status_set_error();
        return 1;
    }
    
    // System monitoring intervals
    uint64_t last_perf_check = 0;
    uint64_t last_health_check = 0;
    const uint32_t PERF_CHECK_INTERVAL = 1000000;  // 1 second
    const uint32_t HEALTH_CHECK_INTERVAL = 500000; // 500ms
    
    // Main control loop with error handling
    while (1) {
        // Kick watchdog
        kick_watchdog();
        
        // Check system health
        uint64_t current_time = get_system_time();
        if (current_time - last_health_check >= HEALTH_CHECK_INTERVAL) {
            // Verify system stability
            if (!optimize_verify_stability() || check_watchdog()) {
                system_recover("System instability detected");
                continue;
            }
            
            // Check error threshold
            if (state.error_count > 10) {
                status_set_error();
                optimize_set_mode(PROCESS_MODE_SAFE);
                state.error_count = 0;
            }
            
            last_health_check = current_time;
        }
        // Check HDMI first
        if (!state.hdmi_connected) {
            status_update(LED_STATE_HDMI_WAIT);
            // HDMI detection handled by status module
            continue;
        }
        
        // Check PS5 connection
        if (!state.ps5_connected) {
            status_update(LED_STATE_PS5_WAIT);
            if (usb_detect_device(USB_DEVICE_PS5)) {
                state.ps5_connected = 1;
                ps5_enable_low_latency(); // Enable low latency mode
            }
            continue;
        }
        
        // Check controller connection
        if (!state.controller_connected) {
            status_update(LED_STATE_CTRL_WAIT);
            if (usb_detect_device(USB_DEVICE_CONTROLLER)) {
                state.controller_connected = 1;
                ps5_calibrate_controller(); // Calibrate on connection
            }
            continue;
        }
        
        // Process controller input/output
        if (state.ps5_connected && state.controller_connected) {
            status_update(LED_STATE_ACTIVE);
            
            // Process input with optimizations
            if (optimize_process_input(&state.controller_state)) {
                // Process output with optimizations
                optimize_process_output(&state.controller_output);
                
                // Update LED color based on battery level
                uint8_t battery = ps5_get_battery_level();
                if (battery < 20) {
                    ps5_set_led_color(255, 0, 0); // Red for low battery
                } else if (battery < 50) {
                    ps5_set_led_color(255, 165, 0); // Orange for medium
                } else {
                    ps5_set_led_color(0, 255, 0); // Green for good
                }
            }
            
            // Check for disconnections
            if (!usb_detect_device(USB_DEVICE_PS5)) {
                state.ps5_connected = 0;
                status_update(LED_STATE_PS5_WAIT);
            }
            if (!usb_detect_device(USB_DEVICE_CONTROLLER)) {
                state.controller_connected = 0;
                status_update(LED_STATE_CTRL_WAIT);
            }
        }
        
        // Performance monitoring and tuning
        uint64_t current_time = get_system_time();
        if (current_time - last_perf_check >= PERF_CHECK_INTERVAL) {
            optimize_get_stats(&state.perf_stats);
            
            // Auto-tune performance based on metrics
            optimize_tune_performance();
            
            // Check temperature and throttle if needed
            if (state.perf_stats.temperature > 80) { // 80°C threshold
                optimize_set_mode(PROCESS_MODE_NORMAL);
            } else if (state.perf_stats.temperature < 70) {
                optimize_set_mode(PROCESS_MODE_FAST);
            }
            
            last_perf_check = current_time;
        }
    }

    return 0;
}

// Boot code
__attribute__((section(".text.boot"), naked)) void _start(void) {
    __asm__ __volatile__(
        "mov sp, #0x8000\n\t"
        "bl main\n\t"
    );
}

// Vector table
__attribute__((section(".vector_table")))
static const uint32_t vectors[] = {
    0x8000,             // Initial stack pointer
    (uint32_t)_start,   // Reset vector
    (uint32_t)_start,   // Undefined instruction
    (uint32_t)_start,   // Software interrupt
    (uint32_t)_start,   // Prefetch abort
    (uint32_t)_start,   // Data abort
    0,                  // Reserved
    (uint32_t)_start    // IRQ
};
