/**
 * @file cli_control.c
 * @brief Command-line control utility for Yuki-Frame
 * 
 * This utility allows manual control of tools while the framework is running.
 * It operates independently and does not interfere with the running framework.
 */

#include "yuki_frame/framework.h"
#include "yuki_frame/logger.h"
#include "yuki_frame/config.h"
#include "yuki_frame/tool.h"
#include "yuki_frame/platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global state
FrameworkConfig g_config;
bool g_running = true;

void print_usage(const char* prog_name) {
    printf("Yuki-Frame Control Utility v%s\n\n", YUKI_FRAME_VERSION_STRING);
    printf("Usage: %s [COMMAND] [TOOL_NAME]\n\n", prog_name);
    printf("Commands:\n");
    printf("  start <tool>     Start a tool\n");
    printf("  stop <tool>      Stop a tool\n");
    printf("  restart <tool>   Restart a tool\n");
    printf("  list             List all registered tools\n");
    printf("  status <tool>    Show detailed status of a tool\n");
    printf("  help             Show this help message\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s start my_tool\n", prog_name);
    printf("  %s stop my_tool\n", prog_name);
    printf("  %s list\n", prog_name);
    printf("  %s status my_tool\n", prog_name);
    printf("\n");
}

// Initialize minimal framework components for control utility
int control_util_init(const char* config_file) {
    int ret;
    
    // Load configuration
    ret = config_load(config_file);
    if (ret != FW_OK) {
        fprintf(stderr, "Failed to load configuration: %s\n", config_file);
        return ret;
    }
    
    // Initialize platform (needed for process operations)
    ret = platform_init();
    if (ret != FW_OK) {
        fprintf(stderr, "Failed to initialize platform layer\n");
        return ret;
    }
    
    // Initialize tool registry (but DON'T register tools yet)
    ret = tool_registry_init();
    if (ret != FW_OK) {
        fprintf(stderr, "Failed to initialize tool registry\n");
        return ret;
    }
    
    // Load and register tools from configuration
    ToolConfig* tools;
    int tool_count;
    ret = config_get_tools(&tools, &tool_count);
    if (ret == FW_OK && tools) {
        for (int i = 0; i < tool_count; i++) {
            tool_register(tools[i].name, tools[i].command);
            
            // Subscribe to events
            if (strlen(tools[i].subscriptions) > 0) {
                char* subs = strdup(tools[i].subscriptions);
                char* token = strtok(subs, ",");
                while (token) {
                    while (*token == ' ') token++;
                    tool_subscribe(tools[i].name, token);
                    token = strtok(NULL, ",");
                }
                free(subs);
            }
        }
        config_free_tools(tools, tool_count);
    }
    
    return FW_OK;
}

// Clean shutdown (WITHOUT stopping tools)
void control_util_shutdown(void) {
    // DON'T call tool_registry_shutdown() - it would stop all tools
    // Just cleanup platform resources
    platform_shutdown();
}

int cmd_start(const char* tool_name) {
    printf("Starting tool: %s\n", tool_name);
    
    int result = tool_start(tool_name);
    
    if (result == FW_OK) {
        printf("Tool '%s' started successfully\n", tool_name);
        return 0;
    } else if (result == FW_ERROR_NOT_FOUND) {
        fprintf(stderr, "Error: Tool '%s' not found in configuration\n", tool_name);
        return 1;
    } else {
        fprintf(stderr, "Error: Failed to start tool '%s' (error code: %d)\n", tool_name, result);
        return 1;
    }
}

int cmd_stop(const char* tool_name) {
    printf("Stopping tool: %s\n", tool_name);
    
    int result = tool_stop(tool_name);
    
    if (result == FW_OK) {
        printf("Tool '%s' stopped successfully\n", tool_name);
        return 0;
    } else if (result == FW_ERROR_NOT_FOUND) {
        fprintf(stderr, "Error: Tool '%s' not found\n", tool_name);
        return 1;
    } else {
        fprintf(stderr, "Error: Failed to stop tool '%s'\n", tool_name);
        return 1;
    }
}

int cmd_restart(const char* tool_name) {
    printf("Restarting tool: %s\n", tool_name);
    
    int result = tool_restart(tool_name);
    
    if (result == FW_OK) {
        printf("Tool '%s' restarted successfully\n", tool_name);
        return 0;
    } else if (result == FW_ERROR_NOT_FOUND) {
        fprintf(stderr, "Error: Tool '%s' not found\n", tool_name);
        return 1;
    } else {
        fprintf(stderr, "Error: Failed to restart tool '%s'\n", tool_name);
        return 1;
    }
}

