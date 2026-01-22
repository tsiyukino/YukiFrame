/**
 * @file main.c
 * @brief Main entry point for Yuki-Frame v2.0
 * 
 * This version includes integrated control via command file monitoring.
 * No separate control utility needed - all control is built into the framework.
 */

#include "yuki_frame/framework.h"
#include "yuki_frame/logger.h"
#include "yuki_frame/config.h"
#include "yuki_frame/tool.h"
#include "yuki_frame/event.h"
#include "yuki_frame/platform.h"
#include "yuki_frame/control_api.h"
#include "yuki_frame/console.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>

// Declare control_api_init (internal function)
void control_api_init(void);

// Global state
FrameworkConfig g_config;
bool g_running = true;
bool g_enable_console = false;  // Global console enable flag

// Signal handler
void signal_handler(int sig) {
    switch (sig) {
        case SIGINT:
        case SIGTERM:
            LOG_INFO("main", "Received shutdown signal");
            g_running = false;
            break;
#ifndef PLATFORM_WINDOWS
        case SIGHUP:
            LOG_INFO("main", "Received reload signal");
            config_reload();
            break;
#endif
    }
}

// Print usage
void print_usage(const char* prog_name) {
    printf("%s v%s - Event-driven tool orchestration framework\n\n", 
           YUKI_FRAME_NAME, YUKI_FRAME_VERSION_STRING);
    printf("Usage: %s [OPTIONS]\n\n", prog_name);
    printf("Options:\n");
    printf("  -c, --config FILE    Configuration file (default: yuki-frame.conf)\n");
    printf("  -h, --help          Show this help message\n");
    printf("  -v, --version       Show version information\n");
    printf("  -d, --debug         Enable debug mode\n");
    printf("  -i, --interactive   Enable interactive console\n");
    printf("\n");
    printf("Interactive Console:\n");
    printf("  When enabled, you can type commands directly:\n");
    printf("    list, start <tool>, stop <tool>, status <tool>, etc.\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s -c yuki-frame.conf\n", prog_name);
    printf("  %s -c yuki-frame.conf -i     # With interactive console\n", prog_name);
    printf("\n");
}

// Initialize framework
int framework_init(const char* config_file) {
    int ret;
    
    // Load configuration
    ret = config_load(config_file);
    if (ret != FW_OK) {
        fprintf(stderr, "Failed to load configuration: %s\n", config_file);
        return ret;
    }
    
    // Initialize logger
    ret = logger_init(g_config.log_file, g_config.log_level);
    if (ret != FW_OK) {
        fprintf(stderr, "Failed to initialize logger\n");
        return ret;
    }
    
    LOG_INFO("main", "%s v%s starting", YUKI_FRAME_NAME, YUKI_FRAME_VERSION_STRING);
    
    // Initialize platform-specific code
    ret = platform_init();
    if (ret != FW_OK) {
        LOG_ERROR("main", "Failed to initialize platform layer");
        return ret;
    }
    
    // Initialize event bus
    ret = event_bus_init();
    if (ret != FW_OK) {
        LOG_ERROR("main", "Failed to initialize event bus");
        return ret;
    }
    
    // Initialize tool registry
    ret = tool_registry_init();
    if (ret != FW_OK) {
        LOG_ERROR("main", "Failed to initialize tool registry");
        return ret;
    }
    
    // Initialize control system
    ret = control_init();
    if (ret != FW_OK) {
        LOG_ERROR("main", "Failed to initialize control system");
        return ret;
    }
    
    // Initialize debug system
    if (g_config.enable_debug) {
        debug_init();
        LOG_INFO("main", "Debug mode enabled");
    }
    
    // Initialize control API
    control_api_init();
    LOG_INFO("main", "Control API initialized");
    
    // Initialize console if enabled
    if (g_enable_console) {
        ret = console_init();
        if (ret != FW_OK) {
            LOG_ERROR("main", "Failed to initialize console");
            return ret;
        }
    }
    
    // Load and register tools from configuration
    ToolConfig* tools;
    int tool_count;
    ret = config_get_tools(&tools, &tool_count);
    if (ret == FW_OK) {
        LOG_INFO("main", "Found %d tools in configuration", tool_count);
        
        for (int i = 0; i < tool_count; i++) {
            LOG_INFO("main", "Registering tool: %s", tools[i].name);
            tool_register(tools[i].name, tools[i].command);
            
            // Subscribe to events
            if (strlen(tools[i].subscriptions) > 0) {
                char* subs = strdup(tools[i].subscriptions);
                char* token = strtok(subs, ",");
                while (token) {
                    // Trim whitespace
                    while (*token == ' ') token++;
                    tool_subscribe(tools[i].name, token);
                    token = strtok(NULL, ",");
                }
                free(subs);
            }
            
            // Autostart if configured
            if (tools[i].autostart) {
                LOG_INFO("main", "Auto-starting tool: %s", tools[i].name);
                int start_result = tool_start(tools[i].name);
                if (start_result != FW_OK) {
                    LOG_ERROR("main", "Failed to start tool: %s (error %d)", tools[i].name, start_result);
                }
            }
        }
        config_free_tools(tools, tool_count);
    } else {
        LOG_WARN("main", "No tools found in configuration");
    }
    
    LOG_INFO("main", "Framework initialized successfully");
    if (g_enable_console) {
        LOG_INFO("main", "Interactive console available (will start after main loop begins)");
    }
    return FW_OK;
}

