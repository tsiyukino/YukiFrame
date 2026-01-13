#include "framework.h"
#include "logger.h"
#include "platform.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>

int platform_init(void) {
    LOG_INFO("platform", "Windows platform initialized");
    return FW_OK;
}

void platform_shutdown(void) {
    LOG_INFO("platform", "Windows platform shutdown");
}

void platform_sleep_ms(int milliseconds) {
    Sleep(milliseconds);
}

void platform_sleep(int seconds) {
    Sleep(seconds * 1000);
}

ProcessHandle platform_spawn_process(const char* command, int* stdin_fd, int* stdout_fd, int* stderr_fd) {
    // TODO: Implement Windows process spawning with CreateProcess
    // This is a stub - needs full implementation
    LOG_ERROR("platform", "platform_spawn_process not yet implemented");
    return INVALID_HANDLE_VALUE;
}

int platform_kill_process(ProcessHandle handle, bool force) {
    if (handle == INVALID_HANDLE_VALUE) {
        return FW_ERROR_INVALID_ARG;
    }
    
    if (force) {
        if (TerminateProcess(handle, 1)) {
            return FW_OK;
        }
    } else {
        // Try graceful shutdown first
        if (TerminateProcess(handle, 0)) {
            return FW_OK;
        }
    }
    
    return FW_ERROR_PROCESS_FAILED;
}

bool platform_is_process_running(ProcessHandle handle) {
    if (handle == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    DWORD exit_code;
    if (GetExitCodeProcess(handle, &exit_code)) {
        return (exit_code == STILL_ACTIVE);
    }
    
    return false;
}

int platform_wait_process(ProcessHandle handle, int timeout_ms) {
    if (handle == INVALID_HANDLE_VALUE) {
        return FW_ERROR_INVALID_ARG;
    }
    
    DWORD result = WaitForSingleObject(handle, timeout_ms);
    if (result == WAIT_OBJECT_0) {
        return FW_OK;
    } else if (result == WAIT_TIMEOUT) {
        return FW_ERROR_TIMEOUT;
    }
    
    return FW_ERROR_PROCESS_FAILED;
}

ProcessID platform_get_process_id(ProcessHandle handle) {
    if (handle == INVALID_HANDLE_VALUE) {
        return 0;
    }
    
    return GetProcessId(handle);
}

int platform_read_nonblocking(int fd, char* buffer, size_t size) {
    // TODO: Implement Windows non-blocking read
    LOG_ERROR("platform", "platform_read_nonblocking not yet implemented");
    return FW_ERROR_IO;
}

int platform_write_nonblocking(int fd, const char* data, size_t size) {
    // TODO: Implement Windows non-blocking write
    LOG_ERROR("platform", "platform_write_nonblocking not yet implemented");
    return FW_ERROR_IO;
}

int platform_set_nonblocking(int fd) {
    // TODO: Implement Windows non-blocking mode
    LOG_ERROR("platform", "platform_set_nonblocking not yet implemented");
    return FW_ERROR_IO;
}
