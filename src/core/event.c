#include "yuki_frame/framework.h"
#include "yuki_frame/event.h"
#include "yuki_frame/tool.h"
#include "yuki_frame/tool_queue.h"
#include "yuki_frame/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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
    // Route events to subscribed tools
    while (bus.count > 0) {
        Event* event = bus.queue[bus.head];
        if (event) {
            LOG_DEBUG("event", "Processing event: %s from %s", 
                     event->type, event->sender);
            
            // Deliver event to all subscribed tools
            Tool* tool = tool_get_first();
            int delivery_count = 0;
            
            while (tool != NULL) {
                // Check if tool is subscribed to this event type
                bool is_subscribed = false;
                
                // Check for wildcard subscription (*)
                for (int i = 0; i < tool->subscription_count; i++) {
                    // Trim leading/trailing quotes and whitespace from subscription
                    char sub[256];
                    strncpy(sub, tool->subscriptions[i], sizeof(sub) - 1);
                    sub[sizeof(sub) - 1] = '\0';
                    
                    // Remove leading quote/whitespace
                    char* s = sub;
                    while (*s == '\'' || *s == '"' || *s == ' ') s++;
                    
                    // Remove trailing quote/whitespace
                    char* end = s + strlen(s) - 1;
                    while (end > s && (*end == '\'' || *end == '"' || *end == ' ' || *end == '\n' || *end == '\r')) {
                        *end = '\0';
                        end--;
                    }
                    
                    if (strcmp(s, "*") == 0) {
                        is_subscribed = true;
                        break;
                    }
                    if (strcmp(s, event->type) == 0) {
                        is_subscribed = true;
                        break;
                    }
                }
                
                if (is_subscribed) {
                    // Format event message: TYPE|sender|data
                    char event_msg[8192];
                    snprintf(event_msg, sizeof(event_msg), "%s|%s|%s\n",
                             event->type, event->sender, event->data);
                    
                    // Add event to tool's inbox queue
                    int result = tool_queue_add(tool->inbox, event_msg);
                    
                    if (result == FW_OK) {
                        delivery_count++;
                        LOG_DEBUG("event", "Queued %s for tool: %s (queue: %d/%d)", 
                                 event->type, tool->name,
                                 tool_queue_count(tool->inbox),
                                 tool_queue_capacity(tool->inbox));
                        
                        // On-demand tool support
                        if (tool->is_on_demand && tool->status == TOOL_STOPPED && !tool->is_starting) {
                            LOG_INFO("event", "Starting on-demand tool: %s (triggered by %s)", 
                                    tool->name, event->type);
                            tool_start(tool->name);
                            tool->is_starting = true;
                        }
                    } else {
                        LOG_ERROR("event", "Failed to queue event for %s: %d", tool->name, result);
                    }
                }
                
                tool = tool_get_next();
            }
            
            if (delivery_count > 0) {
                LOG_DEBUG("event", "Event %s queued for %d tools", event->type, delivery_count);
            }
            
            free(event);
            bus.queue[bus.head] = NULL;
        }
        bus.head = (bus.head + 1) % MAX_EVENTS_QUEUE;
        bus.count--;
    }
}
