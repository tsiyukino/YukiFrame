#include "yuki_frame/framework.h"
#include "yuki_frame/logger.h"
#include "yuki_frame/platform.h"
#include <windows.h>
#include <io.h>
#include <fcntl.h>
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
    if (!command || !stdin_fd || !stdout_fd || !stderr_fd) {
        return INVALID_HANDLE_VALUE;
    }

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    // Create pipes for stdin/stdout/stderr
    HANDLE child_stdin_r = NULL, child_stdin_w = NULL;
    HANDLE child_stdout_r = NULL, child_stdout_w = NULL;
    HANDLE child_stderr_r = NULL, child_stderr_w = NULL;

    // Create stdin pipe
    if (!CreatePipe(&child_stdin_r, &child_stdin_w, &sa, 0)) {
        LOG_ERROR("platform", "Failed to create stdin pipe");
        return INVALID_HANDLE_VALUE;
    }

    // Create stdout pipe
    if (!CreatePipe(&child_stdout_r, &child_stdout_w, &sa, 0)) {
        LOG_ERROR("platform", "Failed to create stdout pipe");
        CloseHandle(child_stdin_r);
        CloseHandle(child_stdin_w);
        return INVALID_HANDLE_VALUE;
    }

    // Create stderr pipe
    if (!CreatePipe(&child_stderr_r, &child_stderr_w, &sa, 0)) {
        LOG_ERROR("platform", "Failed to create stderr pipe");
        CloseHandle(child_stdin_r);
        CloseHandle(child_stdin_w);
        CloseHandle(child_stdout_r);
        CloseHandle(child_stdout_w);
        return INVALID_HANDLE_VALUE;
    }

    // Ensure parent sides of pipes are not inherited
    SetHandleInformation(child_stdin_w, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(child_stdout_r, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(child_stderr_r, HANDLE_FLAG_INHERIT, 0);

    // Setup STARTUPINFO
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdInput = child_stdin_r;
    si.hStdOutput = child_stdout_w;
    si.hStdError = child_stderr_w;
    si.dwFlags |= STARTF_USESTDHANDLES;
    
    ZeroMemory(&pi, sizeof(pi));

    // Create the child process
    char cmd_buf[MAX_COMMAND_LENGTH];
    strncpy(cmd_buf, command, sizeof(cmd_buf) - 1);
    cmd_buf[sizeof(cmd_buf) - 1] = '\0';
    
    BOOL result = CreateProcessA(
        NULL,           // No module name (use command line)
        cmd_buf,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        TRUE,           // Set handle inheritance to TRUE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi             // Pointer to PROCESS_INFORMATION structure
    );

    if (!result) {
        DWORD error = GetLastError();
        LOG_ERROR("platform", "CreateProcess failed with error %lu", error);
        CloseHandle(child_stdin_r);
        CloseHandle(child_stdin_w);
        CloseHandle(child_stdout_r);
        CloseHandle(child_stdout_w);
        CloseHandle(child_stderr_r);
        CloseHandle(child_stderr_w);
        return INVALID_HANDLE_VALUE;
    }

    // Close child-side handles in parent process
    CloseHandle(child_stdin_r);
    CloseHandle(child_stdout_w);
    CloseHandle(child_stderr_w);

    // Convert Windows handles to file descriptors
    *stdin_fd = _open_osfhandle((intptr_t)child_stdin_w, _O_WRONLY);
    *stdout_fd = _open_osfhandle((intptr_t)child_stdout_r, _O_RDONLY);
    *stderr_fd = _open_osfhandle((intptr_t)child_stderr_r, _O_RDONLY);

    // Close thread handle (not needed)
    CloseHandle(pi.hThread);

    LOG_INFO("platform", "Process spawned with PID %lu", pi.dwProcessId);

    // Return process handle
    return pi.hProcess;
}

int platform_kill_process(ProcessHandle handle, bool force) {
    if (handle == INVALID_HANDLE_VALUE) {
        return FW_ERROR_INVALID_ARG;
    }
    
    UINT exit_code = force ? 1 : 0;
    
    if (TerminateProcess(handle, exit_code)) {
        CloseHandle(handle);
        return FW_OK;
    }
    
    DWORD error = GetLastError();
    LOG_ERROR("platform", "TerminateProcess failed with error %lu", error);
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
    
    DWORD wait_time = (timeout_ms <= 0) ? INFINITE : (DWORD)timeout_ms;
    DWORD result = WaitForSingleObject(handle, wait_time);
    
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
    if (fd < 0 || !buffer || size == 0) {
        return FW_ERROR_INVALID_ARG;
    }

    // Get the HANDLE from file descriptor
    HANDLE handle = (HANDLE)_get_osfhandle(fd);
    if (handle == INVALID_HANDLE_VALUE) {
        return FW_ERROR_IO;
    }

    // Check if data is available
    DWORD bytes_available = 0;
    if (!PeekNamedPipe(handle, NULL, 0, NULL, &bytes_available, NULL)) {
        DWORD error = GetLastError();
        if (error == ERROR_BROKEN_PIPE) {
            return 0; // Pipe closed
        }
        return FW_ERROR_IO;
    }

    if (bytes_available == 0) {
        return 0; // No data available
    }

    // Read available data
    DWORD bytes_read = 0;
    DWORD to_read = (bytes_available < size) ? bytes_available : (DWORD)size;
    
    if (!ReadFile(handle, buffer, to_read, &bytes_read, NULL)) {
        DWORD error = GetLastError();
        if (error == ERROR_BROKEN_PIPE) {
            return 0; // Pipe closed
        }
        return FW_ERROR_IO;
    }

    return (int)bytes_read;
}

int platform_write_nonblocking(int fd, const char* data, size_t size) {
    if (fd < 0 || !data || size == 0) {
        return FW_ERROR_INVALID_ARG;
    }

    // Get the HANDLE from file descriptor
    HANDLE handle = (HANDLE)_get_osfhandle(fd);
    if (handle == INVALID_HANDLE_VALUE) {
        return FW_ERROR_IO;
    }

    DWORD bytes_written = 0;
    if (!WriteFile(handle, data, (DWORD)size, &bytes_written, NULL)) {
        DWORD error = GetLastError();
        LOG_ERROR("platform", "WriteFile failed with error %lu", error);
        return FW_ERROR_IO;
    }

    return (int)bytes_written;
}

int platform_set_nonblocking(int fd) {
    // On Windows, named pipes are non-blocking by default when using PeekNamedPipe
    // No additional setup needed for our use case
    (void)fd;
    return FW_OK;
}
