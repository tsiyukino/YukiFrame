#include "yuki_frame/framework.h"
#include "yuki_frame/tool.h"
#include "yuki_frame/logger.h"
#include "yuki_frame/platform.h"
#include <stdlib.h>
#include <string.h>

#ifdef PLATFORM_WINDOWS
    #include <io.h>
    #define close _close
#else
    #include <unistd.h>
#endif

static ToolRegistry registry;
static int current_iterator = 0;

int tool_registry_init(void) {
    memset(&registry, 0, sizeof(registry));
    registry.count = 0;
    LOG_INFO("tool", "Tool registry initialized");
    return FW_OK;
}

void tool_registry_shutdown(void) {
    // Stop all running tools
    for (int i = 0; i < registry.count; i++) {
        if (registry.tools[i] && registry.tools[i]->status == TOOL_RUNNING) {
            tool_stop(registry.tools[i]->name);
        }
    }
    
    // Free all tools
    for (int i = 0; i < registry.count; i++) {
        if (registry.tools[i]) {
            // Free subscriptions
            for (int j = 0; j < registry.tools[i]->subscription_count; j++) {
                if (registry.tools[i]->subscriptions[j]) {
                    free(registry.tools[i]->subscriptions[j]);
                }
            }
            free(registry.tools[i]);
            registry.tools[i] = NULL;
        }
    }
    registry.count = 0;
    LOG_INFO("tool", "Tool registry shutdown");
}

int tool_register(const char* name, const char* command) {
    if (!name || !command) {
        return FW_ERROR_INVALID_ARG;
    }
    
    // Check if tool already exists
    if (tool_find(name) != NULL) {
        LOG_ERROR("tool", "Tool already registered: %s", name);
        return FW_ERROR_ALREADY_EXISTS;
    }
    
    if (registry.count >= MAX_TOOLS) {
        LOG_ERROR("tool", "Maximum number of tools reached");
        return FW_ERROR_GENERIC;
    }
    
    Tool* tool = (Tool*)malloc(sizeof(Tool));
    if (!tool) {
        return FW_ERROR_MEMORY;
    }
    
    memset(tool, 0, sizeof(Tool));
    strncpy(tool->name, name, MAX_TOOL_NAME - 1);
    strncpy(tool->command, command, MAX_COMMAND_LENGTH - 1);
    tool->status = TOOL_STOPPED;
    tool->autostart = false;
    tool->restart_on_crash = false;
    tool->max_restarts = 3;
    tool->restart_count = 0;
    tool->subscription_count = 0;
    tool->stdin_fd = -1;
    tool->stdout_fd = -1;
    tool->stderr_fd = -1;
    tool->events_sent = 0;
    tool->events_received = 0;
    tool->log_lines = 0;
    
    registry.tools[registry.count++] = tool;
    
    LOG_INFO("tool", "Registered tool: %s", name);
    return FW_OK;
}

int tool_unregister(const char* name) {
    Tool* tool = tool_find(name);
    if (!tool) {
        return FW_ERROR_NOT_FOUND;
    }
    
    // Stop tool if running
    if (tool->status == TOOL_RUNNING) {
        tool_stop(name);
    }
    
    // Remove from registry
    for (int i = 0; i < registry.count; i++) {
        if (registry.tools[i] == tool) {
            // Free subscriptions
            for (int j = 0; j < tool->subscription_count; j++) {
                if (tool->subscriptions[j]) {
                    free(tool->subscriptions[j]);
                }
            }
            
            free(tool);
            
            // Shift remaining tools
            for (int j = i; j < registry.count - 1; j++) {
                registry.tools[j] = registry.tools[j + 1];
            }
            registry.count--;
            LOG_INFO("tool", "Unregistered tool: %s", name);
            return FW_OK;
        }
    }
    
    return FW_ERROR_NOT_FOUND;
}

Tool* tool_find(const char* name) {
    if (!name) {
        return NULL;
    }
    
    for (int i = 0; i < registry.count; i++) {
        if (strcmp(registry.tools[i]->name, name) == 0) {
            return registry.tools[i];
        }
    }
    
    return NULL;
}

