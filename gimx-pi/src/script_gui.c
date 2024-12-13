#include "script_gui.h"
#include "status.h"
#include "util.h"

#define MAX_MENU_ITEMS 16
#define MAX_MESSAGE_LEN 256
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define ITEM_HEIGHT 20
#define SCROLL_MARGIN 4

// GUI state
static struct {
    gui_state_t state;
    gui_callback_t callback;
    uint32_t selected_item;
    uint32_t scroll_offset;
    uint32_t total_items;
    char message[MAX_MESSAGE_LEN];
    uint32_t message_timer;
    struct {
        script_entry_t* items;
        uint32_t count;
    } list;
    struct {
        char operation[64];
        uint32_t progress;
        int show;
    } progress;
} gui = {0};

// Menu definitions
static const char* main_menu_items[] = {
    "Active Scripts",
    "Browse Scripts",
    "Search Catalog",
    "Create New",
    "Settings",
    NULL
};

static const char* script_menu_items[] = {
    "Activate/Deactivate",
    "Edit",
    "Test",
    "Delete",
    "Back",
    NULL
};

// Initialize GUI
int script_gui_init(void) {
    gui.state = GUI_STATE_MAIN_MENU;
    gui.selected_item = 0;
    gui.scroll_offset = 0;
    gui.message_timer = 0;
    gui.progress.show = 0;
    return 1;
}

// Render menu items
static void render_menu_items(const char** items, uint32_t count) {
    // Calculate visible range
    uint32_t visible_start = gui.scroll_offset;
    uint32_t visible_end = visible_start + (SCREEN_HEIGHT / ITEM_HEIGHT);
    if (visible_end > count) visible_end = count;
    
    // Render items
    for (uint32_t i = visible_start; i < visible_end; i++) {
        uint32_t y = (i - visible_start) * ITEM_HEIGHT;
        int selected = (i == gui.selected_item);
        
        // Draw selection highlight
        if (selected) {
            // TODO: Draw highlight rectangle
        }
        
        // Draw item text
        // TODO: Draw text with appropriate color
    }
    
    // Draw scroll indicators if needed
    if (visible_start > 0) {
        // TODO: Draw up arrow
    }
    if (visible_end < count) {
        // TODO: Draw down arrow
    }
}

// Render script list
static void render_script_list(void) {
    if (!gui.list.items || !gui.list.count) return;
    
    for (uint32_t i = 0; i < gui.list.count; i++) {
        const script_entry_t* script = &gui.list.items[i];
        uint32_t y = i * ITEM_HEIGHT;
        
        // Skip if not visible
        if (y < gui.scroll_offset * ITEM_HEIGHT) continue;
        if (y >= SCREEN_HEIGHT) break;
        
        // Draw script entry
        y -= gui.scroll_offset * ITEM_HEIGHT;
        
        // Draw selection highlight
        if (i == gui.selected_item) {
            // TODO: Draw highlight rectangle
        }
        
        // Draw script info
        // TODO: Draw script name, status, rating etc.
    }
}

// Render script details
static void render_script_details(void) {
    if (!gui.list.items || gui.selected_item >= gui.list.count) return;
    
    const script_entry_t* script = &gui.list.items[gui.selected_item];
    
    // TODO: Draw script details (name, author, description, etc.)
    // TODO: Draw script menu options
}

// Render progress bar
static void render_progress(void) {
    if (!gui.progress.show) return;
    
    // TODO: Draw progress bar and operation text
}

// Main render function
void script_gui_render(void) {
    // Clear screen
    // TODO: Clear screen buffer
    
    // Render current state
    switch (gui.state) {
        case GUI_STATE_MAIN_MENU:
            render_menu_items(main_menu_items, sizeof(main_menu_items)/sizeof(char*) - 1);
            break;
            
        case GUI_STATE_SCRIPT_LIST:
            render_script_list();
            break;
            
        case GUI_STATE_SCRIPT_DETAILS:
            render_script_details();
            break;
            
        case GUI_STATE_SCRIPT_EDIT:
            // TODO: Implement script editor rendering
            break;
            
        case GUI_STATE_CATALOG_BROWSE:
        case GUI_STATE_CATALOG_SEARCH:
            render_script_list();
            break;
            
        case GUI_STATE_SETTINGS:
            // TODO: Implement settings rendering
            break;
    }
    
    // Render progress if active
    render_progress();
    
    // Render message if active
    if (gui.message_timer > 0) {
        // TODO: Draw message box
        gui.message_timer--;
    }
}

