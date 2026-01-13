#include "framework.h"
#include "event.h"
#include "logger.h"
#include <stdlib.h>
#include <string.h>

static MessageBus bus;

int event_bus_init(void) {
    memset(&bus, 0, sizeof(bus));
    bus.head = 0;
    bus.tail = 0;
    bus.count = 0;
    LOG_INFO("event", "Event bus initialized");
    return FW_OK;
}

void event_bus_shutdown(void) {
    // Free any events in queue
    for (int i = 0; i < bus.count; i++) {
        int index = (bus.head + i) % MAX_EVENTS_QUEUE;
        if (bus.queue[index]) {
            free(bus.queue[index]);
            bus.queue[index] = NULL;
        }
    }
    bus.count = 0;
    LOG_INFO("event", "Event bus shutdown");
}

int event_publish(const char* type, const char* sender, const char* data) {
    if (!type || !sender) {
        return FW_ERROR_INVALID_ARG;
    }
    
    if (bus.count >= MAX_EVENTS_QUEUE) {
        LOG_ERROR("event", "Event queue full");
        return FW_ERROR_QUEUE_FULL;
    }
    
    Event* event = (Event*)malloc(sizeof(Event));
    if (!event) {
        return FW_ERROR_MEMORY;
    }
    
    memset(event, 0, sizeof(Event));
    strncpy(event->type, type, MAX_EVENT_TYPE - 1);
    strncpy(event->sender, sender, MAX_TOOL_NAME - 1);
    if (data) {
        strncpy(event->data, data, MAX_EVENT_DATA - 1);
    }
    event->timestamp = time(NULL);
    
    bus.queue[bus.tail] = event;
    bus.tail = (bus.tail + 1) % MAX_EVENTS_QUEUE;
    bus.count++;
    
    LOG_DEBUG("event", "Published event: %s from %s", type, sender);
    
    return FW_OK;
}

int event_parse(const char* line, Event* event) {
    if (!line || !event) {
        return FW_ERROR_INVALID_ARG;
    }
    
    memset(event, 0, sizeof(Event));
    
    // Parse format: TYPE|sender|data
    char buffer[MAX_EVENT_DATA + 256];
    strncpy(buffer, line, sizeof(buffer) - 1);
    
    char* type = strtok(buffer, "|");
    if (!type) return FW_ERROR_PARSE_FAILED;
    
    char* sender = strtok(NULL, "|");
    if (!sender) return FW_ERROR_PARSE_FAILED;
    
    char* data = strtok(NULL, "\n");
    
    strncpy(event->type, type, MAX_EVENT_TYPE - 1);
    strncpy(event->sender, sender, MAX_TOOL_NAME - 1);
    if (data) {
        strncpy(event->data, data, MAX_EVENT_DATA - 1);
    }
    event->timestamp = time(NULL);
    
    return FW_OK;
}

int event_format(const Event* event, char* buffer, size_t size) {
    if (!event || !buffer || size == 0) {
        return FW_ERROR_INVALID_ARG;
    }
    
    snprintf(buffer, size, "%s|%s|%s\n", 
             event->type, event->sender, event->data);
    
    return FW_OK;
}

void event_process_queue(void) {
    // TODO: Route events to subscribed tools
    // For now, just clear the queue
    while (bus.count > 0) {
        Event* event = bus.queue[bus.head];
        if (event) {
            LOG_TRACE("event", "Processing event: %s from %s", 
                     event->type, event->sender);
            free(event);
            bus.queue[bus.head] = NULL;
        }
        bus.head = (bus.head + 1) % MAX_EVENTS_QUEUE;
        bus.count--;
    }
}
