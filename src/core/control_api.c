/**
 * @file control_api.c
 * @brief Framework Control API implementation
 * 
 * This implements the public control API that tools use to manage other tools.
 */

#include "yuki_frame/control_api.h"
#include "yuki_frame/framework.h"
#include "yuki_frame/tool.h"
#include "yuki_frame/logger.h"
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <inttypes.h>

// Framework start time (set during initialization)
static time_t g_framework_start_time = 0;

// Initialize control API (called during framework init)
void control_api_init(void) {
    g_framework_start_time = time(NULL);
}

/* ============================================================================
 * Tool Management API Implementation
 * ============================================================================ */

int control_start_tool(const char* tool_name) {
    if (!tool_name) {
        LOG_ERROR("control_api", "tool_name is NULL");
        return FW_ERROR_INVALID_ARG;
    }
    
    LOG_INFO("control_api", "Starting tool: %s", tool_name);
    return tool_start(tool_name);
}

int control_stop_tool(const char* tool_name) {
    if (!tool_name) {
        LOG_ERROR("control_api", "tool_name is NULL");
        return FW_ERROR_INVALID_ARG;
    }
    
    LOG_INFO("control_api", "Stopping tool: %s", tool_name);
    return tool_stop(tool_name);
}

int control_restart_tool(const char* tool_name) {
    if (!tool_name) {
        LOG_ERROR("control_api", "tool_name is NULL");
        return FW_ERROR_INVALID_ARG;
    }
    
    LOG_INFO("control_api", "Restarting tool: %s", tool_name);
    return tool_restart(tool_name);
}

int control_get_tool_status(const char* tool_name, ControlToolInfo* info) {
    if (!tool_name || !info) {
        LOG_ERROR("control_api", "Invalid arguments");
        return FW_ERROR_INVALID_ARG;
    }
    
    Tool* tool = tool_find(tool_name);
    if (!tool) {
        return FW_ERROR_NOT_FOUND;
    }
    
    // Fill in the info structure
    memset(info, 0, sizeof(ControlToolInfo));
    strncpy(info->name, tool->name, sizeof(info->name) - 1);
    info->name[sizeof(info->name) - 1] = '\0';
    strncpy(info->command, tool->command, sizeof(info->command) - 1);
    info->command[sizeof(info->command) - 1] = '\0';
    strncpy(info->description, tool->description, sizeof(info->description) - 1);
    info->description[sizeof(info->description) - 1] = '\0';
    info->status = tool->status;
    info->pid = tool->pid;
    info->autostart = tool->autostart;
    info->restart_on_crash = tool->restart_on_crash;
    info->max_restarts = tool->max_restarts;
    info->restart_count = tool->restart_count;
    info->events_sent = tool->events_sent;
    info->events_received = tool->events_received;
    info->subscription_count = tool->subscription_count;
    
    return FW_OK;
}

int control_list_tools(control_list_callback_t callback, void* user_data) {
    if (!callback) {
        LOG_ERROR("control_api", "callback is NULL");
        return FW_ERROR_INVALID_ARG;
    }
    
    int count = 0;
    Tool* tool = tool_get_first();
    
    while (tool) {
        ControlToolInfo info;
        
        // Fill info structure
        memset(&info, 0, sizeof(ControlToolInfo));
        strncpy(info.name, tool->name, sizeof(info.name) - 1);
        info.name[sizeof(info.name) - 1] = '\0';
        strncpy(info.command, tool->command, sizeof(info.command) - 1);
        info.command[sizeof(info.command) - 1] = '\0';
        strncpy(info.description, tool->description, sizeof(info.description) - 1);
        info.description[sizeof(info.description) - 1] = '\0';
        info.status = tool->status;
        info.pid = tool->pid;
        info.autostart = tool->autostart;
        info.restart_on_crash = tool->restart_on_crash;
        info.max_restarts = tool->max_restarts;
        info.restart_count = tool->restart_count;
        info.events_sent = tool->events_sent;
        info.events_received = tool->events_received;
        info.subscription_count = tool->subscription_count;
        
        // Call callback
        bool continue_iteration = callback(&info, user_data);
        count++;
        
        if (!continue_iteration) {
            break;
        }
        
        tool = tool_get_next();
    }
    
    return count;
}

