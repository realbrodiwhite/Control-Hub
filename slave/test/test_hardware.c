#include "test_framework.h"
#include "hardware_types.h"
#include "../src/hardware.h"
#include "../src/status.h"

// Mock hardware state for testing
static struct {
    float temperature;
    float voltage;
    uint32_t gpio_state;
} mock_hw_state = {
    .temperature = 45.0f,
    .voltage = 5.0f,
    .gpio_state = 0
};

// Test hardware initialization
static void test_hardware_init(void) {
    TEST_ASSERT(hardware_init() == 0);
    TEST_ASSERT(status_get_led_state() == LED_STATE_INIT);
}

// Test temperature monitoring
static void test_temperature_monitoring(void) {
    float temp = 0.0f;
    TEST_ASSERT(hardware_get_temperature(&temp) == 0);
    TEST_ASSERT(temp == mock_hw_state.temperature);
}

// Test voltage monitoring
static void test_voltage_monitoring(void) {
    float voltage = 0.0f;
    TEST_ASSERT(hardware_get_voltage(&voltage) == 0);
    TEST_ASSERT(voltage == mock_hw_state.voltage);
}

// Test GPIO operations
static void test_gpio_operations(void) {
    // Test GPIO output
    TEST_ASSERT(hardware_gpio_set(16, 1) == 0);
    TEST_ASSERT((mock_hw_state.gpio_state & (1 << 16)) != 0);
    
    // Test GPIO input
    uint32_t value = 0;
    TEST_ASSERT(hardware_gpio_get(16, &value) == 0);
    TEST_ASSERT(value == 1);
}

// Test LED patterns
static void test_led_patterns(void) {
    // Test initialization pattern
    TEST_ASSERT(status_set_led_state(LED_STATE_INIT) == 0);
    TEST_ASSERT(status_get_led_state() == LED_STATE_INIT);
    
    // Test HDMI wait pattern
    TEST_ASSERT(status_set_led_state(LED_STATE_HDMI_WAIT) == 0);
    TEST_ASSERT(status_get_led_state() == LED_STATE_HDMI_WAIT);
}

// Test hardware error handling
static void test_error_handling(void) {
    // Test invalid GPIO pin
    TEST_ASSERT(hardware_gpio_set(64, 1) != 0);
    
    // Test invalid temperature read
    mock_hw_state.temperature = -999.0f;
    float temp = 0.0f;
    TEST_ASSERT(hardware_get_temperature(&temp) != 0);
}

// Register all hardware tests
void register_hardware_tests(void) {
    TEST_ADD(test_hardware_init);
    TEST_ADD(test_temperature_monitoring);
    TEST_ADD(test_voltage_monitoring);
    TEST_ADD(test_gpio_operations);
    TEST_ADD(test_led_patterns);
    TEST_ADD(test_error_handling);
}
