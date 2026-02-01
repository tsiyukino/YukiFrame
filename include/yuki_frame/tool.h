#ifndef YUKI_FRAME_TOOL_H
#define YUKI_FRAME_TOOL_H

#include "framework.h"
#include "tool_queue.h"

// Tool status
typedef enum {
    TOOL_STOPPED = 0,
    TOOL_STARTING = 1,
    TOOL_RUNNING = 2,
    TOOL_STOPPING = 3,
    TOOL_CRASHED = 4,
    TOOL_ERROR = 5
} ToolStatus;

// Restart policy
typedef enum {
    RESTART_NEVER = 0,
    RESTART_ALWAYS = 1,
    RESTART_ON_DEMAND = 2
} RestartPolicy;

// Tool structure (YOUR EXISTING STRUCTURE + NEW QUEUE FIELDS)
typedef struct Tool {
    char name[MAX_TOOL_NAME];
    char command[MAX_COMMAND_LENGTH];
    char description[256];  // Your existing field
    
    // Process info (YOUR EXISTING FIELDS)
    HANDLE process_handle;  // Your field name
    ProcessID pid;
    ToolStatus status;
    
    // Pipes (YOUR EXISTING FIELDS)
    int stdin_fd;
    int stdout_fd;
    int stderr_fd;
    HANDLE stdin_handle;
    HANDLE stdout_handle;
    HANDLE stderr_handle;
    
    // Configuration (YOUR EXISTING FIELDS)
    bool autostart;
    bool restart_on_crash;
    RestartPolicy restart_policy;
    int restart_max_delay_sec;
    int max_restarts;  // Your existing field
    
    // Subscriptions (YOUR EXISTING FIELDS)
    char subscriptions[MAX_SUBSCRIPTIONS][MAX_EVENT_TYPE];
    int subscription_count;
    
    // ============ NEW QUEUE FIELDS ============
    ToolQueue* inbox;          // Per-tool event queue
    int max_queue_size;        // Config: max queue size
    QueuePolicy queue_policy;  // Config: queue policy
    
    // On-demand state
    bool is_on_demand;         // restart_policy == RESTART_ON_DEMAND
    bool is_starting;          // Tool is starting but not ready yet
    // ============ END NEW FIELDS ============
    
    // Statistics (YOUR EXISTING FIELDS)
    int events_sent;
    int events_received;
    int restart_count;
    time_t start_time;
    time_t started_at;         // Your existing field
    time_t last_heartbeat;     // Your existing field
    int log_lines;             // Your existing field
    
    // Linked list (YOUR EXISTING FIELD)
    struct Tool* next;
} Tool;

// Tool registry structure (YOUR EXISTING STRUCTURE)
typedef struct {
    Tool* tools[MAX_TOOLS];
    int count;
} ToolRegistry;

// Tool registry functions
int tool_registry_init(void);
void tool_registry_shutdown(void);

// Tool management
int tool_register(const char* name, const char* command);
int tool_unregister(const char* name);
Tool* tool_find(const char* name);
int tool_start(const char* name);
int tool_stop(const char* name);
int tool_restart(const char* name);
int tool_subscribe(const char* name, const char* event_type);

// Tool iteration
Tool* tool_get_first(void);
Tool* tool_get_next(void);

// Tool status
const char* tool_status_string(ToolStatus status);
int tool_get_status(const char* name, char* buffer, size_t size);
bool tool_is_running(const char* name);

// Tool communication
int tool_send_event(const char* name, const char* event_msg);
int tool_send_event_nonblocking(const char* name, const char* event_msg);  // NEW!

// Tool health monitoring
void tool_check_health(void);
void tool_update_heartbeat(const char* name);

#endif // YUKI_FRAME_TOOL_H
