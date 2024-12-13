#include "ps5.h"
#include "usb.h"
#include "status.h"

// Raspberry Pi 3B CPU Cache Control
#define CACHE_LINE_SIZE     64
#define L1_CACHE_SIZE      32768
#define L2_CACHE_SIZE      512000

// DMA Buffer for USB transfers (aligned to cache line)
static __attribute__((aligned(CACHE_LINE_SIZE))) uint8_t usb_buffer[2048];

// Controller state cache
static ps5_state_t current_state;
static ps5_output_t current_output;

// Performance optimizations
static void enable_cache(void) {
    // Enable L1 cache
    asm volatile("mrc p15, 0, r0, c1, c0, 0");
    asm volatile("orr r0, r0, #(1 << 12)"); // Enable I-cache
    asm volatile("orr r0, r0, #(1 << 2)");  // Enable D-cache
    asm volatile("mcr p15, 0, r0, c1, c0, 0");
    
    // Enable L2 cache
    asm volatile("mrc p15, 0, r0, c1, c0, 1");
    asm volatile("orr r0, r0, #(1 << 1)");
    asm volatile("mcr p15, 0, r0, c1, c0, 1");
}

// Initialize PS5 subsystem
void ps5_init(void) {
    // Enable CPU caches for better performance
    enable_cache();
    
    // Clear state
    __builtin_memset(&current_state, 0, sizeof(ps5_state_t));
    __builtin_memset(&current_output, 0, sizeof(ps5_output_t));
    
    // Set default output state
    current_output.led_r = 0;
    current_output.led_g = 0;
    current_output.led_b = 64;  // Dim blue by default
    current_output.speaker_volume = 64;
    current_output.mic_volume = 64;
    
    // Apply initial settings
    ps5_send_output(&current_output);
}

// Process controller input with minimal latency
int ps5_process_input(ps5_state_t* state) {
    static uint32_t last_poll = 0;
    const uint32_t POLL_INTERVAL = 1000; // 1ms polling
    
    uint32_t now = *((volatile uint32_t*)0x3F003004); // System timer
    if (now - last_poll < POLL_INTERVAL) {
        return 0;
    }
    last_poll = now;
    
    // Read input report
    if (!usb_read_endpoint(USB_DEVICE_CONTROLLER, 0x84, usb_buffer, 64)) {
        return 0;
    }
    
    // Process input data
    __builtin_memcpy(&current_state, usb_buffer + 1, sizeof(ps5_state_t));
    
    // Copy to output if requested
    if (state) {
        __builtin_memcpy(state, &current_state, sizeof(ps5_state_t));
    }
    
    return 1;
}

// Send output report with haptics, LED, etc.
int ps5_send_output(const ps5_output_t* output) {
    if (!output) {
        return 0;
    }
    
    // Cache the output state
    __builtin_memcpy(&current_output, output, sizeof(ps5_output_t));
    
    // Prepare output report
    usb_buffer[0] = PS5_REPORT_OUTPUT;
    __builtin_memcpy(usb_buffer + 1, output, sizeof(ps5_output_t));
    
    // Send output report
    return usb_write_endpoint(USB_DEVICE_CONTROLLER, 0x05, usb_buffer, 64);
}

// Handle PS5 events and maintain connection
void ps5_handle_events(void) {
    static uint32_t last_check = 0;
    const uint32_t CHECK_INTERVAL = 1000000; // 1s health check
    
    uint32_t now = *((volatile uint32_t*)0x3F003004);
    if (now - last_check >= CHECK_INTERVAL) {
        last_check = now;
        
        // Check controller health
        if (!ps5_get_battery_level()) {
            status_set_error();
        }
    }
}

// Calibrate controller sensors
int ps5_calibrate_controller(void) {
    // Send calibration command
    usb_buffer[0] = PS5_REPORT_FEATURE;
    usb_buffer[1] = 0x05; // Calibration feature report
    return usb_write_endpoint(USB_DEVICE_CONTROLLER, 0x05, usb_buffer, 64);
}

// Enable low latency mode
void ps5_enable_low_latency(void) {
    // Disable USB polling interval
    usb_set_polling_interval(USB_DEVICE_CONTROLLER, 1); // 1ms polling
    
    // Disable controller processing features that add latency
    current_output.haptic_left_enable = 0;
    current_output.haptic_right_enable = 0;
    ps5_send_output(&current_output);
}

// Get controller battery level
int ps5_get_battery_level(void) {
    return current_state.battery_level;
}

// Set LED color
void ps5_set_led_color(uint8_t r, uint8_t g, uint8_t b) {
    current_output.led_r = r;
    current_output.led_g = g;
    current_output.led_b = b;
    ps5_send_output(&current_output);
}

// Set haptic feedback
void ps5_set_haptic_feedback(uint8_t left, uint8_t right) {
    current_output.haptic_left_enable = left > 0;
    current_output.haptic_right_enable = right > 0;
    current_output.haptic_left_intensity = left;
    current_output.haptic_right_intensity = right;
    ps5_send_output(&current_output);
}

// Set adaptive trigger feedback
void ps5_set_trigger_feedback(uint8_t left_mode, uint8_t right_mode, 
                            uint8_t left_force, uint8_t right_force) {
    current_output.trigger_left_mode = left_mode;
    current_output.trigger_right_mode = right_mode;
    current_output.trigger_left_force = left_force;
    current_output.trigger_right_force = right_force;
    ps5_send_output(&current_output);
}

// Set audio settings
void ps5_set_audio(uint8_t speaker_vol, uint8_t mic_vol, uint8_t enable) {
    current_output.speaker_volume = speaker_vol;
    current_output.mic_volume = mic_vol;
    current_output.audio_enable = enable;
    ps5_send_output(&current_output);
}
