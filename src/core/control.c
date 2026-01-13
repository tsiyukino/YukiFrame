#include "framework.h"
#include "logger.h"
#include "tool.h"
#include <string.h>
#include <stdio.h>

int control_init(void) {
    LOG_INFO("control", "Control system initialized");
    return FW_OK;
}

void control_shutdown(void) {
    LOG_INFO("control", "Control system shutdown");
}

int control_process_command(const ControlRequest* request, ControlResponse* response) {
    if (!request || !response) {
        return FW_ERROR_INVALID_ARG;
    }
    
    response->success = false;
    
    switch (request->command) {
        case CMD_START_TOOL:
            response->success = (tool_start(request->tool_name) == FW_OK);
            snprintf(response->message, sizeof(response->message), 
                     "Tool '%s' %s", request->tool_name,
                     response->success ? "started" : "failed to start");
            break;
            
        case CMD_STOP_TOOL:
            response->success = (tool_stop(request->tool_name) == FW_OK);
            snprintf(response->message, sizeof(response->message),
                     "Tool '%s' %s", request->tool_name,
                     response->success ? "stopped" : "failed to stop");
            break;
            
        case CMD_RESTART_TOOL:
            response->success = (tool_restart(request->tool_name) == FW_OK);
            snprintf(response->message, sizeof(response->message),
                     "Tool '%s' %s", request->tool_name,
                     response->success ? "restarted" : "failed to restart");
            break;
            
        case CMD_LIST_TOOLS:
            response->success = (control_list_tools(response->data, sizeof(response->data)) == FW_OK);
            snprintf(response->message, sizeof(response->message), "Tool list retrieved");
            break;
            
        case CMD_GET_STATUS:
            response->success = (control_get_status(request->tool_name, response->data, sizeof(response->data)) == FW_OK);
            snprintf(response->message, sizeof(response->message), "Status retrieved for '%s'", request->tool_name);
            break;
            
        default:
            snprintf(response->message, sizeof(response->message), "Unknown command");
            break;
    }
    
    return FW_OK;
}

int control_start_tool(const char* tool_name) {
    return tool_start(tool_name);
}

int control_stop_tool(const char* tool_name) {
    return tool_stop(tool_name);
}

int control_restart_tool(const char* tool_name) {
    return tool_restart(tool_name);
}

int control_list_tools(char* buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) {
        return FW_ERROR_INVALID_ARG;
    }
    
    buffer[0] = '\0';
    size_t offset = 0;
    
    Tool* tool = tool_get_first();
    while (tool && offset < buffer_size - 100) {
        offset += snprintf(buffer + offset, buffer_size - offset,
                          "%-20s %-10s PID: %d\n",
                          tool->name,
                          tool->status == TOOL_RUNNING ? "RUNNING" :
                          tool->status == TOOL_STOPPED ? "STOPPED" :
                          tool->status == TOOL_CRASHED ? "CRASHED" : "UNKNOWN",
                          (int)tool->pid);
        tool = tool_get_next();
    }
    
    return FW_OK;
}

int control_get_status(const char* tool_name, char* buffer, size_t buffer_size) {
    if (!tool_name || !buffer || buffer_size == 0) {
        return FW_ERROR_INVALID_ARG;
    }
    
    Tool* tool = tool_find(tool_name);
    if (!tool) {
        snprintf(buffer, buffer_size, "Tool '%s' not found", tool_name);
        return FW_ERROR_NOT_FOUND;
    }
    
    snprintf(buffer, buffer_size,
             "Tool: %s\n"
             "Status: %s\n"
             "PID: %d\n"
             "Events Sent: %lu\n"
             "Events Received: %lu\n"
             "Restart Count: %d\n",
             tool->name,
             tool->status == TOOL_RUNNING ? "RUNNING" :
             tool->status == TOOL_STOPPED ? "STOPPED" :
             tool->status == TOOL_CRASHED ? "CRASHED" : "UNKNOWN",
             (int)tool->pid,
             tool->events_sent,
             tool->events_received,
             tool->restart_count);
    
    return FW_OK;
}
