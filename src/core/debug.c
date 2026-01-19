#include "yuki_frame/framework.h"
#include "yuki_frame/logger.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define MAX_DEBUG_EVENTS 1000

static DebugEvent debug_events[MAX_DEBUG_EVENTS];
static int debug_event_count = 0;
static int debug_event_index = 0;

void debug_init(void) {
    memset(debug_events, 0, sizeof(debug_events));
    debug_event_count = 0;
    debug_event_index = 0;
    LOG_INFO("debug", "Debug system initialized");
}

void debug_shutdown(void) {
    LOG_INFO("debug", "Debug system shutdown - %d events captured", debug_event_count);
}

void debug_log(DebugEventType type, const char* tool_name, const char* format, ...) {
    DebugEvent* event = &debug_events[debug_event_index];
    
    event->type = type;
    event->timestamp = time(NULL);
    
    if (tool_name) {
        strncpy(event->tool_name, tool_name, MAX_TOOL_NAME - 1);
    } else {
        event->tool_name[0] = '\0';
    }
    
    va_list args;
    va_start(args, format);
    vsnprintf(event->message, MAX_LOG_MESSAGE, format, args);
    va_end(args);
    
    // Log to framework log as well
    const char* type_str = "";
    switch (type) {
        case DEBUG_TOOL_START: type_str = "TOOL_START"; break;
        case DEBUG_TOOL_STOP: type_str = "TOOL_STOP"; break;
        case DEBUG_TOOL_CRASH: type_str = "TOOL_CRASH"; break;
        case DEBUG_EVENT_PUBLISH: type_str = "EVENT_PUBLISH"; break;
        case DEBUG_EVENT_RECEIVE: type_str = "EVENT_RECEIVE"; break;
        case DEBUG_CONFIG_RELOAD: type_str = "CONFIG_RELOAD"; break;
        case DEBUG_ERROR: type_str = "ERROR"; break;
    }
    
    LOG_DEBUG("debug", "[%s] %s: %s", type_str, tool_name ? tool_name : "system", event->message);
    
    // Circular buffer
    debug_event_index = (debug_event_index + 1) % MAX_DEBUG_EVENTS;
    if (debug_event_count < MAX_DEBUG_EVENTS) {
        debug_event_count++;
    }
}

void debug_dump_state(void) {
    LOG_INFO("debug", "=== Debug State Dump ===");
    LOG_INFO("debug", "Total events captured: %d", debug_event_count);
    
    // Dump last 100 events
    int start = (debug_event_count < MAX_DEBUG_EVENTS) ? 0 : debug_event_index;
    int count = (debug_event_count < 100) ? debug_event_count : 100;
    
    for (int i = 0; i < count; i++) {
        int index = (start + i) % MAX_DEBUG_EVENTS;
        DebugEvent* event = &debug_events[index];
        
        char time_str[32];
        strftime(time_str, sizeof(time_str), "%H:%M:%S", localtime(&event->timestamp));
        
        LOG_INFO("debug", "[%s] %s - %s", time_str, event->tool_name, event->message);
    }
    
    LOG_INFO("debug", "=== End Debug Dump ===");
}

int debug_get_events(DebugEvent* events, int max_events) {
    if (!events || max_events <= 0) {
        return 0;
    }
    
    int count = (debug_event_count < max_events) ? debug_event_count : max_events;
    int start = (debug_event_count < MAX_DEBUG_EVENTS) ? 0 : debug_event_index;
    
    for (int i = 0; i < count; i++) {
        int index = (start + i) % MAX_DEBUG_EVENTS;
        events[i] = debug_events[index];
    }
    
    return count;
}
