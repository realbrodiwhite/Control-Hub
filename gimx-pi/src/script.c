#include "script.h"
#include "optimize.h"
#include "status.h"
#include <stddef.h>

#define MAX_SCRIPTS 32
#define MAX_MACRO_LENGTH 1024
#define MAX_COMBO_LENGTH 16
#define SCRIPT_TIMEOUT_US 500 // 500μs timeout for script execution

// Combo definition
typedef struct {
    uint16_t buttons[MAX_COMBO_LENGTH];  // Button sequence
    uint32_t timing[MAX_COMBO_LENGTH];   // Timing requirements in microseconds
    uint32_t length;                     // Length of combo
    ps5_state_t result_state;            // State to apply when combo is detected
} combo_t;

// Script storage
static struct {
    script_context_t contexts[MAX_SCRIPTS];
    uint32_t script_count;
    
    // Macro recording/playback
    struct {
        ps5_state_t states[MAX_MACRO_LENGTH];
        uint32_t timings[MAX_MACRO_LENGTH];  // Time between states in microseconds
        uint32_t length;
        uint32_t current_pos;
        int is_recording;
        uint64_t last_state_time;
        char current_macro[64];
    } macro;
    
    // Combo system
    struct {
        combo_t combos[MAX_SCRIPTS];
        uint32_t combo_count;
        uint16_t current_buttons[MAX_COMBO_LENGTH];
        uint32_t current_timings[MAX_COMBO_LENGTH];
        uint32_t current_length;
        uint64_t last_button_time;
    } combo;
    
    // Performance tracking
    struct {
        uint32_t total_exec_time_us;
        uint32_t script_overruns;
        uint32_t cached_scripts;
        uint32_t successful_combos;
        uint32_t failed_combos;
    } stats;
} script_state = {0};

// Initialize scripting subsystem
int script_init(void) {
    script_state.macro.last_state_time = get_system_time();
    script_state.combo.last_button_time = get_system_time();
    return 1;
}

// Process input through scripts with ultra-low latency
void script_process_input(ps5_state_t* state) {
    uint64_t start_time = get_system_time();
    
    // Process scripts in priority order
    for (uint32_t i = 0; i < script_state.script_count; i++) {
        script_context_t* ctx = &script_state.contexts[i];
        uint64_t script_start = get_system_time();
        
        // Check timeout to ensure low latency
        if (get_system_time() - start_time > SCRIPT_TIMEOUT_US) {
            script_state.stats.script_overruns++;
            break;
        }
        
        switch (ctx->type) {
            case SCRIPT_TYPE_MACRO:
                if (script_state.macro.is_recording) {
                    // Record current state with timing
                    if (script_state.macro.length < MAX_MACRO_LENGTH) {
                        uint64_t current_time = get_system_time();
                        script_state.macro.states[script_state.macro.length] = *state;
                        script_state.macro.timings[script_state.macro.length] = 
                            (uint32_t)(current_time - script_state.macro.last_state_time);
                        script_state.macro.length++;
                        script_state.macro.last_state_time = current_time;
                    }
                } else if (script_state.macro.length > 0) {
                    // Playback recorded state with timing
                    *state = script_state.macro.states[script_state.macro.current_pos];
                    
                    // Wait for correct timing
                    uint64_t current_time = get_system_time();
                    if (current_time - script_state.macro.last_state_time >= 
                        script_state.macro.timings[script_state.macro.current_pos]) {
                        script_state.macro.current_pos++;
                        if (script_state.macro.current_pos >= script_state.macro.length) {
                            script_state.macro.current_pos = 0;
                        }
                        script_state.macro.last_state_time = current_time;
                    }
                }
                break;
                
            case SCRIPT_TYPE_COMBO:
                // Update current button sequence
                if (state->buttons != script_state.combo.current_buttons[script_state.combo.current_length - 1]) {
                    uint64_t current_time = get_system_time();
                    if (script_state.combo.current_length < MAX_COMBO_LENGTH) {
                        script_state.combo.current_buttons[script_state.combo.current_length] = state->buttons;
                        script_state.combo.current_timings[script_state.combo.current_length] = 
                            (uint32_t)(current_time - script_state.combo.last_button_time);
                        script_state.combo.current_length++;
                        script_state.combo.last_button_time = current_time;
                    }
                    
                    // Check for matching combos
                    for (uint32_t j = 0; j < script_state.combo.combo_count; j++) {
                        combo_t* combo = &script_state.combo.combos[j];
                        if (script_state.combo.current_length >= combo->length) {
                            int match = 1;
                            for (uint32_t k = 0; k < combo->length; k++) {
                                if (script_state.combo.current_buttons[
                                    script_state.combo.current_length - combo->length + k] 
                                    != combo->buttons[k] ||
                                    script_state.combo.current_timings[k] > combo->timing[k]) {
                                    match = 0;
                                    break;
                                }
                            }
                            if (match) {
                                *state = combo->result_state;
                                script_state.stats.successful_combos++;
                                break;
                            }
                        }
                    }
                }
                break;
        }
        
        // Update script statistics
        ctx->last_exec_us = get_system_time();
        ctx->exec_time_us = (uint32_t)(ctx->last_exec_us - script_start);
        ctx->exec_count++;
    }
    
    // Update total execution time
    script_state.stats.total_exec_time_us = (uint32_t)(get_system_time() - start_time);
}

// Start macro recording
int script_record_macro(const char* name) {
    if (script_state.macro.is_recording) {
        return 0;
    }
    
    script_state.macro.is_recording = 1;
    script_state.macro.length = 0;
    script_state.macro.current_pos = 0;
    script_state.macro.last_state_time = get_system_time();
    
    // Copy name with bounds checking
    size_t name_len = 0;
    while (name[name_len] && name_len < sizeof(script_state.macro.current_macro) - 1) {
        script_state.macro.current_macro[name_len] = name[name_len];
        name_len++;
    }
    script_state.macro.current_macro[name_len] = '\0';
    
    return 1;
}

// Stop macro recording
int script_stop_recording(void) {
    if (!script_state.macro.is_recording) {
        return 0;
    }
    
    script_state.macro.is_recording = 0;
    return 1;
}

// Add a new combo
int script_add_combo(const uint16_t* buttons, const uint32_t* timings, 
                    uint32_t length, const ps5_state_t* result) {
    if (script_state.combo.combo_count >= MAX_SCRIPTS || length > MAX_COMBO_LENGTH) {
        return 0;
    }
    
    combo_t* combo = &script_state.combo.combos[script_state.combo.combo_count];
    
    // Copy combo data
    for (uint32_t i = 0; i < length; i++) {
        combo->buttons[i] = buttons[i];
        combo->timing[i] = timings[i];
    }
    combo->length = length;
    combo->result_state = *result;
    
    script_state.combo.combo_count++;
    return 1;
}

// Cleanup scripting system
void script_cleanup(void) {
    script_state.script_count = 0;
    script_state.combo.combo_count = 0;
    script_state.macro.length = 0;
    script_state.macro.is_recording = 0;
}
