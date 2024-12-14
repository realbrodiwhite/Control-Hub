#ifndef USB_H
#define USB_H

#include <stdint.h>

// USB Controller Registers
#define USB_BASE            0x3F980000
#define USB_CORE_BASE       (USB_BASE + 0x0)
#define USB_HOST_BASE       (USB_BASE + 0x400)
#define USB_POWER_BASE      (USB_BASE + 0xE00)

// USB Core Registers
#define USB_GAHBCFG         ((volatile uint32_t*)(USB_CORE_BASE + 0x008))
#define USB_GUSBCFG         ((volatile uint32_t*)(USB_CORE_BASE + 0x00C))
#define USB_GRSTCTL         ((volatile uint32_t*)(USB_CORE_BASE + 0x010))
#define USB_GINTSTS         ((volatile uint32_t*)(USB_CORE_BASE + 0x014))
#define USB_GINTMSK         ((volatile uint32_t*)(USB_CORE_BASE + 0x018))

// USB Host Registers
#define USB_HCFG            ((volatile uint32_t*)(USB_HOST_BASE + 0x000))
#define USB_HPRT            ((volatile uint32_t*)(USB_HOST_BASE + 0x040))

// USB Power Registers
#define USB_PCGCCTL         ((volatile uint32_t*)(USB_POWER_BASE + 0x000))

// Device Types
typedef enum {
    USB_DEVICE_PS5 = 0,         // PS5 Console
    USB_DEVICE_CONTROLLER = 1    // PS5 Controller
} usb_device_type_t;

// PS5 Console VID/PID
#define PS5_CONSOLE_VID     0x054C
#define PS5_CONSOLE_PID     0x0CF2

// PS5 Controller VID/PID
#define PS5_CONTROLLER_VID  0x054C
#define PS5_CONTROLLER_PID  0x0CE6

// Function Prototypes
void usb_init(void);
int usb_detect_device(usb_device_type_t device_type);
void usb_handle_controller(void);

#endif // USB_H
