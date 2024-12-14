#include "usb.h"
#include "status.h"

// USB Controller States
typedef enum {
    USB_STATE_RESET,
    USB_STATE_INIT,
    USB_STATE_READY
} usb_state_t;

static usb_state_t usb_state = USB_STATE_RESET;

// Port assignments
#define PS5_PORT        1  // PS5 console on first port
#define CONTROLLER_PORT 2  // Controller on second port

// USB Reset Sequence
static void usb_core_reset(void) {
    // Assert core soft reset
    *USB_GRSTCTL = 1;
    
    // Wait for reset completion
    while (*USB_GRSTCTL & 1) {
        __asm__ __volatile__("nop");
    }
    
    // Wait for AHB master idle
    while (!(*USB_GRSTCTL & (1 << 31))) {
        __asm__ __volatile__("nop");
    }
}

// Initialize USB Controller
void usb_init(void) {
    // Power up USB controller
    *USB_PCGCCTL = 0;
    
    // Perform core soft reset
    usb_core_reset();
    
    // Configure USB core
    *USB_GUSBCFG = (1 << 30) |    // Force host mode
                   (0 << 29) |     // PHY interface: UTMI+
                   (0 << 3);       // ULPI interface disabled
    
    // Configure AHB
    *USB_GAHBCFG = (1 << 0);      // Enable global interrupts
    
    // Configure host
    *USB_HCFG = (0 << 1) |        // 12MHz PHY clock
                (1 << 0);          // FS/LS PHY clock select
    
    // Enable port power on both ports
    volatile uint32_t* port1 = (volatile uint32_t*)(USB_HOST_BASE + 0x040);
    volatile uint32_t* port2 = (volatile uint32_t*)(USB_HOST_BASE + 0x060);
    *port1 |= (1 << 12);  // Port 1 power on
    *port2 |= (1 << 12);  // Port 2 power on
    
    usb_state = USB_STATE_INIT;
}

// Check device VID/PID
static int check_device_id(int port, uint16_t expected_vid, uint16_t expected_pid) {
    // Note: In real implementation, this would involve:
    // 1. USB enumeration
    // 2. Get device descriptor
    // 3. Compare VID/PID
    // For now, we'll simulate detection based on port connection
    
    volatile uint32_t* port_reg = (volatile uint32_t*)(USB_HOST_BASE + 0x040 + (port * 0x20));
    uint32_t port_status = *port_reg;
    
    // Check if device connected and enabled
    return (port_status & (1 << 1)) && (port_status & (1 << 3));
}

// Detect specific USB device
int usb_detect_device(usb_device_type_t device_type) {
    if (usb_state < USB_STATE_INIT) {
        return 0;
    }
    
    switch (device_type) {
        case USB_DEVICE_PS5:
            return check_device_id(PS5_PORT, PS5_CONSOLE_VID, PS5_CONSOLE_PID);
            
        case USB_DEVICE_CONTROLLER:
            return check_device_id(CONTROLLER_PORT, PS5_CONTROLLER_VID, PS5_CONTROLLER_PID);
            
        default:
            return 0;
    }
}

// Handle Controller Communication
void usb_handle_controller(void) {
    if (!usb_detect_device(USB_DEVICE_CONTROLLER)) {
        return;
    }
    
    // TODO: Implement full USB protocol stack for PS5 controller
    // This includes:
    // 1. Read controller input
    // 2. Process input
    // 3. Forward to PS5
    // 4. Handle feedback
    
    // For now, we just maintain the connection
    static uint32_t last_check = 0;
    last_check++;
    
    if (last_check >= 1000000) {
        last_check = 0;
        if (!usb_detect_device(USB_DEVICE_PS5)) {
            status_set_error();
        }
    }
}
