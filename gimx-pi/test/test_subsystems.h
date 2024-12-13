#ifndef TEST_SUBSYSTEMS_H
#define TEST_SUBSYSTEMS_H

#include <stdint.h>

// Subsystem initialization functions
int hardware_init(void);
int optimize_init(void);
int status_init(void);
int usb_init(void);
int ps5_init(void);

// Subsystem utility functions
uint64_t get_system_time(void);
void delay_microseconds(uint32_t us);
void status_update(int state);
void status_set_error(void);
int optimize_process_input(void* state);
void optimize_process_output(void* output);
int usb_detect_device(int device_type);
void optimize_set_mode(int mode);
void optimize_get_stats(void* stats);
void ps5_set_led_color(uint8_t r, uint8_t g, uint8_t b);
void ps5_set_haptic_feedback(uint8_t left, uint8_t right);

// Device types
#define USB_DEVICE_PS5        1
#define USB_DEVICE_CONTROLLER 2

// LED states
#define LED_STATE_INIT      0
#define LED_STATE_PS5_WAIT  1
#define LED_STATE_ACTIVE    2

// Process modes
#define PROCESS_MODE_FAST     0
#define PROCESS_MODE_ACCURATE 1

#endif // TEST_SUBSYSTEMS_H
