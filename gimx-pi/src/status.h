#ifndef STATUS_H
#define STATUS_H

#include <stdint.h>

// LED Pattern States
typedef enum {
    LED_STATE_INIT,           // Initial boot - single blink
    LED_STATE_HDMI_WAIT,     // Waiting for HDMI - double blink
    LED_STATE_PS5_WAIT,      // Waiting for PS5 - triple blink
    LED_STATE_CTRL_WAIT,     // Waiting for controller - quad blink
    LED_STATE_READY,         // All connected - steady on
    LED_STATE_ERROR,         // Error state - rapid blink
    LED_STATE_ACTIVE         // Processing input - pulse
} led_state_t;

// LED Pattern Timings (microseconds)
#define LED_BLINK_ON         100000   // Basic on time
#define LED_BLINK_OFF        100000   // Basic off time
#define LED_PATTERN_GAP      500000   // Gap between pattern repeats
#define LED_ERROR_SPEED      50000    // Fast blink for errors
#define LED_PULSE_MIN        50000    // Minimum brightness time
#define LED_PULSE_MAX        200000   // Maximum brightness time

// Function Prototypes
void status_init(void);
void status_update(led_state_t state);
void status_set_error(void);

#endif // STATUS_H
