#ifndef SCRIPT_GUI_H
#define SCRIPT_GUI_H

#include "script_lib.h"
#include "input.h"
#include <stdint.h>

// GUI States
typedef enum {
    GUI_STATE_MAIN_MENU,
    GUI_STATE_SCRIPT_LIST,
    GUI_STATE_SCRIPT_DETAILS,
    GUI_STATE_SCRIPT_EDIT,
    GUI_STATE_CATALOG_BROWSE,
    GUI_STATE_CATALOG_SEARCH,
    GUI_STATE_SETTINGS
} gui_state_t;

// GUI Events
typedef enum {
    GUI_EVENT_NONE,
    GUI_EVENT_SELECT,
    GUI_EVENT_BACK,
    GUI_EVENT_ACTIVATE,
    GUI_EVENT_DEACTIVATE,
    GUI_EVENT_EDIT,
    GUI_EVENT_DELETE,
    GUI_EVENT_SEARCH,
    GUI_EVENT_DOWNLOAD,
    GUI_EVENT_UPLOAD,
    GUI_EVENT_RATE,
    GUI_EVENT_COMMENT
} gui_event_t;

// GUI Callbacks
typedef void (*gui_callback_t)(gui_event_t event, void* data);

// GUI Functions
int script_gui_init(void);
void script_gui_render(void);
void script_gui_handle_input(uint32_t buttons);
void script_gui_set_callback(gui_callback_t callback);
void script_gui_show_message(const char* message);
void script_gui_show_error(const char* error);
void script_gui_show_progress(const char* operation, uint32_t progress);
void script_gui_cleanup(void);

// Script List Functions
void script_gui_show_scripts(script_entry_t* scripts, uint32_t count);
void script_gui_show_script_details(const script_entry_t* script);
void script_gui_show_catalog(void);
void script_gui_show_search_results(script_entry_t* results, uint32_t count);

// Script Editor Functions
void script_gui_edit_script(const char* script_name);
void script_gui_new_script(void);
void script_gui_show_test_results(const char* script_name, int passed, const char* details);

// Settings Functions
void script_gui_show_settings(void);
void script_gui_update_settings(void);

// State query functions for testing
int script_gui_get_state(gui_state_t* state);
int script_gui_get_selection(uint32_t* selected_item);
int script_gui_get_message(char* message, uint32_t max_len);
int script_gui_get_progress(char* operation, uint32_t max_len, uint32_t* progress);

#endif // SCRIPT_GUI_H
