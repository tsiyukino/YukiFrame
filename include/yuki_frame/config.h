#ifndef YUKI_FRAME_CONFIG_H
#define YUKI_FRAME_CONFIG_H

#include "yuki_frame/framework.h"

// Configuration parsing
int config_load(const char* config_file);
int config_reload(void);
const char* config_get(const char* section, const char* key);
int config_get_int(const char* section, const char* key, int default_value);
bool config_get_bool(const char* section, const char* key, bool default_value);

// Tool configuration
typedef struct {
    char name[MAX_TOOL_NAME];
    char command[MAX_COMMAND_LENGTH];
    char description[256];
    bool autostart;
    bool restart_on_crash;
    int max_restarts;
    char subscriptions[512]; // Comma-separated list
} ToolConfig;

int config_get_tools(ToolConfig** tools, int* count);
void config_free_tools(ToolConfig* tools, int count);

#endif  // YUKI_FRAME_CONFIG_H
