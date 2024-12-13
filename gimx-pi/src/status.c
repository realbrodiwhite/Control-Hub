#include "status.h"

// Hardware registers for GPIO
#define MMIO_BASE       0x3F000000UL
#define GPIO_BASE       (MMIO_BASE + 0x200000)
#define GPIO_GPFSEL4    ((volatile uint32_t*)(GPIO_BASE + 0x10))
#define GPIO_GPSET1     ((volatile uint32_t*)(GPIO_BASE + 0x20))
#define GPIO_GPCLR1     ((volatile uint32_t*)(GPIO_BASE + 0x2C))

// HDMI registers
#define HDMI_BASE       (MMIO_BASE + 0x902000)
#define HDMI_STATUS     ((volatile uint32_t*)(HDMI_BASE + 0x004))

static volatile uint32_t current_time = 0;
static led_state_t current_state = LED_STATE_INIT;
static uint32_t pattern_position = 0;

// Simple delay function
static inline void delay(uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        __asm__ __volatile__("nop");
        current_time++;
    }
}

// LED control functions
static void led_on(void) {
    *GPIO_GPSET1 = 1UL << 15;  // GPIO 47
}

static void led_off(void) {
    *GPIO_GPCLR1 = 1UL << 15;  // GPIO 47
}

// Initialize GPIO for status LED
void status_init(void) {
    // Configure GPIO 47 as output
    *GPIO_GPFSEL4 = (*GPIO_GPFSEL4 & ~(7UL << 21)) | (1UL << 21);
}

// Check HDMI connection status
static int hdmi_connected(void) {
    return (*HDMI_STATUS & 0x1) != 0;
}

// Pattern implementations
static void pattern_single(void) {
    led_on();
    delay(LED_BLINK_ON);
    led_off();
    delay(LED_PATTERN_GAP);
}

static void pattern_double(void) {
    for (int i = 0; i < 2; i++) {
        led_on();
        delay(LED_BLINK_ON);
        led_off();
        delay(LED_BLINK_OFF);
    }
    delay(LED_PATTERN_GAP);
}

static void pattern_triple(void) {
    for (int i = 0; i < 3; i++) {
        led_on();
        delay(LED_BLINK_ON);
        led_off();
        delay(LED_BLINK_OFF);
    }
    delay(LED_PATTERN_GAP);
}

static void pattern_quad(void) {
    for (int i = 0; i < 4; i++) {
        led_on();
        delay(LED_BLINK_ON);
        led_off();
        delay(LED_BLINK_OFF);
    }
    delay(LED_PATTERN_GAP);
}

static void pattern_steady(void) {
    led_on();
}

static void pattern_error(void) {
    led_on();
    delay(LED_ERROR_SPEED);
    led_off();
    delay(LED_ERROR_SPEED);
}

static void pattern_pulse(void) {
    // Pulse LED smoothly
    static uint32_t pulse_time = LED_PULSE_MIN;
    static int increasing = 1;
    
    led_on();
    delay(pulse_time);
    led_off();
    delay(LED_PULSE_MIN);
    
    if (increasing) {
        pulse_time += 10000;
        if (pulse_time >= LED_PULSE_MAX) {
            increasing = 0;
        }
    } else {
        pulse_time -= 10000;
        if (pulse_time <= LED_PULSE_MIN) {
            increasing = 1;
        }
    }
}

// Update LED pattern based on state
void status_update(led_state_t state) {
    current_state = state;
    
    switch (state) {
        case LED_STATE_INIT:
            pattern_single();
            break;
        case LED_STATE_HDMI_WAIT:
            pattern_double();
            break;
        case LED_STATE_PS5_WAIT:
            pattern_triple();
            break;
        case LED_STATE_CTRL_WAIT:
            pattern_quad();
            break;
        case LED_STATE_READY:
            pattern_steady();
            break;
        case LED_STATE_ERROR:
            pattern_error();
            break;
        case LED_STATE_ACTIVE:
            pattern_pulse();
            break;
    }
}

// Set error state
void status_set_error(void) {
    current_state = LED_STATE_ERROR;
}
