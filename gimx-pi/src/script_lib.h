#ifndef SCRIPT_LIB_H
#define SCRIPT_LIB_H

#include "script.h"
#include <stdint.h>

// Script metadata
typedef struct {
    char name[64];
    char author[32];
    char version[16];
    char description[256];
    char game[32];
    uint32_t downloads;
    uint32_t rating;
    uint32_t size;
    uint32_t checksum;
    int is_private;
    script_type_t type;
} script_meta_t;

// Script library entry
typedef struct {
    script_meta_t meta;
    int is_active;
    int is_local;
    char path[256];
} script_entry_t;

// Script library functions
int script_lib_init(void);
int script_lib_load_local(const char* directory);
int script_lib_connect_catalog(const char* server_url);
int script_lib_download(const char* script_name);
int script_lib_upload(const char* script_path, const script_meta_t* meta);
int script_lib_publish(const char* script_name);
int script_lib_activate(const char* script_name);
int script_lib_deactivate(const char* script_name);
int script_lib_search(const char* query, script_entry_t* results, uint32_t max_results);
int script_lib_get_popular(script_entry_t* results, uint32_t max_results);
int script_lib_get_active(script_entry_t* results, uint32_t max_results);
void script_lib_cleanup(void);

// Script catalog functions
typedef void (*catalog_callback_t)(const script_entry_t* entry, void* user_data);
int script_catalog_browse(const char* category, catalog_callback_t callback, void* user_data);
int script_catalog_search(const char* query, catalog_callback_t callback, void* user_data);
int script_catalog_get_updates(script_entry_t* updates, uint32_t* num_updates);
int script_catalog_rate_script(const char* script_name, uint32_t rating);
int script_catalog_add_comment(const char* script_name, const char* comment);

// Script management functions
int script_create_new(const char* template_name, const char* script_name);
int script_edit(const char* script_name);
int script_test(const char* script_name);
int script_package(const char* script_name, const script_meta_t* meta);
int script_validate(const char* script_path);

#endif // SCRIPT_LIB_H
