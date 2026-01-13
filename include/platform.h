#ifndef PLATFORM_H
#define PLATFORM_H

#include "framework.h"

// Platform-specific process functions
ProcessHandle platform_spawn_process(const char* command, int* stdin_fd, int* stdout_fd, int* stderr_fd);
int platform_kill_process(ProcessHandle handle, bool force);
bool platform_is_process_running(ProcessHandle handle);
int platform_wait_process(ProcessHandle handle, int timeout_ms);
ProcessID platform_get_process_id(ProcessHandle handle);

// Platform-specific I/O
int platform_read_nonblocking(int fd, char* buffer, size_t size);
int platform_write_nonblocking(int fd, const char* data, size_t size);
int platform_set_nonblocking(int fd);

// Platform-specific utilities
void platform_sleep_ms(int milliseconds);
void platform_sleep(int seconds);

// Platform-specific initialization
int platform_init(void);
void platform_shutdown(void);

#endif // PLATFORM_H
