#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>

// Button definitions
#define BUTTON_UP     (1 << 0)
#define BUTTON_DOWN   (1 << 1)
#define BUTTON_LEFT   (1 << 2)
#define BUTTON_RIGHT  (1 << 3)
#define BUTTON_A      (1 << 4)
#define BUTTON_B      (1 << 5)
#define BUTTON_X      (1 << 6)
#define BUTTON_Y      (1 << 7)
#define BUTTON_L1     (1 << 8)
#define BUTTON_R1     (1 << 9)
#define BUTTON_L2     (1 << 10)
#define BUTTON_R2     (1 << 11)
#define BUTTON_START  (1 << 12)
#define BUTTON_SELECT (1 << 13)

// Input functions
uint32_t input_get_buttons(void);
void input_update(void);
int input_init(void);
void input_cleanup(void);

#endif // INPUT_H
