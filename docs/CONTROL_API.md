# Control API Reference

## Overview

The Control API is integrated into the Yuki-Frame core, allowing **any tool** to manage other tools programmatically. This is a key feature of v2.0 - control is no longer external, it's built-in.

## Architecture

```
┌────────────────────────────────────────┐
│  Yuki-Frame Core Process               │
│                                        │
│  ┌──────────────────────────────────┐ │
│  │  Control API (Integrated!)       │ │
│  │  • control_start_tool()          │ │
│  │  • control_stop_tool()           │ │
│  │  • control_list_tools()          │ │
│  │  • control_get_tool_status()     │ │
│  │  • control_shutdown_framework()  │ │
│  └──────────────────────────────────┘ │
│            ▲                           │
│            │ Any tool can call!        │
│            │                           │
│  ┌─────────┴──────────────────────┐   │
│  │  Tool Registry                 │   │
│  │  • monitor      RUNNING        │   │
│  │  • alerter      RUNNING        │   │
│  │  • backup       STOPPED        │   │
│  └────────────────────────────────┘   │
└────────────────────────────────────────┘
```

## API Functions

### Tool Management

#### `control_start_tool()`

Start a tool by name.

**Signature:**
```c
int control_start_tool(const char* tool_name);
```

**Parameters:**
- `tool_name`: Name of the tool to start (must be registered in config)

**Returns:**
- `FW_OK` (0) on success
- `FW_ERROR_NOT_FOUND` (-1) if tool not found
- `FW_ERROR_GENERIC` (-2) if start failed

**Example:**
```c
#include "yuki_frame/control_api.h"

int result = control_start_tool("backup");
if (result == FW_OK) {
    printf("Backup tool started successfully\n");
} else {
    fprintf(stderr, "Failed to start backup tool\n");
}
```

#### `control_stop_tool()`

Stop a running tool.

**Signature:**
```c
int control_stop_tool(const char* tool_name);
```

**Parameters:**
- `tool_name`: Name of the tool to stop

**Returns:**
- `FW_OK` (0) on success
- `FW_ERROR_NOT_FOUND` (-1) if tool not found
- `FW_ERROR_GENERIC` (-2) if stop failed

**Example:**
```c
int result = control_stop_tool("backup");
if (result == FW_OK) {
    printf("Backup tool stopped\n");
}
```

#### `control_restart_tool()`

Restart a tool (stop then start).

**Signature:**
```c
int control_restart_tool(const char* tool_name);
```

**Parameters:**
- `tool_name`: Name of the tool to restart

**Returns:**
- `FW_OK` (0) on success
- `FW_ERROR_NOT_FOUND` (-1) if tool not found
- `FW_ERROR_GENERIC` (-2) if restart failed

**Example:**
```c
int result = control_restart_tool("monitor");
if (result == FW_OK) {
    printf("Monitor restarted\n");
}
```

### Tool Information

#### `control_get_tool_status()`

Get detailed status information for a specific tool.

**Signature:**
```c
int control_get_tool_status(const char* tool_name, ControlToolInfo* info);
```

**Parameters:**
- `tool_name`: Name of the tool
- `info`: Output buffer for tool information (caller-allocated)

**Returns:**
- `FW_OK` (0) on success
- `FW_ERROR_NOT_FOUND` (-1) if tool not found

**ControlToolInfo Structure:**
```c
typedef struct {
    char name[256];              // Tool name
    char command[1024];          // Command line
    char description[256];       // Tool description
    int status;                  // TOOL_STOPPED, TOOL_RUNNING, etc.
    uint32_t pid;                // Process ID (0 if not running)
    bool autostart;              // Auto-start on framework startup
    bool restart_on_crash;       // Restart if crashed
    int max_restarts;            // Maximum restart attempts
    int restart_count;           // Current restart count
    uint64_t events_sent;        // Number of events sent
    uint64_t events_received;    // Number of events received
    int subscription_count;      // Number of subscriptions
} ControlToolInfo;
```

**Example:**
```c
ControlToolInfo info;
int result = control_get_tool_status("monitor", &info);

if (result == FW_OK) {
    printf("Tool: %s\n", info.name);
    printf("Status: %s\n", info.status == TOOL_RUNNING ? "RUNNING" : "STOPPED");
    printf("PID: %u\n", info.pid);
    printf("Events sent: %lu\n", info.events_sent);
}
```