// Handle input
void script_gui_handle_input(uint32_t buttons) {
    gui_event_t event = GUI_EVENT_NONE;
    
    // Handle navigation
    if (buttons & BUTTON_UP) {
        if (gui.selected_item > 0) {
            gui.selected_item--;
            if (gui.selected_item < gui.scroll_offset) {
                gui.scroll_offset = gui.selected_item;
            }
        }
    }
    
    if (buttons & BUTTON_DOWN) {
        if (gui.selected_item < gui.total_items - 1) {
            gui.selected_item++;
            if (gui.selected_item >= gui.scroll_offset + (SCREEN_HEIGHT / ITEM_HEIGHT)) {
                gui.scroll_offset = gui.selected_item - (SCREEN_HEIGHT / ITEM_HEIGHT) + 1;
            }
        }
    }
    
    // Handle selection/activation
    if (buttons & BUTTON_A) {
        event = GUI_EVENT_SELECT;
    }
    
    if (buttons & BUTTON_B) {
        event = GUI_EVENT_BACK;
    }
    
    // Call callback if set
    if (event != GUI_EVENT_NONE && gui.callback) {
        gui.callback(event, (void*)(uintptr_t)gui.selected_item);
    }
}

// Set callback
void script_gui_set_callback(gui_callback_t callback) {
    gui.callback = callback;
}

// Show message
void script_gui_show_message(const char* message) {
    str_copy(gui.message, message, sizeof(gui.message));
    gui.message_timer = 60; // Show for 1 second at 60fps
}

// Show error
void script_gui_show_error(const char* error) {
    // TODO: Add error icon/color
    script_gui_show_message(error);
}

// Show progress
void script_gui_show_progress(const char* operation, uint32_t progress) {
    str_copy(gui.progress.operation, operation, sizeof(gui.progress.operation));
    gui.progress.progress = progress;
    gui.progress.show = 1;
}

// Show scripts
void script_gui_show_scripts(script_entry_t* scripts, uint32_t count) {
    gui.list.items = scripts;
    gui.list.count = count;
    gui.total_items = count;
    gui.selected_item = 0;
    gui.scroll_offset = 0;
    gui.state = GUI_STATE_SCRIPT_LIST;
}

// Show script details
void script_gui_show_script_details(const script_entry_t* script) {
    gui.state = GUI_STATE_SCRIPT_DETAILS;
    gui.selected_item = 0;
}

// Show catalog
void script_gui_show_catalog(void) {
    gui.state = GUI_STATE_CATALOG_BROWSE;
    gui.selected_item = 0;
    gui.scroll_offset = 0;
}

// Show search results
void script_gui_show_search_results(script_entry_t* results, uint32_t count) {
    script_gui_show_scripts(results, count);
    gui.state = GUI_STATE_CATALOG_SEARCH;
}

// Edit script
void script_gui_edit_script(const char* script_name) {
    gui.state = GUI_STATE_SCRIPT_EDIT;
    // TODO: Initialize editor state
}

// New script
void script_gui_new_script(void) {
    gui.state = GUI_STATE_SCRIPT_EDIT;
    // TODO: Initialize editor state with template
}

// Show test results
void script_gui_show_test_results(const char* script_name, int passed, const char* details) {
    char message[MAX_MESSAGE_LEN];
    str_copy(message, passed ? "Test passed: " : "Test failed: ", sizeof(message));
    str_copy(message + str_len(message), details, 
            sizeof(message) - str_len(message));
    script_gui_show_message(message);
}

// Show settings
void script_gui_show_settings(void) {
    gui.state = GUI_STATE_SETTINGS;
    gui.selected_item = 0;
    gui.scroll_offset = 0;
}

// Update settings
void script_gui_update_settings(void) {
    // TODO: Save settings
    script_gui_show_message("Settings updated");
}

// State query functions for testing
int script_gui_get_state(gui_state_t* state) {
    if (!state) return 0;
    *state = gui.state;
    return 1;
}

int script_gui_get_selection(uint32_t* selected_item) {
    if (!selected_item) return 0;
    *selected_item = gui.selected_item;
    return 1;
}

int script_gui_get_message(char* message, uint32_t max_len) {
    if (!message || max_len == 0) return 0;
    str_copy(message, gui.message, max_len);
    return 1;
}

int script_gui_get_progress(char* operation, uint32_t max_len, uint32_t* progress) {
    if (!operation || !progress || max_len == 0) return 0;
    str_copy(operation, gui.progress.operation, max_len);
    *progress = gui.progress.progress;
    return 1;
}

// Cleanup
void script_gui_cleanup(void) {
    gui.list.items = NULL;
    gui.list.count = 0;
    gui.callback = NULL;
    gui.message_timer = 0;
    gui.progress.show = 0;
}
