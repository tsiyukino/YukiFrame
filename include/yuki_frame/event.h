#ifndef YUKI_FRAME_EVENT_H
#define YUKI_FRAME_EVENT_H

#include "yuki_frame/framework.h"

// Event structure
typedef struct {
    char type[MAX_EVENT_TYPE];
    char sender[MAX_TOOL_NAME];
    char data[MAX_EVENT_DATA];
    time_t timestamp;
} Event;

// Message bus
typedef struct {
    Event* queue[MAX_EVENTS_QUEUE];
    int head;
    int tail;
    int count;
} MessageBus;

// Event functions
int event_bus_init(void);
void event_bus_shutdown(void);
int event_publish(const char* type, const char* sender, const char* data);
int event_parse(const char* line, Event* event);
int event_format(const Event* event, char* buffer, size_t size);
void event_process_queue(void);

#endif  // YUKI_FRAME_EVENT_H
