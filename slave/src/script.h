#ifndef SCRIPT_H
#define SCRIPT_H

#include <stdint.h>
#include "ps5.h"

// Script types
typedef enum {
    SCRIPT_TYPE_LUA,    // Lua scripts for complex game mods
    SCRIPT_TYPE_MACRO,  // Simple macro recording/playback
    SCRIPT_TYPE_COMBO   // Button combination macros
} script_type_t;

// Script execution context
typedef struct {
    script_type_t type;
    const char* name;
    uint32_t priority;      // Higher priority scripts run first
    uint32_t exec_count;    // Number of times executed
    uint64_t last_exec_us;  // Timestamp of last execution
    uint32_t exec_time_us;  // Average execution time
} script_context_t;

// Script API functions
int script_init(void);
int script_load(const char* filename, script_type_t type);
int script_unload(const char* name);
int script_enable(const char* name);
int script_disable(const char* name);
void script_process_input(ps5_state_t* state);
void script_process_output(ps5_output_t* output);
int script_record_macro(const char* name);
int script_stop_recording(void);
int script_save_macro(const char* name);
void script_cleanup(void);

#endif // SCRIPT_H
