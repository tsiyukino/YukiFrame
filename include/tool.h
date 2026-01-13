#ifndef TOOL_H
#define TOOL_H

#include "framework.h"
#include <time.h>

// Tool status
typedef enum {
    TOOL_STOPPED = 0,
    TOOL_STARTING,
    TOOL_RUNNING,
    TOOL_STOPPING,
    TOOL_CRASHED,
    TOOL_ERROR
} ToolStatus;

// Tool structure
typedef struct {
    char name[MAX_TOOL_NAME];
    char command[MAX_COMMAND_LENGTH];
    char description[256];
    
    // Process info
    ProcessHandle process_handle;
    ProcessID pid;
    
    // Status
    ToolStatus status;
    bool autostart;
    bool restart_on_crash;
    int max_restarts;
    int restart_count;
    time_t started_at;
    time_t last_heartbeat;
    
    // I/O handles
    int stdin_fd;
    int stdout_fd;
    int stderr_fd;
    
    // Event subscriptions
    char* subscriptions[MAX_SUBSCRIPTIONS];
    int subscription_count;
    
    // Statistics
    unsigned long events_sent;
    unsigned long events_received;
    unsigned long log_lines;
    
} Tool;

// Tool registry
typedef struct {
    Tool* tools[MAX_TOOLS];
    int count;
} ToolRegistry;

// Tool management functions
int tool_registry_init(void);
void tool_registry_shutdown(void);
int tool_register(const char* name, const char* command);
int tool_unregister(const char* name);
Tool* tool_find(const char* name);
int tool_start(const char* name);
int tool_stop(const char* name);
int tool_restart(const char* name);
int tool_subscribe(const char* name, const char* event_type);
int tool_send_event(const char* name, const char* event);
bool tool_is_running(const char* name);
void tool_update_heartbeat(const char* name);
void tool_check_health(void);

// Tool iteration
Tool* tool_get_first(void);
Tool* tool_get_next(void);

#endif // TOOL_H
