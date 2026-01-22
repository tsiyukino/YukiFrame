#include "yuki_frame/framework.h"
#include "yuki_frame/config.h"
#include "yuki_frame/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE 1024
#define MAX_SECTION 64
#define MAX_KEY 1024
#define MAX_VALUE 512
#define MAX_CONFIG_ENTRIES 256

// Configuration entry storage
typedef struct {
    char section[MAX_SECTION];
    char key[MAX_KEY];
    char value[MAX_VALUE];
} ConfigEntry;

static ConfigEntry config_entries[MAX_CONFIG_ENTRIES];
static int config_entry_count = 0;
static char current_config_file[256] = "";

// Simple INI parser
static char* trim(char* str) {
    char* end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

int config_load(const char* config_file) {
    if (!config_file) {
        return FW_ERROR_INVALID_ARG;
    }
    
    strncpy(current_config_file, config_file, sizeof(current_config_file) - 1);
    current_config_file[sizeof(current_config_file) - 1] = '\0';
    
    FILE* fp = fopen(config_file, "r");
    if (!fp) {
        fprintf(stderr, "Failed to open config file: %s\n", config_file);
        return FW_ERROR_IO;
    }
    
    // Reset config storage
    config_entry_count = 0;
    
    // Set defaults
    strncpy(g_config.log_file, "logs/yuki-frame.log", sizeof(g_config.log_file) - 1);
    g_config.log_file[sizeof(g_config.log_file) - 1] = '\0';
    g_config.log_level = LOG_INFO;
    strncpy(g_config.pid_file, "yuki-frame.pid", sizeof(g_config.pid_file) - 1);
    g_config.pid_file[sizeof(g_config.pid_file) - 1] = '\0';
    g_config.max_tools = 50;
    g_config.message_queue_size = 1000;
    g_config.enable_debug = false;
    g_config.enable_remote_control = false;
    g_config.control_port = 9999;
    
    char line[MAX_LINE];
    char section[MAX_SECTION] = "";
    
    while (fgets(line, sizeof(line), fp)) {
        char* p = trim(line);
        
        // Skip comments and empty lines
        if (*p == '#' || *p == ';' || *p == '\0') {
            continue;
        }
        
        // Section header
        if (*p == '[') {
            char* end = strchr(p, ']');
            if (end) {
                *end = '\0';
                strncpy(section, p + 1, sizeof(section) - 1);
                section[sizeof(section) - 1] = '\0';
            }
            continue;
        }
        
        // Key=value pair
        char* eq = strchr(p, '=');
        if (eq) {
            *eq = '\0';
            char* key = trim(p);
            char* value = trim(eq + 1);
            
            // Store in generic config entries array
            if (config_entry_count < MAX_CONFIG_ENTRIES) {
                strncpy(config_entries[config_entry_count].section, section, MAX_SECTION - 1);
                config_entries[config_entry_count].section[MAX_SECTION - 1] = '\0';
                
                strncpy(config_entries[config_entry_count].key, key, MAX_KEY - 1);
                config_entries[config_entry_count].key[MAX_KEY - 1] = '\0';
                
                strncpy(config_entries[config_entry_count].value, value, MAX_VALUE - 1);
                config_entries[config_entry_count].value[MAX_VALUE - 1] = '\0';
                
                config_entry_count++;
            }
            
            // Also update g_config for core section (backward compatibility)
            if (strcmp(section, "core") == 0 || strcmp(section, "framework") == 0) {
                if (strcmp(key, "log_file") == 0) {
                    strncpy(g_config.log_file, value, sizeof(g_config.log_file) - 1);
                    g_config.log_file[sizeof(g_config.log_file) - 1] = '\0';
                } else if (strcmp(key, "log_level") == 0) {
                    if (strcmp(value, "TRACE") == 0) g_config.log_level = LOG_TRACE;
                    else if (strcmp(value, "DEBUG") == 0) g_config.log_level = LOG_DEBUG;
                    else if (strcmp(value, "INFO") == 0) g_config.log_level = LOG_INFO;
                    else if (strcmp(value, "WARN") == 0) g_config.log_level = LOG_WARN;
                    else if (strcmp(value, "ERROR") == 0) g_config.log_level = LOG_ERROR;
                    else if (strcmp(value, "FATAL") == 0) g_config.log_level = LOG_FATAL;
                } else if (strcmp(key, "pid_file") == 0) {
                    strncpy(g_config.pid_file, value, sizeof(g_config.pid_file) - 1);
                    g_config.pid_file[sizeof(g_config.pid_file) - 1] = '\0';
                } else if (strcmp(key, "max_tools") == 0) {
                    g_config.max_tools = atoi(value);
                } else if (strcmp(key, "message_queue_size") == 0) {
                    g_config.message_queue_size = atoi(value);
                } else if (strcmp(key, "enable_debug") == 0) {
                    g_config.enable_debug = (strcmp(value, "yes") == 0 || strcmp(value, "true") == 0 || strcmp(value, "1") == 0);
                }
            }
        }
    }
    
    fclose(fp);
    return FW_OK;
}

int config_reload(void) {
    return config_load(current_config_file);
}

const char* config_get(const char* section, const char* key) {
    if (!section || !key) {
        return NULL;
    }
    
    // Search through config entries
    for (int i = 0; i < config_entry_count; i++) {
        if (strcmp(config_entries[i].section, section) == 0 &&
            strcmp(config_entries[i].key, key) == 0) {
            return config_entries[i].value;
        }
    }
    
    return NULL;
}

int config_get_int(const char* section, const char* key, int default_value) {
    const char* value = config_get(section, key);
    if (value) {
        return atoi(value);
    }
    return default_value;
}

bool config_get_bool(const char* section, const char* key, bool default_value) {
    const char* value = config_get(section, key);
    if (value) {
        return (strcmp(value, "yes") == 0 || 
                strcmp(value, "true") == 0 || 
                strcmp(value, "1") == 0 ||
                strcmp(value, "True") == 0 ||
                strcmp(value, "YES") == 0);
    }
    return default_value;
}

int config_get_tools(ToolConfig** tools_out, int* count_out) {
    if (!tools_out || !count_out) {
        return FW_ERROR_INVALID_ARG;
    }
    
    FILE* fp = fopen(current_config_file, "r");
    if (!fp) {
        *tools_out = NULL;
        *count_out = 0;
        return FW_ERROR_IO;
    }
    
    // First pass: count tools
    int tool_count = 0;
    char line[MAX_LINE];
    char section[MAX_SECTION] = "";
    
    while (fgets(line, sizeof(line), fp)) {
        char* p = trim(line);
        
        if (*p == '[') {
            char* end = strchr(p, ']');
            if (end) {
                *end = '\0';
                strncpy(section, p + 1, sizeof(section) - 1);
                section[sizeof(section) - 1] = '\0';
                
                // Check if it's a tool section (format: tool:toolname)
                if (strncmp(section, "tool:", 5) == 0) {
                    tool_count++;
                }
            }
        }
    }
    
    if (tool_count == 0) {
        fclose(fp);
        *tools_out = NULL;
        *count_out = 0;
        return FW_OK;
    }
    
    // Allocate tools array
    ToolConfig* tools = (ToolConfig*)calloc(tool_count, sizeof(ToolConfig));
    if (!tools) {
        fclose(fp);
        return FW_ERROR_MEMORY;
    }
    
    // Second pass: parse tools
    rewind(fp);
    int current_tool = -1;
    section[0] = '\0';
    
    while (fgets(line, sizeof(line), fp)) {
        char* p = trim(line);
        
        // Skip comments and empty lines
        if (*p == '#' || *p == ';' || *p == '\0') {
            continue;
        }
        
        // Section header
        if (*p == '[') {
            char* end = strchr(p, ']');
            if (end) {
                *end = '\0';
                strncpy(section, p + 1, sizeof(section) - 1);
                section[sizeof(section) - 1] = '\0';
                
                // Check if it's a tool section (format: tool:toolname)
                if (strncmp(section, "tool:", 5) == 0) {
                    current_tool++;
                    // Extract tool name (after "tool:")
                    strncpy(tools[current_tool].name, section + 5, MAX_TOOL_NAME - 1);
                    tools[current_tool].name[MAX_TOOL_NAME - 1] = '\0';
                    // Set defaults
                    tools[current_tool].autostart = false;
                    tools[current_tool].restart_on_crash = false;
                    tools[current_tool].max_restarts = 3;
                    tools[current_tool].subscriptions[0] = '\0';
                }
            }
            continue;
        }
        
        // Key=value pair for current tool
        if (current_tool >= 0 && strncmp(section, "tool:", 5) == 0) {
            char* eq = strchr(p, '=');
            if (eq) {
                *eq = '\0';
                char* key = trim(p);
                char* value = trim(eq + 1);
                
                if (strcmp(key, "command") == 0) {
                    strncpy(tools[current_tool].command, value, MAX_COMMAND_LENGTH - 1);
                    tools[current_tool].command[MAX_COMMAND_LENGTH - 1] = '\0';
                } else if (strcmp(key, "description") == 0) {
                    strncpy(tools[current_tool].description, value, 255);
                    tools[current_tool].description[255] = '\0';
                } else if (strcmp(key, "autostart") == 0) {
                    tools[current_tool].autostart = (strcmp(value, "yes") == 0 || strcmp(value, "true") == 0);
                } else if (strcmp(key, "restart_on_crash") == 0) {
                    tools[current_tool].restart_on_crash = (strcmp(value, "yes") == 0 || strcmp(value, "true") == 0);
                } else if (strcmp(key, "max_restarts") == 0) {
                    tools[current_tool].max_restarts = atoi(value);
                } else if (strcmp(key, "subscribe_to") == 0) {
                    strncpy(tools[current_tool].subscriptions, value, 511);
                    tools[current_tool].subscriptions[511] = '\0';
                }
            }
        }
    }
    
    fclose(fp);
    
    *tools_out = tools;
    *count_out = tool_count;
    
    return FW_OK;
}

void config_free_tools(ToolConfig* tools, int count) {
    (void)count;
    if (tools) {
        free(tools);
    }
}
