#ifndef YUKI_FRAME_FRAMEWORK_H
#define YUKI_FRAME_FRAMEWORK_H

#include <stddef.h>
#include <stdbool.h>
#include <time.h>

// Version information (Semantic Versioning)
#define YUKI_FRAME_VERSION_MAJOR 2
#define YUKI_FRAME_VERSION_MINOR 0
#define YUKI_FRAME_VERSION_PATCH 0
#define YUKI_FRAME_VERSION_STRING "2.0.0"
#define YUKI_FRAME_NAME "Yuki-Frame"

// Platform detection - don't redefine if already defined by CMake
#ifndef PLATFORM_WINDOWS
#ifndef PLATFORM_LINUX
#ifdef _WIN32
    #define PLATFORM_WINDOWS
#else
    #define PLATFORM_LINUX
#endif
#endif
#endif

// Platform-specific includes and types
#ifdef PLATFORM_WINDOWS
    #include <windows.h>
    typedef HANDLE ProcessHandle;
    typedef DWORD ProcessID;
    #define SIGHUP -1  // Not used on Windows, define as dummy
#else
    #include <sys/types.h>
    #include <signal.h>
    typedef pid_t ProcessHandle;
    typedef pid_t ProcessID;
#endif

// Error codes
typedef enum {
    FW_OK = 0,
    FW_ERROR_GENERIC = -1,
    FW_ERROR_INVALID_ARG = -2,
    FW_ERROR_NOT_FOUND = -3,
    FW_ERROR_ALREADY_EXISTS = -4,
    FW_ERROR_PROCESS_FAILED = -5,
    FW_ERROR_PIPE_FAILED = -6,
    FW_ERROR_PARSE_FAILED = -7,
    FW_ERROR_QUEUE_FULL = -8,
    FW_ERROR_TIMEOUT = -9,
    FW_ERROR_MEMORY = -10,
    FW_ERROR_IO = -11
} FrameworkError;

// Log levels
typedef enum {
    LOG_TRACE = 0,
    LOG_DEBUG = 1,
    LOG_INFO = 2,
    LOG_WARN = 3,
    LOG_ERROR = 4,
    LOG_FATAL = 5
} LogLevel;

// Constants
#define MAX_TOOL_NAME 64
#define MAX_COMMAND_LENGTH 512
#define MAX_EVENT_TYPE 64
#define MAX_EVENT_DATA 4096
#define MAX_LOG_MESSAGE 1024
#define MAX_TOOLS 100
#define MAX_EVENTS_QUEUE 1000
#define MAX_SUBSCRIPTIONS 50

// Framework configuration
typedef struct {
    char log_file[256];
    LogLevel log_level;
    char pid_file[256];
    int max_tools;
    int message_queue_size;
    bool enable_debug;
    bool enable_remote_control;
    int control_port;
} FrameworkConfig;

// Global framework state
extern FrameworkConfig g_config;
extern bool g_running;

// Core framework functions
int framework_init(const char* config_file);
void framework_run(void);
void framework_shutdown(void);
const char* framework_version(void);

// Control commands
typedef enum {
    CMD_START_TOOL,
    CMD_STOP_TOOL,
    CMD_RESTART_TOOL,
    CMD_LIST_TOOLS,
    CMD_GET_STATUS,
    CMD_RELOAD_CONFIG,
    CMD_SHUTDOWN
} ControlCommand;

typedef struct {
    ControlCommand command;
    char tool_name[MAX_TOOL_NAME];
    char data[MAX_EVENT_DATA];
} ControlRequest;

typedef struct {
    bool success;
    char message[MAX_LOG_MESSAGE];
    char data[MAX_EVENT_DATA];
} ControlResponse;

// Legacy Control API (for internal use only - use control_api.h for tools)
int control_init(void);
void control_shutdown(void);
int control_process_command(const ControlRequest* request, ControlResponse* response);

// Debug API (integrated into core)
typedef enum {
    DEBUG_TOOL_START,
    DEBUG_TOOL_STOP,
    DEBUG_TOOL_CRASH,
    DEBUG_EVENT_PUBLISH,
    DEBUG_EVENT_RECEIVE,
    DEBUG_CONFIG_RELOAD,
    DEBUG_ERROR
} DebugEventType;

typedef struct {
    DebugEventType type;
    time_t timestamp;
    char tool_name[MAX_TOOL_NAME];
    char message[MAX_LOG_MESSAGE];
    char details[MAX_EVENT_DATA];
} DebugEvent;

void debug_init(void);
void debug_shutdown(void);
void debug_log(DebugEventType type, const char* tool_name, const char* format, ...);
void debug_dump_state(void);
int debug_get_events(DebugEvent* events, int max_events);

#endif  // YUKI_FRAME_FRAMEWORK_H
