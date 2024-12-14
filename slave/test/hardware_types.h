#ifndef HARDWARE_TYPES_H
#define HARDWARE_TYPES_H

#include <stdint.h>

// Performance stats structure
typedef struct {
    float temperature;
    float cpu_usage;
    float memory_usage;
} performance_stats_t;

// PS5 state structure
typedef struct {
    uint8_t buttons[32];
    uint8_t analog[8];
} ps5_state_t;

// PS5 output structure
typedef struct {
    uint8_t rumble[4];
    uint8_t led[3];
} ps5_output_t;

// Hardware function declarations
int hardware_init(void);
int optimize_init(void);
int status_init(void);
int usb_init(void);
int ps5_init(void);
uint64_t get_system_time(void);
void delay_microseconds(uint32_t us);
void status_update(int state);
void status_set_error(void);
int optimize_process_input(ps5_state_t* state);
void optimize_process_output(ps5_output_t* output);
int usb_detect_device(int device_type);
void optimize_set_mode(int mode);
void optimize_get_stats(performance_stats_t* stats);
void ps5_set_led_color(uint8_t r, uint8_t g, uint8_t b);
void ps5_set_haptic_feedback(uint8_t left, uint8_t right);

// Hardware constants
#define USB_DEVICE_PS5        1
#define USB_DEVICE_CONTROLLER 2

#define LED_STATE_INIT      0
#define LED_STATE_PS5_WAIT  1
#define LED_STATE_ACTIVE    2

#define PROCESS_MODE_FAST     0
#define PROCESS_MODE_ACCURATE 1

#endif // HARDWARE_TYPES_H
