#ifndef TEST_SCRIPT_GUI_H
#define TEST_SCRIPT_GUI_H

#include "../src/script_gui.h"
#include "test_framework.h"

// GUI Test Categories
typedef enum {
    TEST_GUI_NAVIGATION,
    TEST_GUI_INTERACTION,
    TEST_GUI_RENDERING,
    TEST_GUI_SCRIPTS,
    TEST_GUI_CATALOG,
    TEST_GUI_EDITOR
} gui_test_category_t;

// GUI Test Cases
void test_gui_init(void);
void test_gui_navigation(void);
void test_gui_script_list(void);
void test_gui_script_details(void);
void test_gui_catalog_browse(void);
void test_gui_script_edit(void);
void test_gui_settings(void);

// GUI Test Utilities
void test_gui_simulate_input(uint32_t buttons);
void test_gui_verify_state(gui_state_t expected_state);
void test_gui_verify_selection(uint32_t expected_item);
void test_gui_verify_message(const char* expected_message);
void test_gui_verify_progress(const char* operation, uint32_t progress);

#endif // TEST_SCRIPT_GUI_H
