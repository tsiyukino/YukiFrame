/**
 * @file main.c
 * @brief Main entry point for Yuki-Frame v2.0 (Windows)
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
#include "yuki_frame/control_socket.h"
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

// Signal handler (Windows-style)
void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM || sig == SIGABRT) {
        LOG_INFO("main", "Received shutdown signal");
        g_running = false;
    }
}

// Windows Console Control Handler
BOOL WINAPI console_ctrl_handler(DWORD ctrl_type) {
    switch (ctrl_type) {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            LOG_INFO("main", "Received Windows console control signal");
            g_running = false;
            return TRUE;
        default:
            return FALSE;
    }
}

// Print usage
void print_usage(const char* prog_name) {
    printf("%s v%s - Event-driven tool orchestration framework for Windows\n\n", 
           YUKI_FRAME_NAME, YUKI_FRAME_VERSION_STRING);
    printf("Usage: %s [OPTIONS]\n\n", prog_name);
    printf("Options:\n");
    printf("  -c, --config FILE    Configuration file (default: yuki-frame.conf)\n");
    printf("  -p, --port PORT      Control socket port (default: 9999)\n");
    printf("  -h, --help           Show this help message\n");
    printf("  -v, --version        Show version information\n");
    printf("  -d, --debug          Enable debug mode\n");
    printf("\n");
    printf("Control Interface:\n");
    printf("  The framework includes an integrated control socket server.\n");
    printf("  Connect using:\n");
    printf("    python yuki-console.py\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s -c yuki-frame.conf\n", prog_name);
    printf("  %s -c yuki-frame.conf -p 8888   # Custom port\n", prog_name);
    printf("  %s -c yuki-frame.conf -d        # With debug mode\n", prog_name);
    printf("\n");
}

// Initialize framework
int framework_init(const char* config_file, int control_port) {
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
    
    LOG_INFO("main", "========================================");
    LOG_INFO("main", "%s v%s starting on Windows", YUKI_FRAME_NAME, YUKI_FRAME_VERSION_STRING);
    LOG_INFO("main", "========================================");
    
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
    
    // ================================================================
    // INTEGRATED CONTROL SOCKET (PART OF FRAMEWORK!)
    // ================================================================
    
    // Initialize control socket
    ret = control_socket_init();
    if (ret != FW_OK) {
        LOG_ERROR("main", "Failed to initialize control socket");
        return ret;
    }
    
    // Start control socket server
    ret = control_socket_start(control_port);
    if (ret != FW_OK) {
        LOG_ERROR("main", "Failed to start control socket server");
        return ret;
    }
    
    LOG_INFO("main", "========================================");
    LOG_INFO("main", "Control Socket Server: ACTIVE");
    LOG_INFO("main", "Listening on: localhost:%d", control_port);
    LOG_INFO("main", "Connect using: python yuki-console.py");
    LOG_INFO("main", "========================================");
    
    // ================================================================
    
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
                char* subs = _strdup(tools[i].subscriptions);
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
    return FW_OK;
}

// Forward declaration
void handle_console_command(const char* tool_name, const char* command);

// Main loop
void framework_run(void) {
    LOG_INFO("main", "Entering main loop");
    
    char buffer[4096];
    char line_buffer[8192];  // Buffer for accumulating partial lines
    int line_pos = 0;
    
    while (g_running) {
        // Process events
        event_process_queue();
        
        // Read from tool stdout/stderr
        Tool* tool = tool_get_first();
        while (tool) {
            if (tool->status == TOOL_RUNNING) {
                // Read stdout (events and commands)
                if (tool->stdout_fd >= 0) {
                    int bytes = platform_read_nonblocking(tool->stdout_fd, buffer, sizeof(buffer) - 1);
                    if (bytes > 0) {
                        buffer[bytes] = '\0';
                        
                        // Accumulate into line buffer
                        for (int i = 0; i < bytes && line_pos < (int)sizeof(line_buffer) - 1; i++) {
                            if (buffer[i] == '\n') {
                                line_buffer[line_pos] = '\0';
                                
                                // Parse event: TYPE|sender|data
                                char* type = strtok(line_buffer, "|");
                                char* sender = strtok(NULL, "|");
                                char* data = strtok(NULL, "");
                                
                                if (type && sender && data) {
                                    if (strcmp(type, "COMMAND") == 0) {
                                        // Console command - handle it
                                        LOG_DEBUG("main", "Command from %s: %s", sender, data);
                                        handle_console_command(sender, data);
                                    } else {
                                        // Regular event - publish to event bus
                                        LOG_DEBUG("main", "Event from %s: %s|%s", sender, type, data);
                                        event_publish(type, sender, data);
                                    }
                                }
                                
                                line_pos = 0;
                            } else {
                                line_buffer[line_pos++] = buffer[i];
                            }
                        }
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
    LOG_INFO("main", "========================================");
    LOG_INFO("main", "Shutting down framework");
    LOG_INFO("main", "========================================");
    
    // Stop control socket server (PART OF FRAMEWORK!)
    control_socket_stop();
    control_socket_shutdown();
    LOG_INFO("main", "Control socket server stopped");
    
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

// Handle console commands
void handle_console_command(const char* tool_name, const char* command) {
    char response[8192];
    response[0] = '\0';
    
    // Parse command
    char cmd_copy[1024];
    strncpy(cmd_copy, command, sizeof(cmd_copy) - 1);
    cmd_copy[sizeof(cmd_copy) - 1] = '\0';
    
    char* cmd = strtok(cmd_copy, " ");
    char* arg1 = strtok(NULL, " ");
    char* arg2 = strtok(NULL, " ");
    
    if (!cmd) {
        snprintf(response, sizeof(response), "RESPONSE|framework|Error: Empty command\n");
        tool_send_event(tool_name, response);
        return;
    }
    
    // Convert to lowercase
    for (char* p = cmd; *p; p++) {
        *p = (char)tolower(*p);
    }
    
    // Execute command
    if (strcmp(cmd, "list") == 0) {
        int offset = 0;
        offset += snprintf(response + offset, sizeof(response) - offset,
                          "\nTools Status:\n");
        offset += snprintf(response + offset, sizeof(response) - offset,
                          "%-20s %-10s %-10s\n", "Name", "Status", "PID");
        offset += snprintf(response + offset, sizeof(response) - offset,
                          "------------------------------------------------------------\n");
        
        Tool* tool = tool_get_first();
        while (tool && offset < (int)sizeof(response) - 100) {
            const char* status_str;
            switch (tool->status) {
                case TOOL_STOPPED: status_str = "STOPPED"; break;
                case TOOL_RUNNING: status_str = "RUNNING"; break;
                case TOOL_CRASHED: status_str = "CRASHED"; break;
                case TOOL_ERROR:   status_str = "ERROR";   break;
                default:           status_str = "UNKNOWN"; break;
            }
            
            offset += snprintf(response + offset, sizeof(response) - offset,
                             "%-20s %-10s %-10d\n", tool->name, status_str, (int)tool->pid);
            tool = tool_get_next();
        }
        snprintf(response + offset, sizeof(response) - offset, "\n");
    }
    else if (strcmp(cmd, "start") == 0 && arg1) {
        int result = tool_start(arg1);
        if (result == FW_OK) {
            Tool* tool = tool_find(arg1);
            snprintf(response, sizeof(response),
                    "Success: Tool '%s' started\n  PID: %d\n  Status: RUNNING\n",
                    arg1, tool ? (int)tool->pid : 0);
        } else if (result == FW_ERROR_NOT_FOUND) {
            snprintf(response, sizeof(response),
                    "Error: Tool '%s' not found in configuration\n", arg1);
        } else {
            snprintf(response, sizeof(response),
                    "Error: Failed to start tool '%s'\n", arg1);
        }
    }
    else if (strcmp(cmd, "stop") == 0 && arg1) {
        int result = tool_stop(arg1);
        if (result == FW_OK) {
            snprintf(response, sizeof(response),
                    "Success: Tool '%s' stopped\n", arg1);
        } else {
            snprintf(response, sizeof(response),
                    "Error: Failed to stop tool '%s'\n", arg1);
        }
    }
    else if (strcmp(cmd, "restart") == 0 && arg1) {
        int result = tool_restart(arg1);
        if (result == FW_OK) {
            Tool* tool = tool_find(arg1);
            snprintf(response, sizeof(response),
                    "Success: Tool '%s' restarted\n  PID: %d\n",
                    arg1, tool ? (int)tool->pid : 0);
        } else {
            snprintf(response, sizeof(response),
                    "Error: Failed to restart tool '%s'\n", arg1);
        }
    }
    else if (strcmp(cmd, "status") == 0 && arg1) {
        Tool* tool = tool_find(arg1);
        
        if (!tool) {
            snprintf(response, sizeof(response),
                    "Error: Tool '%s' not found\n", arg1);
        } else {
            int offset = 0;
            offset += snprintf(response + offset, sizeof(response) - offset,
                             "\nTool Status:\n");
            offset += snprintf(response + offset, sizeof(response) - offset,
                             "  Name: %s\n", tool->name);
            offset += snprintf(response + offset, sizeof(response) - offset,
                             "  Command: %s\n", tool->command);
            if (strlen(tool->description) > 0) {
                offset += snprintf(response + offset, sizeof(response) - offset,
                                 "  Description: %s\n", tool->description);
            }
            offset += snprintf(response + offset, sizeof(response) - offset,
                             "  Status: %s\n",
                             tool->status == TOOL_RUNNING ? "RUNNING" :
                             tool->status == TOOL_STOPPED ? "STOPPED" :
                             tool->status == TOOL_CRASHED ? "CRASHED" : "UNKNOWN");
            offset += snprintf(response + offset, sizeof(response) - offset,
                             "  PID: %d\n", (int)tool->pid);
            offset += snprintf(response + offset, sizeof(response) - offset,
                             "  Autostart: %s\n", tool->autostart ? "yes" : "no");
            offset += snprintf(response + offset, sizeof(response) - offset,
                             "  Restart on crash: %s\n", tool->restart_on_crash ? "yes" : "no");
            offset += snprintf(response + offset, sizeof(response) - offset,
                             "  Events sent: %lu\n", tool->events_sent);
            offset += snprintf(response + offset, sizeof(response) - offset,
                             "  Events received: %lu\n", tool->events_received);
            snprintf(response + offset, sizeof(response) - offset, "\n");
        }
    }
    else if (strcmp(cmd, "shutdown") == 0) {
        snprintf(response, sizeof(response), "Shutting down framework...\n");
        g_running = false;
    }
    else if (strcmp(cmd, "uptime") == 0) {
        // Calculate uptime
        static time_t start_time = 0;
        if (start_time == 0) start_time = time(NULL);
        
        time_t now = time(NULL);
        uint64_t uptime = (uint64_t)(now - start_time);
        uint64_t hours = uptime / 3600;
        uint64_t minutes = (uptime % 3600) / 60;
        uint64_t seconds = uptime % 60;
        
        snprintf(response, sizeof(response),
                "Framework uptime: %lluh %llum %llus\n",
                hours, minutes, seconds);
    }
    else if (strcmp(cmd, "version") == 0) {
        snprintf(response, sizeof(response),
                "Yuki-Frame version %s\n", framework_version());
    }
    else if (strcmp(cmd, "help") == 0) {
        snprintf(response, sizeof(response),
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
    }
    else {
        snprintf(response, sizeof(response),
                "Error: Unknown command '%s'\nType 'help' for available commands\n",
                cmd);
    }
    
    // Send response back to console tool
    char response_event[8192];
    snprintf(response_event, sizeof(response_event), "RESPONSE|framework|%s", response);
    tool_send_event(tool_name, response_event);
}

// Main entry point
int main(int argc, char** argv) {
    const char* config_file = "yuki-frame.conf";
    int control_port = 9999;  // Default control port
    bool debug_mode = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("%s v%s (Windows)\n", YUKI_FRAME_NAME, YUKI_FRAME_VERSION_STRING);
            printf("Integrated Control Socket Server\n");
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
        else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0) {
            if (i + 1 < argc) {
                control_port = atoi(argv[++i]);
                if (control_port <= 0 || control_port > 65535) {
                    fprintf(stderr, "Error: Invalid port number\n");
                    return 1;
                }
            } else {
                fprintf(stderr, "Error: -p requires a port number\n");
                return 1;
            }
        }
        else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0) {
            debug_mode = true;
        }
    }
    
    // Print banner
    printf("\n");
    printf("========================================\n");
    printf("  %s v%s\n", YUKI_FRAME_NAME, YUKI_FRAME_VERSION_STRING);
    printf("  Event-driven tool orchestration\n");
    printf("========================================\n");
    printf("\n");
    
    // Setup signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGABRT, signal_handler);
    
    // Setup Windows console control handler
    SetConsoleCtrlHandler(console_ctrl_handler, TRUE);
    
    // Set debug flag before framework_init()
    if (debug_mode) {
        g_config.enable_debug = true;
    }
    
    // Initialize framework (including Control Socket Server!)
    if (framework_init(config_file, control_port) != FW_OK) {
        fprintf(stderr, "Failed to initialize framework\n");
        return 1;
    }
    
    // Override logger level if debug mode
    if (debug_mode) {
        logger_set_level(LOG_DEBUG);
    }
    
    printf("Framework is running. Press Ctrl+C to shutdown.\n");
    printf("Connect console: python yuki-console.py\n");
    printf("\n");
    
    // Run main loop
    framework_run();
    
    // Shutdown
    framework_shutdown();
    
    return 0;
}
