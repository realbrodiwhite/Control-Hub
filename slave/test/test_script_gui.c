#include "test_script_gui.h"
#include "test_framework.h"
#include "../src/input.h"
#include "../src/util.h"

// Test data
static script_entry_t test_scripts[] = {
    {
        .meta = {
            .name = "Test Script 1",
            .author = "Test Author",
            .version = "1.0.0",
            .description = "Test script description",
            .game = "Test Game",
            .downloads = 100,
            .rating = 5,
            .size = 1024,
            .checksum = 0x12345678,
            .is_private = 0
        },
        .is_active = 1,
        .is_local = 1,
        .path = "test/scripts/test1.script"
    },
    {
        .meta = {
            .name = "Test Script 2",
            .author = "Test Author",
            .version = "1.0.0",
            .description = "Another test script",
            .game = "Test Game 2",
            .downloads = 50,
            .rating = 4,
            .size = 2048,
            .checksum = 0x87654321,
            .is_private = 1
        },
        .is_active = 0,
        .is_local = 1,
        .path = "test/scripts/test2.script"
    }
};

// Test callback data
static struct {
    gui_event_t last_event;
    void* last_data;
    int callback_called;
} test_data;

// Test callback
static void test_gui_callback(gui_event_t event, void* data) {
    test_data.last_event = event;
    test_data.last_data = data;
    test_data.callback_called = 1;
}

// Initialize GUI tests
void test_gui_init(void) {
    TEST_ASSERT(script_gui_init());
    script_gui_set_callback(test_gui_callback);
    test_data.callback_called = 0;
}

// Test GUI navigation
void test_gui_navigation(void) {
    // Test main menu navigation
    test_gui_simulate_input(BUTTON_DOWN);
    test_gui_verify_selection(1);
    
    test_gui_simulate_input(BUTTON_UP);
    test_gui_verify_selection(0);
    
    // Test menu selection
    test_gui_simulate_input(BUTTON_A);
    TEST_ASSERT(test_data.callback_called);
    TEST_ASSERT(test_data.last_event == GUI_EVENT_SELECT);
    
    // Test back navigation
    test_gui_simulate_input(BUTTON_B);
    TEST_ASSERT(test_data.last_event == GUI_EVENT_BACK);
}

// Test script list display and interaction
void test_gui_script_list(void) {
    script_gui_show_scripts(test_scripts, 2);
    test_gui_verify_state(GUI_STATE_SCRIPT_LIST);
    
    // Test script selection
    test_gui_simulate_input(BUTTON_DOWN);
    test_gui_verify_selection(1);
    
    test_gui_simulate_input(BUTTON_A);
    TEST_ASSERT(test_data.callback_called);
    TEST_ASSERT(test_data.last_event == GUI_EVENT_SELECT);
}

// Test script details display
void test_gui_script_details(void) {
    script_gui_show_script_details(&test_scripts[0]);
    test_gui_verify_state(GUI_STATE_SCRIPT_DETAILS);
    
    // Test script activation
    test_gui_simulate_input(BUTTON_A);
    TEST_ASSERT(test_data.callback_called);
    TEST_ASSERT(test_data.last_event == GUI_EVENT_ACTIVATE);
}

// Test catalog browsing
void test_gui_catalog_browse(void) {
    script_gui_show_catalog();
    test_gui_verify_state(GUI_STATE_CATALOG_BROWSE);
    
    // Test script download
    test_gui_simulate_input(BUTTON_A);
    TEST_ASSERT(test_data.callback_called);
    TEST_ASSERT(test_data.last_event == GUI_EVENT_DOWNLOAD);
    
    // Test search
    script_gui_show_search_results(test_scripts, 2);
    test_gui_verify_state(GUI_STATE_CATALOG_SEARCH);
}

// Test script editor
void test_gui_script_edit(void) {
    script_gui_edit_script("Test Script 1");
    test_gui_verify_state(GUI_STATE_SCRIPT_EDIT);
    
    // Test new script creation
    script_gui_new_script();
    test_gui_verify_state(GUI_STATE_SCRIPT_EDIT);
}

// Test settings menu
void test_gui_settings(void) {
    script_gui_show_settings();
    test_gui_verify_state(GUI_STATE_SETTINGS);
    
    // Test settings update
    test_gui_simulate_input(BUTTON_A);
    TEST_ASSERT(test_data.callback_called);
}

// GUI test utilities implementation
void test_gui_simulate_input(uint32_t buttons) {
    script_gui_handle_input(buttons);
}

void test_gui_verify_state(gui_state_t expected_state) {
    gui_state_t current_state;
    TEST_ASSERT(script_gui_get_state(&current_state));
    TEST_ASSERT(current_state == expected_state);
}

void test_gui_verify_selection(uint32_t expected_item) {
    uint32_t current_item;
    TEST_ASSERT(script_gui_get_selection(&current_item));
    TEST_ASSERT(current_item == expected_item);
}

void test_gui_verify_message(const char* expected_message) {
    char current_message[256];
    TEST_ASSERT(script_gui_get_message(current_message, sizeof(current_message)));
    TEST_ASSERT(str_compare(current_message, expected_message) == 0);
}

void test_gui_verify_progress(const char* operation, uint32_t progress) {
    char current_operation[64];
    uint32_t current_progress;
    TEST_ASSERT(script_gui_get_progress(current_operation, sizeof(current_operation), &current_progress));
    TEST_ASSERT(str_compare(current_operation, operation) == 0);
    TEST_ASSERT(current_progress == progress);
}

// Register GUI tests
void register_gui_tests(void) {
    TEST_ADD(test_gui_init);
    TEST_ADD(test_gui_navigation);
    TEST_ADD(test_gui_script_list);
    TEST_ADD(test_gui_script_details);
    TEST_ADD(test_gui_catalog_browse);
    TEST_ADD(test_gui_script_edit);
    TEST_ADD(test_gui_settings);
}
