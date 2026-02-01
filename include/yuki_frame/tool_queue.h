#ifndef YUKI_FRAME_TOOL_QUEUE_H
#define YUKI_FRAME_TOOL_QUEUE_H

#include "framework.h"

// Queue policy when queue is full
typedef enum {
    QUEUE_POLICY_DROP_OLDEST,   // Drop oldest event, add new one (default)
    QUEUE_POLICY_DROP_NEWEST,   // Reject new event, keep existing queue
    QUEUE_POLICY_BLOCK          // Block until space available (use carefully!)
} QueuePolicy;

// Per-tool event queue
typedef struct {
    char** messages;            // Array of event message strings
    int capacity;               // Maximum queue size
    int head;                   // Read position
    int tail;                   // Write position
    int count;                  // Current number of items
    QueuePolicy policy;         // What to do when full
    int dropped_count;          // Statistics: events dropped
    int delivered_count;        // Statistics: events delivered
} ToolQueue;

// Initialize tool queue
int tool_queue_init(ToolQueue** queue, int capacity, QueuePolicy policy);

// Shutdown and free tool queue
void tool_queue_shutdown(ToolQueue* queue);

// Add event to queue (returns FW_OK or error)
int tool_queue_add(ToolQueue* queue, const char* event_msg);

// Peek at next event without removing (returns NULL if empty)
const char* tool_queue_peek(ToolQueue* queue);

// Remove event from queue (call after successful delivery)
void tool_queue_remove(ToolQueue* queue);

// Get queue statistics
int tool_queue_count(ToolQueue* queue);
int tool_queue_capacity(ToolQueue* queue);
int tool_queue_dropped(ToolQueue* queue);
int tool_queue_delivered(ToolQueue* queue);
bool tool_queue_is_empty(ToolQueue* queue);
bool tool_queue_is_full(ToolQueue* queue);

// Clear all events in queue
void tool_queue_clear(ToolQueue* queue);

#endif // YUKI_FRAME_TOOL_QUEUE_H
