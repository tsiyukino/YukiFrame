#include "yuki_frame/framework.h"
#include "yuki_frame/tool.h"
#include "yuki_frame/logger.h"
#include <stdlib.h>
#include <string.h>

static ToolRegistry registry;
static int current_iterator = 0;

int tool_registry_init(void) {
    memset(&registry, 0, sizeof(registry));
    registry.count = 0;
    LOG_INFO("tool", "Tool registry initialized");
    return FW_OK;
}

void tool_registry_shutdown(void) {
    for (int i = 0; i < registry.count; i++) {
        if (registry.tools[i]) {
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
    
    registry.tools[registry.count++] = tool;
    
    LOG_INFO("tool", "Registered tool: %s", name);
    return FW_OK;
}

int tool_unregister(const char* name) {
    Tool* tool = tool_find(name);
    if (!tool) {
        return FW_ERROR_NOT_FOUND;
    }
    
    // Remove from registry
    for (int i = 0; i < registry.count; i++) {
        if (registry.tools[i] == tool) {
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
    
    // TODO: Actually spawn the process using platform_spawn_process
    tool->status = TOOL_RUNNING;
    tool->started_at = time(NULL);
    tool->pid = 0; // Placeholder
    
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
    
    // TODO: Actually kill the process
    tool->status = TOOL_STOPPED;
    tool->pid = 0;
    
    return FW_OK;
}

int tool_restart(const char* name) {
    int ret = tool_stop(name);
    if (ret != FW_OK) {
        return ret;
    }
    
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
    
    // TODO: Actually send event to tool's stdin
    tool->events_received++;
    
    return FW_OK;
}

bool tool_is_running(const char* name) {
    Tool* tool = tool_find(name);
    return (tool && tool->status == TOOL_RUNNING);
}

void tool_update_heartbeat(const char* name) {
    Tool* tool = tool_find(name);
    if (tool) {
        tool->last_heartbeat = time(NULL);
    }
}

void tool_check_health(void) {
    // TODO: Check if tools are still running
    // For now, do nothing
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