#### `control_list_tools()`

List all registered tools using a callback function.

**Signature:**
```c
typedef bool (*control_list_callback_t)(const ControlToolInfo* info, void* user_data);

int control_list_tools(control_list_callback_t callback, void* user_data);
```

**Parameters:**
- `callback`: Function to call for each tool
- `user_data`: User data to pass to callback

**Returns:**
- Number of tools processed, or negative error code

**Callback Return:**
- `true` to continue iteration
- `false` to stop iteration early

**Example:**
```c
bool print_tool(const ControlToolInfo* info, void* user_data) {
    printf("%-20s %-10s PID: %u\n", 
           info->name,
           info->status == TOOL_RUNNING ? "RUNNING" : "STOPPED",
           info->pid);
    return true;  // Continue
}

int count = control_list_tools(print_tool, NULL);
printf("Total tools: %d\n", count);
```

#### `control_get_tool_count()`

Get the total number of registered tools.

**Signature:**
```c
int control_get_tool_count(void);
```

**Returns:**
- Number of tools, or 0 if none registered

**Example:**
```c
int count = control_get_tool_count();
printf("Total registered tools: %d\n", count);
```

#### `control_tool_exists()`

Check if a tool exists in the registry.

**Signature:**
```c
bool control_tool_exists(const char* tool_name);
```

**Parameters:**
- `tool_name`: Name of the tool

**Returns:**
- `true` if tool exists
- `false` otherwise

**Example:**
```c
if (control_tool_exists("backup")) {
    printf("Backup tool is registered\n");
}
```

### Framework Control

#### `control_shutdown_framework()`

Request graceful framework shutdown.

**Signature:**
```c
int control_shutdown_framework(void);
```

**Returns:**
- `FW_OK` (0) on success

**Example:**
```c
control_shutdown_framework();
// Framework will stop all tools and exit cleanly
```

#### `control_get_uptime()`

Get framework uptime in seconds.

**Signature:**
```c
uint64_t control_get_uptime(void);
```

**Returns:**
- Uptime in seconds since framework started

**Example:**
```c
uint64_t uptime = control_get_uptime();
printf("Framework uptime: %lu seconds\n", uptime);
```

#### `control_get_version()`

Get framework version string.

**Signature:**
```c
const char* control_get_version(void);
```

**Returns:**
- Version string (e.g., "2.0.0")

**Example:**
```c
printf("Yuki-Frame version: %s\n", control_get_version());
```

### Command Execution

#### `control_execute_command()`

Process a control command string (convenience function).

**Signature:**
```c
int control_execute_command(const char* command, char* response, size_t response_size);
```

**Parameters:**
- `command`: Command string (e.g., "start my_tool", "list", "status my_tool")
- `response`: Output buffer for response message (caller-allocated)
- `response_size`: Size of response buffer

**Returns:**
- `FW_OK` (0) on success
- Negative error code on failure

**Supported Commands:**
- `list` - List all tools
- `start <tool>` - Start a tool
- `stop <tool>` - Stop a tool
- `restart <tool>` - Restart a tool
- `status <tool>` - Show tool status
- `uptime` - Show framework uptime
- `version` - Show framework version
- `shutdown` - Shutdown framework
- `help` - Show help message

**Example:**
```c
char response[4096];
int result = control_execute_command("list", response, sizeof(response));

if (result == FW_OK) {
    printf("%s", response);
}
```

## Usage Examples

### Example 1: Watchdog Tool

A tool that monitors other tools and restarts them if they crash:

```c
#include "yuki_frame/control_api.h"
#include <stdio.h>
#include <unistd.h>

bool check_tool(const ControlToolInfo* info, void* user_data) {
    if (info->status == TOOL_CRASHED && info->restart_on_crash) {
        printf("[WATCHDOG] Tool %s crashed, restarting...\n", info->name);
        control_restart_tool(info->name);
    }
    return true;
}

int main(void) {
    printf("[WATCHDOG] Started\n");
    
    while (1) {
        // Check all tools every 10 seconds
        control_list_tools(check_tool, NULL);
        sleep(10);
    }
    
    return 0;
}
```

### Example 2: Scheduler Tool

A tool that starts/stops other tools on schedule:

