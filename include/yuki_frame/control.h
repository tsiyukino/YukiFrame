#ifndef YUKI_FRAME_CONTROL_H
#define YUKI_FRAME_CONTROL_H

#include "yuki_frame/framework.h"

// Legacy control functions (internal use)
int control_init(void);
void control_shutdown(void);
int control_process_command(const ControlRequest* request, ControlResponse* response);

#endif  // YUKI_FRAME_CONTROL_H
