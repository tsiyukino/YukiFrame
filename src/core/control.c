#include "yuki_frame/framework.h"
#include "yuki_frame/control_api.h"
#include "yuki_frame/logger.h"
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
            response->success = (control_start_tool(request->tool_name) == FW_OK);
            snprintf(response->message, sizeof(response->message), 
                     "Tool '%s' %s", request->tool_name,
                     response->success ? "started" : "failed to start");
            break;
            
        case CMD_STOP_TOOL:
            response->success = (control_stop_tool(request->tool_name) == FW_OK);
            snprintf(response->message, sizeof(response->message),
                     "Tool '%s' %s", request->tool_name,
                     response->success ? "stopped" : "failed to stop");
            break;
            
        case CMD_RESTART_TOOL:
            response->success = (control_restart_tool(request->tool_name) == FW_OK);
            snprintf(response->message, sizeof(response->message),
                     "Tool '%s' %s", request->tool_name,
                     response->success ? "restarted" : "failed to restart");
            break;
            
        case CMD_LIST_TOOLS:
        case CMD_GET_STATUS:
            // These are now handled by control_api.h functions
            snprintf(response->message, sizeof(response->message), 
                     "Use control_execute_command() from control_api.h");
            response->success = false;
            break;
            
        default:
            snprintf(response->message, sizeof(response->message), "Unknown command");
            break;
    }
    
    return FW_OK;
}
