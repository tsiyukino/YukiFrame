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
    
    // Try to open log file
    log_file = fopen(log_filename, "a");
    if (!log_file) {
        // Try current directory as fallback
        fprintf(stderr, "Failed to open log file: %s\n", log_filename);
        fprintf(stderr, "Trying current directory: yuki-frame.log\n");
        
        log_file = fopen("yuki-frame.log", "a");
        if (!log_file) {
            fprintf(stderr, "Failed to open fallback log file\n");
            return FW_ERROR_IO;
        }
    }
    
    current_level = level;
    
    // Write startup message
    time_t now = time(NULL);
    fprintf(log_file, "\n=== Logger initialized at %s", ctime(&now));
    fflush(log_file);
    
    return FW_OK;
}

void logger_shutdown(void) {
    if (log_file) {
        time_t now = time(NULL);
        fprintf(log_file, "=== Logger shutdown at %s", ctime(&now));
        fclose(log_file);
        log_file = NULL;
    }
}

void logger_log(LogLevel level, const char* component, const char* format, ...) {
    if (!component || !format) {
        return;
    }
    
    // If log file not open, try stderr
    FILE* output = log_file ? log_file : stderr;
    
    if (level < current_level) {
        return;
    }
    
    // Get timestamp
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    
    // Print level, component, and timestamp
    fprintf(output, "%s [%s] [%s] ", 
            timestamp, log_level_string(level), component);
    
    // Print message
    va_list args;
    va_start(args, format);
    vfprintf(output, format, args);
    va_end(args);
    
    fprintf(output, "\n");
    fflush(output);
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
