#include "framework.h"
#include "logger.h"
#include "platform.h"
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int platform_init(void) {
    LOG_INFO("platform", "Linux platform initialized");
    return FW_OK;
}

void platform_shutdown(void) {
    LOG_INFO("platform", "Linux platform shutdown");
}

void platform_sleep_ms(int milliseconds) {
    usleep(milliseconds * 1000);
}

void platform_sleep(int seconds) {
    sleep(seconds);
}

ProcessHandle platform_spawn_process(const char* command, int* stdin_fd, int* stdout_fd, int* stderr_fd) {
    // TODO: Implement Linux process spawning with fork/exec
    // This is a stub - needs full implementation
    LOG_ERROR("platform", "platform_spawn_process not yet implemented");
    return -1;
}

int platform_kill_process(ProcessHandle handle, bool force) {
    if (handle <= 0) {
        return FW_ERROR_INVALID_ARG;
    }
    
    int sig = force ? SIGKILL : SIGTERM;
    if (kill(handle, sig) == 0) {
        return FW_OK;
    }
    
    return FW_ERROR_PROCESS_FAILED;
}

bool platform_is_process_running(ProcessHandle handle) {
    if (handle <= 0) {
        return false;
    }
    
    // Send signal 0 to check if process exists
    return (kill(handle, 0) == 0);
}

int platform_wait_process(ProcessHandle handle, int timeout_ms) {
    if (handle <= 0) {
        return FW_ERROR_INVALID_ARG;
    }
    
    // Simple implementation - doesn't actually use timeout
    int status;
    pid_t result = waitpid(handle, &status, WNOHANG);
    
    if (result == handle) {
        return FW_OK;  // Process has exited
    } else if (result == 0) {
        return FW_ERROR_TIMEOUT;  // Still running
    }
    
    return FW_ERROR_PROCESS_FAILED;
}

ProcessID platform_get_process_id(ProcessHandle handle) {
    return handle;
}

int platform_read_nonblocking(int fd, char* buffer, size_t size) {
    // TODO: Implement Linux non-blocking read
    LOG_ERROR("platform", "platform_read_nonblocking not yet implemented");
    return FW_ERROR_IO;
}

int platform_write_nonblocking(int fd, const char* data, size_t size) {
    // TODO: Implement Linux non-blocking write
    LOG_ERROR("platform", "platform_write_nonblocking not yet implemented");
    return FW_ERROR_IO;
}

int platform_set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        return FW_ERROR_IO;
    }
    
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        return FW_ERROR_IO;
    }
    
    return FW_OK;
}