int control_get_tool_count(void) {
    int count = 0;
    Tool* tool = tool_get_first();
    
    while (tool) {
        count++;
        tool = tool_get_next();
    }
    
    return count;
}

bool control_tool_exists(const char* tool_name) {
    if (!tool_name) {
        return false;
    }
    
    return tool_find(tool_name) != NULL;
}

/* ============================================================================
 * Framework Control API Implementation
 * ============================================================================ */

int control_shutdown_framework(void) {
    LOG_INFO("control_api", "Framework shutdown requested");
    g_running = false;
    return FW_OK;
}

uint64_t control_get_uptime(void) {
    if (g_framework_start_time == 0) {
        return 0;
    }
    
    time_t now = time(NULL);
    return (uint64_t)(now - g_framework_start_time);
}

const char* control_get_version(void) {
    return YUKI_FRAME_VERSION_STRING;
}

/* ============================================================================
 * Interactive Console API Implementation
 * ============================================================================ */

int control_execute_command(const char* command, char* response, size_t response_size) {
    if (!command || !response || response_size == 0) {
        return FW_ERROR_INVALID_ARG;
    }
    
    // Parse command
    char cmd[256], arg[256];
    arg[0] = '\0';
    int parsed = sscanf(command, "%255s %255s", cmd, arg);
    
    if (parsed < 1) {
        snprintf(response, response_size, "Error: Empty command\n");
        return FW_ERROR_INVALID_ARG;
    }
    
    // Convert to lowercase
    for (char* p = cmd; *p; p++) {
        *p = tolower(*p);
    }
    
    // Execute command
    if (strcmp(cmd, "list") == 0) {
        int offset = 0;
        offset += snprintf(response + offset, response_size - offset,
                          "\nTools Status:\n");
        offset += snprintf(response + offset, response_size - offset,
                          "%-20s %-10s %-10s\n", "Name", "Status", "PID");
        offset += snprintf(response + offset, response_size - offset,
                          "------------------------------------------------------------\n");
        
        Tool* tool = tool_get_first();
        while (tool && offset < (int)response_size - 100) {
            const char* status_str;
            switch (tool->status) {
                case TOOL_STOPPED: status_str = "STOPPED"; break;
                case TOOL_RUNNING: status_str = "RUNNING"; break;
                case TOOL_CRASHED: status_str = "CRASHED"; break;
                case TOOL_ERROR:   status_str = "ERROR";   break;
                default:           status_str = "UNKNOWN"; break;
            }
            
            offset += snprintf(response + offset, response_size - offset,
                             "%-20s %-10s %-10d\n", tool->name, status_str, (int)tool->pid);
            tool = tool_get_next();
        }
        snprintf(response + offset, response_size - offset, "\n");
        return FW_OK;
    }
    else if (strcmp(cmd, "start") == 0 && strlen(arg) > 0) {
        int result = control_start_tool(arg);
        if (result == FW_OK) {
            Tool* tool = tool_find(arg);
            snprintf(response, response_size,
                    "Success: Tool '%s' started\n  PID: %d\n  Status: RUNNING\n",
                    arg, tool ? (int)tool->pid : 0);
        } else if (result == FW_ERROR_NOT_FOUND) {
            snprintf(response, response_size,
                    "Error: Tool '%s' not found in configuration\n", arg);
        } else {
            snprintf(response, response_size,
                    "Error: Failed to start tool '%s'\n", arg);
        }
        return result;
    }
    else if (strcmp(cmd, "stop") == 0 && strlen(arg) > 0) {
        int result = control_stop_tool(arg);
        if (result == FW_OK) {
            snprintf(response, response_size,
                    "Success: Tool '%s' stopped\n", arg);
        } else {
            snprintf(response, response_size,
                    "Error: Failed to stop tool '%s'\n", arg);
        }
        return result;
    }
    else if (strcmp(cmd, "restart") == 0 && strlen(arg) > 0) {
        int result = control_restart_tool(arg);
        if (result == FW_OK) {
            Tool* tool = tool_find(arg);
            snprintf(response, response_size,
                    "Success: Tool '%s' restarted\n  PID: %d\n",
                    arg, tool ? (int)tool->pid : 0);
        } else {
            snprintf(response, response_size,
                    "Error: Failed to restart tool '%s'\n", arg);
        }
        return result;
    }
    else if (strcmp(cmd, "status") == 0 && strlen(arg) > 0) {
        ControlToolInfo info;
        int result = control_get_tool_status(arg, &info);
        
        if (result != FW_OK) {
            snprintf(response, response_size,
                    "Error: Tool '%s' not found\n", arg);
            return result;
        }
        
        int offset = 0;
        offset += snprintf(response + offset, response_size - offset,
                         "\nTool Status:\n");
        offset += snprintf(response + offset, response_size - offset,
                         "  Name: %s\n", info.name);
        offset += snprintf(response + offset, response_size - offset,
                         "  Command: %s\n", info.command);
        if (strlen(info.description) > 0) {
            offset += snprintf(response + offset, response_size - offset,
                             "  Description: %s\n", info.description);
        }
        offset += snprintf(response + offset, response_size - offset,
                         "  Status: %s\n",
                         info.status == TOOL_RUNNING ? "RUNNING" :
                         info.status == TOOL_STOPPED ? "STOPPED" :
                         info.status == TOOL_CRASHED ? "CRASHED" : "UNKNOWN");
        offset += snprintf(response + offset, response_size - offset,
                         "  PID: %d\n", (int)info.pid);
        offset += snprintf(response + offset, response_size - offset,
                         "  Autostart: %s\n", info.autostart ? "yes" : "no");
        offset += snprintf(response + offset, response_size - offset,
                         "  Restart on crash: %s\n", info.restart_on_crash ? "yes" : "no");
        // FIXED: Use PRIu64 for uint64_t
        offset += snprintf(response + offset, response_size - offset,
                         "  Events sent: %" PRIu64 "\n", info.events_sent);
        offset += snprintf(response + offset, response_size - offset,
                         "  Events received: %" PRIu64 "\n", info.events_received);
        snprintf(response + offset, response_size - offset, "\n");
        return FW_OK;
    }
    else if (strcmp(cmd, "shutdown") == 0) {
        snprintf(response, response_size, "Shutting down framework...\n");
        return control_shutdown_framework();
    }
    else if (strcmp(cmd, "uptime") == 0) {
        uint64_t uptime = control_get_uptime();
        uint64_t hours = uptime / 3600;
        uint64_t minutes = (uptime % 3600) / 60;
        uint64_t seconds = uptime % 60;
        snprintf(response, response_size,
                "Framework uptime: %" PRIu64 "h %" PRIu64 "m %" PRIu64 "s\n",
                hours, minutes, seconds);
        return FW_OK;
    }
    else if (strcmp(cmd, "version") == 0) {
        snprintf(response, response_size,
                "Yuki-Frame version %s\n", control_get_version());
        return FW_OK;
    }
    else if (strcmp(cmd, "help") == 0) {
        snprintf(response, response_size,
                "\nAvailable commands:\n"
                "  list                 - List all tools and their status\n"
                "  start <tool>         - Start a tool\n"
                "  stop <tool>          - Stop a tool\n"
                "  restart <tool>       - Restart a tool\n"
                "  status <tool>        - Show detailed tool status\n"
                "  uptime               - Show framework uptime\n"
                "  version              - Show framework version\n"
                "  shutdown             - Shutdown the framework\n"
                "  help                 - Show this help message\n"
                "\n");
        return FW_OK;
    }
    else {
        snprintf(response, response_size,
                "Error: Unknown command '%s'\nType 'help' for available commands\n",
                cmd);
        return FW_ERROR_INVALID_ARG;
    }
}
