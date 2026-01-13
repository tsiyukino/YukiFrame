#include "framework.h"
#include "config.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE 1024
#define MAX_SECTION 64
#define MAX_KEY 64
#define MAX_VALUE 512

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
    
    FILE* fp = fopen(config_file, "r");
    if (!fp) {
        fprintf(stderr, "Failed to open config file: %s\n", config_file);
        return FW_ERROR_IO;
    }
    
    // Set defaults
    strncpy(g_config.log_file, "logs/yuki-frame.log", sizeof(g_config.log_file) - 1);
    g_config.log_level = LOG_INFO;
    strncpy(g_config.pid_file, "yuki-frame.pid", sizeof(g_config.pid_file) - 1);
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
            }
            continue;
        }
        
        // Key=value pair
        char* eq = strchr(p, '=');
        if (eq && strcmp(section, "core") == 0) {
            *eq = '\0';
            char* key = trim(p);
            char* value = trim(eq + 1);
            
            if (strcmp(key, "log_file") == 0) {
                strncpy(g_config.log_file, value, sizeof(g_config.log_file) - 1);
            } else if (strcmp(key, "log_level") == 0) {
                if (strcmp(value, "TRACE") == 0) g_config.log_level = LOG_TRACE;
                else if (strcmp(value, "DEBUG") == 0) g_config.log_level = LOG_DEBUG;
                else if (strcmp(value, "INFO") == 0) g_config.log_level = LOG_INFO;
                else if (strcmp(value, "WARN") == 0) g_config.log_level = LOG_WARN;
                else if (strcmp(value, "ERROR") == 0) g_config.log_level = LOG_ERROR;
                else if (strcmp(value, "FATAL") == 0) g_config.log_level = LOG_FATAL;
            } else if (strcmp(key, "pid_file") == 0) {
                strncpy(g_config.pid_file, value, sizeof(g_config.pid_file) - 1);
            } else if (strcmp(key, "max_tools") == 0) {
                g_config.max_tools = atoi(value);
            } else if (strcmp(key, "message_queue_size") == 0) {
                g_config.message_queue_size = atoi(value);
            } else if (strcmp(key, "enable_debug") == 0) {
                g_config.enable_debug = (strcmp(value, "yes") == 0 || strcmp(value, "true") == 0);
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
    (void)section;
    (void)key;
    return NULL;
}

int config_get_int(const char* section, const char* key, int default_value) {
    (void)section;
    (void)key;
    return default_value;
}

bool config_get_bool(const char* section, const char* key, bool default_value) {
    (void)section;
    (void)key;
    return default_value;
}

int config_get_tools(ToolConfig** tools, int* count) {
    // For now, return empty list
    // TODO: Parse [tool:xxx] sections from config file
    *tools = NULL;
    *count = 0;
    return FW_OK;
}

void config_free_tools(ToolConfig* tools, int count) {
    (void)count;
    if (tools) {
        free(tools);
    }
}