int cmd_list(void) {
    printf("\nRegistered Tools:\n");
    printf("-----------------\n");
    
    Tool* tool = tool_get_first();
    if (!tool) {
        printf("No tools registered\n");
        return 0;
    }
    
    while (tool) {
        const char* status_str;
        switch (tool->status) {
            case TOOL_STOPPED: status_str = "STOPPED"; break;
            case TOOL_RUNNING: status_str = "RUNNING"; break;
            case TOOL_CRASHED: status_str = "CRASHED"; break;
            case TOOL_ERROR:   status_str = "ERROR";   break;
            default:           status_str = "UNKNOWN"; break;
        }
        
        printf("%-20s %-10s PID: %d\n", 
               tool->name, 
               status_str,
               (int)tool->pid);
        
        tool = tool_get_next();
    }
    printf("\n");
    
    return 0;
}

int cmd_status(const char* tool_name) {
    Tool* tool = tool_find(tool_name);
    
    if (!tool) {
        fprintf(stderr, "Error: Tool '%s' not found\n", tool_name);
        return 1;
    }
    
    printf("\nTool Status:\n");
    printf("------------\n");
    printf("Tool: %s\n", tool->name);
    printf("Command: %s\n", tool->command);
    printf("Description: %s\n", tool->description[0] ? tool->description : "(none)");
    
    const char* status_str;
    switch (tool->status) {
        case TOOL_STOPPED: status_str = "STOPPED"; break;
        case TOOL_RUNNING: status_str = "RUNNING"; break;
        case TOOL_CRASHED: status_str = "CRASHED"; break;
        case TOOL_ERROR:   status_str = "ERROR";   break;
        default:           status_str = "UNKNOWN"; break;
    }
    printf("Status: %s\n", status_str);
    printf("PID: %d\n", (int)tool->pid);
    printf("Events Sent: %lu\n", tool->events_sent);
    printf("Events Received: %lu\n", tool->events_received);
    printf("Restart Count: %d\n", tool->restart_count);
    printf("Max Restarts: %d\n", tool->max_restarts);
    printf("Autostart: %s\n", tool->autostart ? "yes" : "no");
    printf("Restart on Crash: %s\n", tool->restart_on_crash ? "yes" : "no");
    
    if (tool->subscription_count > 0) {
        printf("Subscriptions:\n");
        for (int i = 0; i < tool->subscription_count; i++) {
            printf("  - %s\n", tool->subscriptions[i]);
        }
    }
    
    printf("\n");
    return 0;
}

int main(int argc, char** argv) {
    const char* config_file = "yuki-frame.conf";
    
    // Parse arguments
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    const char* command = argv[1];
    
    // Check for config file override
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config") == 0) {
            if (i + 1 < argc) {
                config_file = argv[i + 1];
            }
        }
    }
    
    // Handle help command without initializing
    if (strcmp(command, "help") == 0 || strcmp(command, "-h") == 0 || strcmp(command, "--help") == 0) {
        print_usage(argv[0]);
        return 0;
    }
    
    // Initialize control utility
    if (control_util_init(config_file) != FW_OK) {
        fprintf(stderr, "Failed to initialize control utility\n");
        return 1;
    }
    
    int result = 0;
    
    // Execute command
    if (strcmp(command, "start") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: 'start' command requires a tool name\n");
            print_usage(argv[0]);
            result = 1;
        } else {
            result = cmd_start(argv[2]);
        }
    }
    else if (strcmp(command, "stop") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: 'stop' command requires a tool name\n");
            print_usage(argv[0]);
            result = 1;
        } else {
            result = cmd_stop(argv[2]);
        }
    }
    else if (strcmp(command, "restart") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: 'restart' command requires a tool name\n");
            print_usage(argv[0]);
            result = 1;
        } else {
            result = cmd_restart(argv[2]);
        }
    }
    else if (strcmp(command, "list") == 0) {
        result = cmd_list();
    }
    else if (strcmp(command, "status") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: 'status' command requires a tool name\n");
            print_usage(argv[0]);
            result = 1;
        } else {
            result = cmd_status(argv[2]);
        }
    }
    else {
        fprintf(stderr, "Error: Unknown command '%s'\n", command);
        print_usage(argv[0]);
        result = 1;
    }
    
    // Shutdown WITHOUT stopping tools
    control_util_shutdown();
    
    return result;
}
