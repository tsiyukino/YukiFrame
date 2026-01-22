#include "yuki_frame/framework.h"
#include "yuki_frame/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>

#ifdef PLATFORM_WINDOWS
    #include <direct.h>
    #define mkdir(path, mode) _mkdir(path)
#else
    #include <sys/types.h>
#endif

static FILE* log_file = NULL;
static LogLevel current_level = LOG_INFO;

const char* log_level_string(LogLevel level) {
    switch (level) {
        case LOG_TRACE: return "TRACE";
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO:  return "INFO";
        case LOG_WARN:  return "WARN";
        case LOG_ERROR: return "ERROR";
        case LOG_FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

// Create directory if it doesn't exist
static int ensure_directory(const char* path) {
    char tmp[256];
    char* p = NULL;
    size_t len;
    
    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    
    if (tmp[len - 1] == '/' || tmp[len - 1] == '\\')
        tmp[len - 1] = 0;
    
    for (p = tmp + 1; *p; p++) {
        if (*p == '/' || *p == '\\') {
            *p = 0;
            mkdir(tmp, 0755);
            *p = '/';
        }
    }
    mkdir(tmp, 0755);
    
    return 0;
}

// Extract directory from file path
static void extract_directory(const char* filepath, char* directory, size_t dir_size) {
    strncpy(directory, filepath, dir_size - 1);
    directory[dir_size - 1] = '\0';
    
    // Find last slash
    char* last_slash = strrchr(directory, '/');
    char* last_backslash = strrchr(directory, '\\');
    
    char* separator = (last_slash > last_backslash) ? last_slash : last_backslash;
    
    if (separator) {
        *separator = '\0';
    } else {
        directory[0] = '\0';  // No directory, current dir
    }
}

int logger_init(const char* log_filename, LogLevel level) {
    if (!log_filename) {
        return FW_ERROR_INVALID_ARG;
    }
    
    // Extract and create directory
    char directory[256];
    extract_directory(log_filename, directory, sizeof(directory));
    
    if (directory[0] != '\0') {
        ensure_directory(directory);
    }
    
    // FIXED: Open log file in WRITE mode (truncate) instead of APPEND mode
    // This clears the log on each startup
    log_file = fopen(log_filename, "w");
    if (!log_file) {
        // Try current directory as fallback
        fprintf(stderr, "Failed to open log file: %s\n", log_filename);
        fprintf(stderr, "Trying current directory: yuki-frame.log\n");
        
        log_file = fopen("yuki-frame.log", "w");
        if (!log_file) {
            fprintf(stderr, "Failed to open fallback log file\n");
            return FW_ERROR_IO;
        }
    }
    
    current_level = level;
    
    // Write startup message
    time_t now = time(NULL);
    fprintf(log_file, "=== Yuki-Frame v%s started at %s", YUKI_FRAME_VERSION_STRING, ctime(&now));
    fflush(log_file);
    
    return FW_OK;
}

void logger_shutdown(void) {
    if (log_file) {
        time_t now = time(NULL);
        fprintf(log_file, "=== Yuki-Frame shutdown at %s", ctime(&now));
        fclose(log_file);
        log_file = NULL;
    }
}

void logger_log(LogLevel level, const char* component, const char* format, ...) {
    if (!component || !format) {
        return;
    }
    
    if (level < current_level) {
        return;
    }
    
    // Get timestamp
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    
    // Build message
    char message[MAX_LOG_MESSAGE];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    // FIXED: Print to BOTH file AND stderr (so you can see it in console)
    const char* level_str = log_level_string(level);
    
    // Write to log file
    if (log_file) {
        fprintf(log_file, "%s [%s] [%s] %s\n", 
                timestamp, level_str, component, message);
        fflush(log_file);
    }
    
    // ALSO write to stderr (console) for INFO and above
    if (level >= LOG_INFO) {
        fprintf(stderr, "[%s] [%s] %s\n", level_str, component, message);
    }
}

void logger_log_tool(const char* tool_name, LogLevel level, const char* message) {
    logger_log(level, tool_name, "%s", message);
}

void logger_set_level(LogLevel level) {
    current_level = level;
}

LogLevel logger_get_level(void) {
    return current_level;
}

int logger_rotate(void) {
    // Simple rotation: close and reopen
    if (log_file) {
        fclose(log_file);
        log_file = fopen(g_config.log_file, "a");
        if (!log_file) {
            return FW_ERROR_IO;
        }
    }
    return FW_OK;
}

void logger_set_max_size(size_t max_bytes) {
    // TODO: Implement log rotation based on size
    (void)max_bytes;
}