int tool_start(const char* name) {
    Tool* tool = tool_find(name);
    if (!tool) {
        return FW_ERROR_NOT_FOUND;
    }
    
    if (tool->status == TOOL_RUNNING) {
        LOG_WARN("tool", "Tool %s is already running", name);
        return FW_OK;
    }
    
    LOG_INFO("tool", "Starting tool: %s", name);
    
    // Actually spawn the process
    tool->process_handle = platform_spawn_process(
        tool->command,
        &tool->stdin_fd,
        &tool->stdout_fd,
        &tool->stderr_fd
    );
    
#ifdef PLATFORM_WINDOWS
    if (tool->process_handle == INVALID_HANDLE_VALUE) {
#else
    if (tool->process_handle == (ProcessHandle)-1) {
#endif
        LOG_ERROR("tool", "Failed to spawn process for tool: %s", name);
        tool->status = TOOL_ERROR;
        return FW_ERROR_PROCESS_FAILED;
    }
    
    // Set non-blocking I/O
    if (platform_set_nonblocking(tool->stdout_fd) != FW_OK) {
        LOG_WARN("tool", "Failed to set stdout non-blocking for tool: %s", name);
    }
    if (platform_set_nonblocking(tool->stderr_fd) != FW_OK) {
        LOG_WARN("tool", "Failed to set stderr non-blocking for tool: %s", name);
    }
    
    // Update tool status
    tool->status = TOOL_RUNNING;
    tool->started_at = time(NULL);
    tool->pid = platform_get_process_id(tool->process_handle);
    tool->last_heartbeat = time(NULL);
    
    LOG_INFO("tool", "Tool %s started with PID %d", name, (int)tool->pid);
    
    return FW_OK;
}

int tool_stop(const char* name) {
    Tool* tool = tool_find(name);
    if (!tool) {
        return FW_ERROR_NOT_FOUND;
    }
    
    if (tool->status != TOOL_RUNNING) {
        LOG_WARN("tool", "Tool %s is not running", name);
        return FW_OK;
    }
    
    LOG_INFO("tool", "Stopping tool: %s", name);
    
    // Kill the process
    int result = platform_kill_process(tool->process_handle, false);
    
    if (result != FW_OK) {
        LOG_WARN("tool", "Graceful stop failed, forcing...");
        result = platform_kill_process(tool->process_handle, true);
    }
    
    // Wait a bit for process to exit
    platform_wait_process(tool->process_handle, 1000);
    
    // Close file descriptors
    if (tool->stdin_fd >= 0) {
        close(tool->stdin_fd);
        tool->stdin_fd = -1;
    }
    if (tool->stdout_fd >= 0) {
        close(tool->stdout_fd);
        tool->stdout_fd = -1;
    }
    if (tool->stderr_fd >= 0) {
        close(tool->stderr_fd);
        tool->stderr_fd = -1;
    }
    
    tool->status = TOOL_STOPPED;
    tool->pid = 0;
    
    LOG_INFO("tool", "Tool %s stopped", name);
    
    return FW_OK;
}

int tool_restart(const char* name) {
    int ret = tool_stop(name);
    if (ret != FW_OK) {
        return ret;
    }
    
    // Wait a bit before restarting
    platform_sleep_ms(500);
    
    return tool_start(name);
}

int tool_subscribe(const char* name, const char* event_type) {
    Tool* tool = tool_find(name);
    if (!tool) {
        return FW_ERROR_NOT_FOUND;
    }
    
    if (tool->subscription_count >= MAX_SUBSCRIPTIONS) {
        LOG_ERROR("tool", "Maximum subscriptions reached for tool: %s", name);
        return FW_ERROR_GENERIC;
    }
    
    tool->subscriptions[tool->subscription_count] = strdup(event_type);
    tool->subscription_count++;
    
    LOG_DEBUG("tool", "Tool %s subscribed to: %s", name, event_type);
    return FW_OK;
}

int tool_send_event(const char* name, const char* event) {
    Tool* tool = tool_find(name);
    if (!tool) {
        return FW_ERROR_NOT_FOUND;
    }
    
    if (tool->status != TOOL_RUNNING) {
        LOG_WARN("tool", "Cannot send event to stopped tool: %s", name);
        return FW_ERROR_GENERIC;
    }
    
    // Write event to tool's stdin
    size_t len = strlen(event);
    int result = platform_write_nonblocking(tool->stdin_fd, event, len);
    
    if (result < 0) {
        LOG_ERROR("tool", "Failed to send event to tool: %s", name);
        return result;
    }
    
    // Write newline
    platform_write_nonblocking(tool->stdin_fd, "\n", 1);
    
    tool->events_received++;
    
    return FW_OK;
}

bool tool_is_running(const char* name) {
    Tool* tool = tool_find(name);
    if (!tool) {
        return false;
    }
    
    if (tool->status != TOOL_RUNNING) {
        return false;
    }
    
    // Check if process is still alive
    if (!platform_is_process_running(tool->process_handle)) {
        tool->status = TOOL_CRASHED;
        LOG_ERROR("tool", "Tool %s crashed", name);
        return false;
    }
    
    return true;
}

void tool_update_heartbeat(const char* name) {
    Tool* tool = tool_find(name);
    if (tool) {
        tool->last_heartbeat = time(NULL);
    }
}

void tool_check_health(void) {
    for (int i = 0; i < registry.count; i++) {
        Tool* tool = registry.tools[i];
        if (!tool || tool->status != TOOL_RUNNING) {
            continue;
        }
        
        // Check if process is still running
        if (!platform_is_process_running(tool->process_handle)) {
            LOG_ERROR("tool", "Tool %s crashed", tool->name);
            tool->status = TOOL_CRASHED;
            
            // Close file descriptors
            if (tool->stdin_fd >= 0) {
                close(tool->stdin_fd);
                tool->stdin_fd = -1;
            }
            if (tool->stdout_fd >= 0) {
                close(tool->stdout_fd);
                tool->stdout_fd = -1;
            }
            if (tool->stderr_fd >= 0) {
                close(tool->stderr_fd);
                tool->stderr_fd = -1;
            }
            
            // Attempt restart if configured
            if (tool->restart_on_crash && tool->restart_count < tool->max_restarts) {
                LOG_INFO("tool", "Attempting to restart tool: %s (attempt %d/%d)",
                         tool->name, tool->restart_count + 1, tool->max_restarts);
                
                tool->restart_count++;
                platform_sleep_ms(1000); // Wait before restart
                
                if (tool_start(tool->name) == FW_OK) {
                    LOG_INFO("tool", "Tool %s restarted successfully", tool->name);
                } else {
                    LOG_ERROR("tool", "Failed to restart tool: %s", tool->name);
                }
            }
        }
    }
}

Tool* tool_get_first(void) {
    current_iterator = 0;
    if (registry.count > 0) {
        return registry.tools[0];
    }
    return NULL;
}

Tool* tool_get_next(void) {
    current_iterator++;
    if (current_iterator < registry.count) {
        return registry.tools[current_iterator];
    }
    return NULL;
}
