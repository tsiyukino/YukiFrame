#include "yuki_frame/tool.h"
#include "yuki_frame/tool_queue.h"
#include "yuki_frame/logger.h"
#include "yuki_frame/platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <io.h>  // For _open_osfhandle

// Tool registry
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
            // Free queue (NEW!)
            if (registry.tools[i]->inbox) {
                tool_queue_shutdown(registry.tools[i]->inbox);
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
    
    // Check if already registered
    if (tool_find(name)) {
        return FW_ERROR_ALREADY_EXISTS;
    }
    
    if (registry.count >= MAX_TOOLS) {
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
    tool->restart_policy = RESTART_ALWAYS;
    tool->restart_max_delay_sec = 60;
    tool->max_restarts = 3;
    tool->restart_count = 0;
    tool->subscription_count = 0;
    tool->events_sent = 0;
    tool->events_received = 0;
    tool->start_time = 0;
    tool->log_lines = 0;
    
    // Initialize new queue fields (NEW!)
    tool->max_queue_size = 100;  // default
    tool->queue_policy = QUEUE_POLICY_DROP_OLDEST;  // default
    tool->is_on_demand = false;
    tool->is_starting = false;
    tool->inbox = NULL;
    
    // Initialize queue (NEW!)
    int queue_size = tool->max_queue_size;
    int queue_result = tool_queue_init(&tool->inbox, queue_size, tool->queue_policy);
    if (queue_result != FW_OK) {
        LOG_ERROR("tool", "Failed to initialize queue for %s", name);
        free(tool);
        return queue_result;
    }
    
    LOG_DEBUG("tool", "Tool %s queue initialized: size=%d, policy=%d", 
             name, queue_size, tool->queue_policy);
    
    registry.tools[registry.count++] = tool;
    
    LOG_DEBUG("tool", "Registered tool: %s", name);
    return FW_OK;
}

int tool_unregister(const char* name) {
    Tool* tool = tool_find(name);
    if (!tool) {
        return FW_ERROR_NOT_FOUND;
    }
    
    // Stop if running
    if (tool->status == TOOL_RUNNING) {
        tool_stop(name);
    }
    
    // Remove from registry
    for (int i = 0; i < registry.count; i++) {
        if (registry.tools[i] == tool) {
            // Free subscriptions
            for (int j = 0; j < tool->subscription_count; j++) {
                // subscriptions is array of char arrays, don't free individual ones
            }
            
            // Free queue (NEW!)
            if (tool->inbox) {
                tool_queue_shutdown(tool->inbox);
                tool->inbox = NULL;
            }
            
            free(tool);
            
            // Shift array
            for (int j = i; j < registry.count - 1; j++) {
                registry.tools[j] = registry.tools[j + 1];
            }
            registry.count--;
            
            LOG_DEBUG("tool", "Unregistered tool: %s", name);
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
        return FW_OK;  // Already running
    }
    
    LOG_INFO("tool", "Starting tool: %s", name);
    
    tool->status = TOOL_STARTING;
    
    // Spawn process - platform_spawn_process returns fds, not handles
    tool->process_handle = platform_spawn_process(
        tool->command,
        &tool->stdin_fd,
        &tool->stdout_fd,
        &tool->stderr_fd
    );
    
    if (tool->process_handle == INVALID_HANDLE_VALUE) {
        LOG_ERROR("tool", "Failed to start tool: %s", name);
        tool->status = TOOL_ERROR;
        return FW_ERROR_PROCESS_FAILED;
    }
    
    // Get handles from file descriptors
    tool->stdin_handle = (HANDLE)_get_osfhandle(tool->stdin_fd);
    tool->stdout_handle = (HANDLE)_get_osfhandle(tool->stdout_fd);
    tool->stderr_handle = (HANDLE)_get_osfhandle(tool->stderr_fd);
    
    if (tool->stdin_fd < 0 || tool->stdout_fd < 0 || tool->stderr_fd < 0) {
        LOG_ERROR("tool", "Failed to get file descriptors for tool: %s", name);
        platform_kill_process(tool->process_handle, true);
        tool->status = TOOL_ERROR;
        return FW_ERROR_PIPE_FAILED;
    }
    
    tool->status = TOOL_RUNNING;
    tool->started_at = time(NULL);
    tool->pid = platform_get_process_id(tool->process_handle);
    tool->last_heartbeat = time(NULL);
    tool->start_time = time(NULL);
    
    LOG_INFO("tool", "Tool %s started with PID %lu", name, tool->pid);
    
    return FW_OK;
}

int tool_stop(const char* name) {
    Tool* tool = tool_find(name);
    if (!tool) {
        return FW_ERROR_NOT_FOUND;
    }
    
    if (tool->status != TOOL_RUNNING) {
        return FW_OK;  // Already stopped
    }
    
    LOG_INFO("tool", "Stopping tool: %s", name);
    
    tool->status = TOOL_STOPPING;
    
    // Try graceful shutdown first
    int result = platform_kill_process(tool->process_handle, false);
    if (result != FW_OK) {
        // Force kill
        result = platform_kill_process(tool->process_handle, true);
    }
    
    // Wait for process to exit
    platform_wait_process(tool->process_handle, 1000);
    
    // Handle queue (NEW!)
    if (!tool->is_on_demand || !tool->restart_on_crash) {
        // Clear queue if not restarting
        tool_queue_clear(tool->inbox);
        LOG_DEBUG("tool", "Cleared queue for stopped tool: %s", name);
    } else {
        // Keep queue for restart
        LOG_DEBUG("tool", "Preserved queue for tool: %s (%d events)", 
                 name, tool_queue_count(tool->inbox));
    }
    
    tool->status = TOOL_STOPPED;
    tool->process_handle = INVALID_HANDLE_VALUE;
    
    LOG_INFO("tool", "Tool %s stopped", name);
    
    return FW_OK;
}

int tool_restart(const char* name) {
    Tool* tool = tool_find(name);
    if (!tool) {
        return FW_ERROR_NOT_FOUND;
    }
    
    LOG_INFO("tool", "Restarting tool: %s", name);
    
    if (tool->status == TOOL_RUNNING) {
        int result = tool_stop(name);
        if (result != FW_OK) {
            return result;
        }
    }
    
    tool->restart_count++;
    
    return tool_start(name);
}

int tool_subscribe(const char* name, const char* event_type) {
    Tool* tool = tool_find(name);
    if (!tool) {
        return FW_ERROR_NOT_FOUND;
    }
    
    if (tool->subscription_count >= MAX_SUBSCRIPTIONS) {
        LOG_ERROR("tool", "Tool %s subscription limit reached", name);
        return FW_ERROR_GENERIC;
    }
    
    // Store subscription (it's an array of char arrays, not pointers)
    strncpy(tool->subscriptions[tool->subscription_count], event_type, MAX_EVENT_TYPE - 1);
    tool->subscriptions[tool->subscription_count][MAX_EVENT_TYPE - 1] = '\0';
    tool->subscription_count++;
    
    LOG_DEBUG("tool", "Tool %s subscribed to: %s", name, event_type);
    
    return FW_OK;
}

const char* tool_status_string(ToolStatus status) {
    switch (status) {
        case TOOL_STOPPED:  return "STOPPED";
        case TOOL_STARTING: return "STARTING";
        case TOOL_RUNNING:  return "RUNNING";
        case TOOL_STOPPING: return "STOPPING";
        case TOOL_CRASHED:  return "CRASHED";
        case TOOL_ERROR:    return "ERROR";
        default:            return "UNKNOWN";
    }
}

int tool_send_event(const char* name, const char* event_msg) {
    Tool* tool = tool_find(name);
    if (!tool) {
        return FW_ERROR_NOT_FOUND;
    }
    
    if (tool->status != TOOL_RUNNING) {
        return FW_ERROR_GENERIC;
    }
    
    // Write to tool's stdin
    DWORD bytes_written;
    DWORD bytes_to_write = (DWORD)strlen(event_msg);
    
    BOOL success = WriteFile(tool->stdin_handle, event_msg, bytes_to_write, &bytes_written, NULL);
    if (!success || bytes_written != bytes_to_write) {
        LOG_ERROR("tool", "Failed to send event to tool %s", name);
        return FW_ERROR_IO;
    }
    
    tool->events_sent++;
    
    return FW_OK;
}

// NEW FUNCTION: Non-blocking send
int tool_send_event_nonblocking(const char* name, const char* event_msg) {
    Tool* tool = tool_find(name);
    if (!tool || tool->status != TOOL_RUNNING) {
        return FW_ERROR_NOT_FOUND;
    }
    
    // Try to write without blocking
    DWORD bytes_written;
    DWORD bytes_to_write = (DWORD)strlen(event_msg);
    
    BOOL success = WriteFile(tool->stdin_handle, event_msg, bytes_to_write, &bytes_written, NULL);
    
    if (!success) {
        DWORD error = GetLastError();
        if (error == ERROR_NO_DATA || error == ERROR_PIPE_NOT_CONNECTED) {
            return FW_ERROR_QUEUE_FULL;  // Pipe full or not ready
        }
        return FW_ERROR_IO;
    }
    
    if (bytes_written != bytes_to_write) {
        return FW_ERROR_IO;  // Partial write
    }
    
    tool->events_sent++;
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
    
    if (!platform_is_process_running(tool->process_handle)) {
        return false;
    }
    
    return true;
}

void tool_update_heartbeat(const char* name) {
    Tool* tool = tool_find(name);
    if (tool && tool->status == TOOL_RUNNING) {
        tool->last_heartbeat = time(NULL);
    }
}

void tool_check_health(void) {
    for (int i = 0; i < registry.count; i++) {
        Tool* tool = registry.tools[i];
        if (!tool) {
            continue;
        }
        
        // Check if running tool has crashed
        if (tool->status == TOOL_RUNNING) {
            if (!platform_is_process_running(tool->process_handle)) {
                LOG_ERROR("tool", "Tool %s crashed", tool->name);
                tool->status = TOOL_CRASHED;
                
                // Close handles
                if (tool->stdin_handle != INVALID_HANDLE_VALUE) {
                    CloseHandle(tool->stdin_handle);
                    tool->stdin_handle = INVALID_HANDLE_VALUE;
                }
                if (tool->stdout_handle != INVALID_HANDLE_VALUE) {
                    CloseHandle(tool->stdout_handle);
                    tool->stdout_handle = INVALID_HANDLE_VALUE;
                }
                if (tool->stderr_handle != INVALID_HANDLE_VALUE) {
                    CloseHandle(tool->stderr_handle);
                    tool->stderr_handle = INVALID_HANDLE_VALUE;
                }
                
                // Restart if configured
                if (tool->restart_on_crash && tool->restart_count < tool->max_restarts) {
                    LOG_INFO("tool", "Restarting crashed tool %s (attempt %d/%d)", 
                             tool->name, tool->restart_count + 1, tool->max_restarts);
                    tool_restart(tool->name);
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

int tool_get_status(const char* name, char* buffer, size_t size) {
    Tool* tool = tool_find(name);
    if (!tool) {
        return FW_ERROR_NOT_FOUND;
    }
    
    snprintf(buffer, size, "%s", tool_status_string(tool->status));
    return FW_OK;
}