// Main loop
void framework_run(void) {
    LOG_INFO("main", "Entering main loop");
    
    // Start console if enabled
    if (g_enable_console) {
        console_start();
    }
    
    char buffer[4096];
    
    while (g_running) {
        // Process events
        event_process_queue();
        
        // Read from tool stdout/stderr
        Tool* tool = tool_get_first();
        while (tool) {
            if (tool->status == TOOL_RUNNING) {
                // Read stdout
                if (tool->stdout_fd >= 0) {
                    int bytes = platform_read_nonblocking(tool->stdout_fd, buffer, sizeof(buffer) - 1);
                    if (bytes > 0) {
                        buffer[bytes] = '\0';
                        LOG_DEBUG("main", "Tool %s stdout: %s", tool->name, buffer);
                        // TODO: Parse as event and publish
                    }
                }
                
                // Read stderr (logs)
                if (tool->stderr_fd >= 0) {
                    int bytes = platform_read_nonblocking(tool->stderr_fd, buffer, sizeof(buffer) - 1);
                    if (bytes > 0) {
                        buffer[bytes] = '\0';
                        // Remove trailing newline
                        char* newline = strchr(buffer, '\n');
                        if (newline) *newline = '\0';
                        LOG_INFO(tool->name, "%s", buffer);
                    }
                }
            }
            
            tool = tool_get_next();
        }
        
        // Check tool health
        tool_check_health();
        
        // Sleep briefly to avoid busy-waiting
        platform_sleep_ms(100);
    }
    
    LOG_INFO("main", "Main loop exited");
}

// Shutdown framework
void framework_shutdown(void) {
    LOG_INFO("main", "Shutting down framework");
    
    // Shutdown console
    if (g_enable_console) {
        console_shutdown();
    }
    
    // Stop all tools
    Tool* tool = tool_get_first();
    while (tool) {
        if (tool->status == TOOL_RUNNING) {
            tool_stop(tool->name);
        }
        tool = tool_get_next();
    }
    
    // Shutdown subsystems
    if (g_config.enable_debug) {
        debug_shutdown();
    }
    control_shutdown();
    tool_registry_shutdown();
    event_bus_shutdown();
    platform_shutdown();
    logger_shutdown();
    
    printf("%s shutdown complete\n", YUKI_FRAME_NAME);
}

// Get version
const char* framework_version(void) {
    return YUKI_FRAME_VERSION_STRING;
}

// Main entry point
int main(int argc, char** argv) {
    const char* config_file = "yuki-frame.conf";
    bool debug_mode = false;
    bool interactive_mode = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("%s v%s\n", YUKI_FRAME_NAME, YUKI_FRAME_VERSION_STRING);
            return 0;
        }
        else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config") == 0) {
            if (i + 1 < argc) {
                config_file = argv[++i];
            } else {
                fprintf(stderr, "Error: -c requires a filename\n");
                return 1;
            }
        }
        else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0) {
            debug_mode = true;
        }
        else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--interactive") == 0) {
            interactive_mode = true;
        }
    }
    
    // Setup signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
#ifndef PLATFORM_WINDOWS
    signal(SIGHUP, signal_handler);
#endif
    
    // Initialize framework
    if (framework_init(config_file) != FW_OK) {
        fprintf(stderr, "Failed to initialize framework\n");
        return 1;
    }
    
    // Override debug if command line flag set
    if (debug_mode) {
        g_config.enable_debug = true;
        logger_set_level(LOG_DEBUG);
        debug_init();
    }
    
    // Override console if command line flag set
    if (interactive_mode) {
        g_enable_console = true;
    }
    
    // Run main loop
    framework_run();
    
    // Shutdown
    framework_shutdown();
    
    return 0;
}
