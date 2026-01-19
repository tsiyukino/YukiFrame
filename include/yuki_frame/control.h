#ifndef YUKI_FRAME_CONTROL_H
#define YUKI_FRAME_CONTROL_H

#include "yuki_frame/framework.h"

int control_init(void);
void control_shutdown(void);
int control_process_command(const ControlRequest* request, ControlResponse* response);
int control_start_tool(const char* tool_name);
int control_stop_tool(const char* tool_name);
int control_restart_tool(const char* tool_name);
int control_list_tools(char* buffer, size_t buffer_size);
int control_get_status(const char* tool_name, char* buffer, size_t buffer_size);

#endif  // YUKI_FRAME_CONTROL_H
