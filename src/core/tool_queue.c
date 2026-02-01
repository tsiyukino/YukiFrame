#include "yuki_frame/tool_queue.h"
#include "yuki_frame/logger.h"
#include <stdlib.h>
#include <string.h>

int tool_queue_init(ToolQueue** queue, int capacity, QueuePolicy policy) {
    if (!queue || capacity <= 0) {
        return FW_ERROR_INVALID_ARG;
    }
    
    *queue = (ToolQueue*)malloc(sizeof(ToolQueue));
    if (!*queue) {
        return FW_ERROR_MEMORY;
    }
    
    (*queue)->messages = (char**)calloc(capacity, sizeof(char*));
    if (!(*queue)->messages) {
        free(*queue);
        *queue = NULL;
        return FW_ERROR_MEMORY;
    }
    
    (*queue)->capacity = capacity;
    (*queue)->head = 0;
    (*queue)->tail = 0;
    (*queue)->count = 0;
    (*queue)->policy = policy;
    (*queue)->dropped_count = 0;
    (*queue)->delivered_count = 0;
    
    return FW_OK;
}

void tool_queue_shutdown(ToolQueue* queue) {
    if (!queue) {
        return;
    }
    
    // Free all queued messages
    for (int i = 0; i < queue->capacity; i++) {
        if (queue->messages[i]) {
            free(queue->messages[i]);
            queue->messages[i] = NULL;
        }
    }
    
    free(queue->messages);
    free(queue);
}

int tool_queue_add(ToolQueue* queue, const char* event_msg) {
    if (!queue || !event_msg) {
        return FW_ERROR_INVALID_ARG;
    }
    
    // Queue full - apply policy
    if (queue->count >= queue->capacity) {
        switch (queue->policy) {
            case QUEUE_POLICY_DROP_OLDEST:
                // Remove oldest, make space for new
                if (queue->messages[queue->head]) {
                    free(queue->messages[queue->head]);
                    queue->messages[queue->head] = NULL;
                }
                queue->head = (queue->head + 1) % queue->capacity;
                queue->count--;
                queue->dropped_count++;
                LOG_WARN("tool_queue", "Queue full, dropped oldest event");
                break;
                
            case QUEUE_POLICY_DROP_NEWEST:
                // Reject new event
                queue->dropped_count++;
                LOG_WARN("tool_queue", "Queue full, dropped newest event");
                return FW_ERROR_QUEUE_FULL;
                
            case QUEUE_POLICY_BLOCK:
                // In this implementation, we return error
                // Caller should retry later
                LOG_WARN("tool_queue", "Queue full, blocking (retry needed)");
                return FW_ERROR_QUEUE_FULL;
        }
    }
    
    // Add event to queue
    queue->messages[queue->tail] = strdup(event_msg);
    if (!queue->messages[queue->tail]) {
        return FW_ERROR_MEMORY;
    }
    
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->count++;
    
    return FW_OK;
}

const char* tool_queue_peek(ToolQueue* queue) {
    if (!queue || queue->count == 0) {
        return NULL;
    }
    
    return queue->messages[queue->head];
}

void tool_queue_remove(ToolQueue* queue) {
    if (!queue || queue->count == 0) {
        return;
    }
    
    // Free the message
    if (queue->messages[queue->head]) {
        free(queue->messages[queue->head]);
        queue->messages[queue->head] = NULL;
    }
    
    queue->head = (queue->head + 1) % queue->capacity;
    queue->count--;
    queue->delivered_count++;
}

int tool_queue_count(ToolQueue* queue) {
    return queue ? queue->count : 0;
}

int tool_queue_capacity(ToolQueue* queue) {
    return queue ? queue->capacity : 0;
}

int tool_queue_dropped(ToolQueue* queue) {
    return queue ? queue->dropped_count : 0;
}

int tool_queue_delivered(ToolQueue* queue) {
    return queue ? queue->delivered_count : 0;
}

bool tool_queue_is_empty(ToolQueue* queue) {
    return queue ? (queue->count == 0) : true;
}

bool tool_queue_is_full(ToolQueue* queue) {
    return queue ? (queue->count >= queue->capacity) : false;
}

void tool_queue_clear(ToolQueue* queue) {
    if (!queue) {
        return;
    }
    
    while (queue->count > 0) {
        if (queue->messages[queue->head]) {
            free(queue->messages[queue->head]);
            queue->messages[queue->head] = NULL;
        }
        queue->head = (queue->head + 1) % queue->capacity;
        queue->count--;
    }
    
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
}