```c
#include "yuki_frame/control_api.h"
#include <time.h>

void check_schedule(void) {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    
    // Start backup at 2 AM
    if (tm_info->tm_hour == 2 && tm_info->tm_min == 0) {
        if (!control_tool_exists("backup") || 
            !is_tool_running("backup")) {
            control_start_tool("backup");
            printf("[SCHEDULER] Started backup\n");
        }
    }
    
    // Stop backup at 3 AM
    if (tm_info->tm_hour == 3 && tm_info->tm_min == 0) {
        control_stop_tool("backup");
        printf("[SCHEDULER] Stopped backup\n");
    }
}
```

### Example 3: Admin Dashboard Tool

A tool that provides a web-based admin interface:

```c
#include "yuki_frame/control_api.h"

void handle_api_request(const char* endpoint, char* response, size_t size) {
    if (strcmp(endpoint, "/tools") == 0) {
        // List all tools as JSON
        // ... build JSON from control_list_tools()
    }
    else if (strncmp(endpoint, "/tools/", 7) == 0) {
        const char* tool_name = endpoint + 7;
        char* action = strchr(tool_name, '/');
        
        if (action) {
            *action = '\0';
            action++;
            
            if (strcmp(action, "start") == 0) {
                control_start_tool(tool_name);
            }
            else if (strcmp(action, "stop") == 0) {
                control_stop_tool(tool_name);
            }
        }
    }
}
```

## Integration Guide

### In Your Tool

1. **Include the header:**
   ```c
   #include "yuki_frame/control_api.h"
   ```

2. **Call API functions:**
   ```c
   // Start another tool
   control_start_tool("my_other_tool");
   
   // Get tool status
   ControlToolInfo info;
   if (control_get_tool_status("monitor", &info) == FW_OK) {
       printf("Monitor PID: %u\n", info.pid);
   }
   
   // List all tools
   control_list_tools(my_callback, NULL);
   ```

3. **Build and run:**
   - The Control API is automatically available to all tools
   - No special linking needed
   - Just include the header and call the functions

### Error Handling

Always check return values:

```c
int result = control_start_tool("backup");

switch (result) {
    case FW_OK:
        printf("Success!\n");
        break;
    case FW_ERROR_NOT_FOUND:
        fprintf(stderr, "Tool not found\n");
        break;
    case FW_ERROR_GENERIC:
        fprintf(stderr, "Failed to start tool\n");
        break;
    default:
        fprintf(stderr, "Unknown error: %d\n", result);
}
```

## Best Practices

1. **Check tool existence before operating:**
   ```c
   if (control_tool_exists("backup")) {
       control_start_tool("backup");
   }
   ```

2. **Handle errors gracefully:**
   ```c
   if (control_start_tool("tool") != FW_OK) {
       // Fallback or retry
   }
   ```

3. **Avoid tight restart loops:**
   ```c
   // BAD: Can cause rapid restart loop
   if (status == CRASHED) {
       control_restart_tool(name);
   }
   
   // GOOD: Check restart count
   ControlToolInfo info;
   control_get_tool_status(name, &info);
   if (info.restart_count < info.max_restarts) {
       control_restart_tool(name);
   }
   ```

4. **Use callbacks for iteration:**
   ```c
   // Efficient iteration through all tools
   control_list_tools(my_callback, &my_data);
   ```

## Thread Safety

The Control API is **not thread-safe** by default. If you're calling it from multiple threads, you need to add your own synchronization.

For the interactive console (which runs in a separate thread), thread safety is handled internally by the framework.

## Performance Considerations

- Control API calls are fast (microseconds)
- `control_list_tools()` iterates through registry (O(n))
- `control_start_tool()` spawns process (milliseconds)
- No significant performance overhead

## Migration from v1.0

In v1.0, you had to use events to control tools:

```python
# v1.0 - Send event to control module
print("START_TOOL|control|my_tool")
```

In v2.0, use the Control API directly:

```c
// v2.0 - Direct API call
control_start_tool("my_tool");
```

Much simpler and more reliable!

## See Also

- `docs/DEVELOPMENT.md` - Development guide
- `docs/TOOL_DEVELOPMENT.md` - Writing tools
- `README.md` - Quick start
- `include/yuki_frame/control_api.h` - Full API header
