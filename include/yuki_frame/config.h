#ifndef YUKI_FRAME_CONFIG_H
#define YUKI_FRAME_CONFIG_H

#include "framework.h"
#include "tool_queue.h"  // For QueuePolicy

// Forward declarations
typedef enum {
    RESTART_NEVER = 0,
    RESTART_ALWAYS = 1,
    RESTART_ON_DEMAND = 2
} RestartPolicy;

// Tool configuration from config file
typedef struct {
    char name[MAX_TOOL_NAME];
    char command[MAX_COMMAND_LENGTH];
    char description[256];
    bool autostart;
    bool restart_on_crash;
    int max_restarts;
    char subscriptions[512];
    
    // NEW FIELDS for queue system
    RestartPolicy restart_policy;
    int max_queue_size;
    QueuePolicy queue_policy;
} ToolConfig;

// Main config structure
typedef struct {
    char log_file[256];
    LogLevel log_level;
    char pid_file[256];
    int max_tools;
    int message_queue_size;
    bool enable_debug;
    bool enable_remote_control;
    int control_port;
} Config;

// Global config
extern Config g_config;

// Config functions
int config_load(const char* config_file);
int config_reload(void);
const char* config_get(const char* section, const char* key);
int config_get_int(const char* section, const char* key, int default_value);
bool config_get_bool(const char* section, const char* key, bool default_value);
int config_get_tools(ToolConfig** tools_out, int* count_out);
void config_free_tools(ToolConfig* tools, int count);

#endif // YUKI_FRAME_CONFIG_H
