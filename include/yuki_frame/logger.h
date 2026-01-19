#ifndef YUKI_FRAME_LOGGER_H
#define YUKI_FRAME_LOGGER_H

#include "yuki_frame/framework.h"
#include <stdarg.h>

// Logger initialization
int logger_init(const char* log_file, LogLevel level);
void logger_shutdown(void);

// Logging functions
void logger_log(LogLevel level, const char* component, const char* format, ...);
void logger_log_tool(const char* tool_name, LogLevel level, const char* message);
void logger_set_level(LogLevel level);
LogLevel logger_get_level(void);

// Log macros
#define LOG_TRACE(component, ...) logger_log(LOG_TRACE, component, __VA_ARGS__)
#define LOG_DEBUG(component, ...) logger_log(LOG_DEBUG, component, __VA_ARGS__)
#define LOG_INFO(component, ...) logger_log(LOG_INFO, component, __VA_ARGS__)
#define LOG_WARN(component, ...) logger_log(LOG_WARN, component, __VA_ARGS__)
#define LOG_ERROR(component, ...) logger_log(LOG_ERROR, component, __VA_ARGS__)
#define LOG_FATAL(component, ...) logger_log(LOG_FATAL, component, __VA_ARGS__)

// Log rotation
int logger_rotate(void);
void logger_set_max_size(size_t max_bytes);

#endif  // YUKI_FRAME_LOGGER_H
