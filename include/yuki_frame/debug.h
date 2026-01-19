#ifndef YUKI_FRAME_DEBUG_H
#define YUKI_FRAME_DEBUG_H

#include "yuki_frame/framework.h"

void debug_init(void);
void debug_shutdown(void);
void debug_log(DebugEventType type, const char* tool_name, const char* format, ...);
void debug_dump_state(void);
int debug_get_events(DebugEvent* events, int max_events);

#endif  // YUKI_FRAME_DEBUG_H
