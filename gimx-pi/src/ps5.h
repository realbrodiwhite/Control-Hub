#ifndef PS5_H
#define PS5_H

#include <stdint.h>

// PS5 Controller Report IDs
#define PS5_REPORT_INPUT    0x01
#define PS5_REPORT_OUTPUT   0x02
#define PS5_REPORT_FEATURE  0x03

// PS5 Controller Features
#define PS5_FEATURE_HAPTIC  0x20
#define PS5_FEATURE_LED     0x21
#define PS5_FEATURE_AUDIO   0x22

// PS5 Controller Button Masks
typedef struct {
    uint8_t cross      : 1;
    uint8_t circle     : 1;
    uint8_t triangle   : 1;
    uint8_t square     : 1;
    uint8_t l1         : 1;
    uint8_t r1         : 1;
    uint8_t l2         : 1;
    uint8_t r2         : 1;
    uint8_t share      : 1;
    uint8_t options    : 1;
    uint8_t l3         : 1;
    uint8_t r3         : 1;
    uint8_t ps         : 1;
    uint8_t touchpad   : 1;
    uint8_t mute       : 1;
    uint8_t reserved   : 1;
} ps5_buttons_t;

// PS5 Controller Analog Sticks
typedef struct {
    uint8_t lx;
    uint8_t ly;
    uint8_t rx;
    uint8_t ry;
} ps5_sticks_t;

// PS5 Controller Triggers
typedef struct {
    uint8_t l2;
    uint8_t r2;
} ps5_triggers_t;

// PS5 Controller Gyro/Accel
typedef struct {
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
} ps5_motion_t;

// PS5 Controller Touch Points
typedef struct {
    uint8_t active : 1;
    uint8_t id     : 7;
    uint16_t x;
    uint16_t y;
} ps5_touch_point_t;

// PS5 Controller State
typedef struct {
    ps5_buttons_t buttons;
    ps5_sticks_t sticks;
    ps5_triggers_t triggers;
    ps5_motion_t motion;
    ps5_touch_point_t touch[2];
    uint8_t battery_level;
    uint8_t connection_type;
    uint8_t temperature;
} ps5_state_t;

// PS5 Controller Output Features
typedef struct {
    uint8_t led_r;
    uint8_t led_g;
    uint8_t led_b;
    uint8_t led_pulse_options;
    uint8_t haptic_right_enable;
    uint8_t haptic_left_enable;
    uint8_t haptic_right_intensity;
    uint8_t haptic_left_intensity;
    uint8_t trigger_right_mode;
    uint8_t trigger_left_mode;
    uint8_t trigger_right_force;
    uint8_t trigger_left_force;
    uint8_t speaker_volume;
    uint8_t mic_volume;
    uint8_t audio_enable;
} ps5_output_t;

// Function Prototypes
void ps5_init(void);
int ps5_process_input(ps5_state_t* state);
int ps5_send_output(const ps5_output_t* output);
void ps5_handle_events(void);
int ps5_calibrate_controller(void);
void ps5_enable_low_latency(void);
int ps5_get_battery_level(void);
void ps5_set_led_color(uint8_t r, uint8_t g, uint8_t b);
void ps5_set_haptic_feedback(uint8_t left, uint8_t right);
void ps5_set_trigger_feedback(uint8_t left_mode, uint8_t right_mode, uint8_t left_force, uint8_t right_force);
void ps5_set_audio(uint8_t speaker_vol, uint8_t mic_vol, uint8_t enable);

#endif // PS5_H
