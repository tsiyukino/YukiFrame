#include "yuki_frame/framework.h"
#include "yuki_frame/logger.h"
#include "yuki_frame/platform.h"
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

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
    if (!command || !stdin_fd || !stdout_fd || !stderr_fd) {
        return -1;
    }

    int stdin_pipe[2], stdout_pipe[2], stderr_pipe[2];
    
    // Create stdin pipe
    if (pipe(stdin_pipe) < 0) {
        LOG_ERROR("platform", "Failed to create stdin pipe: %s", strerror(errno));
        return -1;
    }
    
    // Create stdout pipe
    if (pipe(stdout_pipe) < 0) {
        LOG_ERROR("platform", "Failed to create stdout pipe: %s", strerror(errno));
        close(stdin_pipe[0]);
        close(stdin_pipe[1]);
        return -1;
    }
    
    // Create stderr pipe
    if (pipe(stderr_pipe) < 0) {
        LOG_ERROR("platform", "Failed to create stderr pipe: %s", strerror(errno));
        close(stdin_pipe[0]);
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);
        return -1;
    }

    // Fork the process
    pid_t pid = fork();
    
    if (pid < 0) {
        // Fork failed
        LOG_ERROR("platform", "Fork failed: %s", strerror(errno));
        close(stdin_pipe[0]);
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);
        close(stderr_pipe[0]);
        close(stderr_pipe[1]);
        return -1;
    }
    
    if (pid == 0) {
        // Child process
        
        // Close parent sides of pipes
        close(stdin_pipe[1]);   // Close write end of stdin
        close(stdout_pipe[0]);  // Close read end of stdout
        close(stderr_pipe[0]);  // Close read end of stderr
        
        // Redirect stdin/stdout/stderr to pipes
        if (dup2(stdin_pipe[0], STDIN_FILENO) < 0) {
            fprintf(stderr, "Failed to redirect stdin: %s\n", strerror(errno));
            exit(1);
        }
        if (dup2(stdout_pipe[1], STDOUT_FILENO) < 0) {
            fprintf(stderr, "Failed to redirect stdout: %s\n", strerror(errno));
            exit(1);
        }
        if (dup2(stderr_pipe[1], STDERR_FILENO) < 0) {
            fprintf(stderr, "Failed to redirect stderr: %s\n", strerror(errno));
            exit(1);
        }
        
        // Close the pipe file descriptors (now duplicated)
        close(stdin_pipe[0]);
        close(stdout_pipe[1]);
        close(stderr_pipe[1]);
        
        // Execute command using shell
        execl("/bin/sh", "sh", "-c", command, (char*)NULL);
        
        // If we get here, exec failed
        fprintf(stderr, "Failed to execute command: %s\n", strerror(errno));
        exit(1);
    }
    
    // Parent process
    
    // Close child sides of pipes
    close(stdin_pipe[0]);   // Close read end of stdin
    close(stdout_pipe[1]);  // Close write end of stdout
    close(stderr_pipe[1]);  // Close write end of stderr
    
    // Return file descriptors
    *stdin_fd = stdin_pipe[1];   // Write to child's stdin
    *stdout_fd = stdout_pipe[0]; // Read from child's stdout
    *stderr_fd = stderr_pipe[0]; // Read from child's stderr
    
    LOG_INFO("platform", "Process spawned with PID %d", pid);
    
    return pid;
}

int platform_kill_process(ProcessHandle handle, bool force) {
    if (handle <= 0) {
        return FW_ERROR_INVALID_ARG;
    }
    
    int sig = force ? SIGKILL : SIGTERM;
    
    if (kill(handle, sig) == 0) {
        // Wait for process to exit (non-blocking)
        int status;
        pid_t result = waitpid(handle, &status, WNOHANG);
        
        if (result == handle) {
            // Process exited
            return FW_OK;
        } else if (result == 0) {
            // Process still running, but signal sent
            return FW_OK;
        }
    }
    
    if (errno == ESRCH) {
        // Process doesn't exist anymore
        return FW_OK;
    }
    
    LOG_ERROR("platform", "Failed to kill process %d: %s", handle, strerror(errno));
    return FW_ERROR_PROCESS_FAILED;
}

bool platform_is_process_running(ProcessHandle handle) {
    if (handle <= 0) {
        return false;
    }
    
    // Send signal 0 to check if process exists
    if (kill(handle, 0) == 0) {
        return true;
    }
    
    return (errno != ESRCH);
}

int platform_wait_process(ProcessHandle handle, int timeout_ms) {
    if (handle <= 0) {
        return FW_ERROR_INVALID_ARG;
    }
    
    int status;
    
    if (timeout_ms <= 0) {
        // No timeout - wait indefinitely
        pid_t result = waitpid(handle, &status, 0);
        if (result == handle) {
            return FW_OK;
        }
        return FW_ERROR_PROCESS_FAILED;
    }
    
    // Use WNOHANG for timeout support
    int elapsed = 0;
    int interval = 100; // Check every 100ms
    
    while (elapsed < timeout_ms) {
        pid_t result = waitpid(handle, &status, WNOHANG);
        
        if (result == handle) {
            // Process has exited
            return FW_OK;
        } else if (result < 0) {
            // Error
            return FW_ERROR_PROCESS_FAILED;
        }
        
        // Still running, sleep a bit
        usleep(interval * 1000);
        elapsed += interval;
    }
    
    // Timeout
    return FW_ERROR_TIMEOUT;
}

ProcessID platform_get_process_id(ProcessHandle handle) {
    return handle;
}

int platform_read_nonblocking(int fd, char* buffer, size_t size) {
    if (fd < 0 || !buffer || size == 0) {
        return FW_ERROR_INVALID_ARG;
    }
    
    ssize_t bytes_read = read(fd, buffer, size);
    
    if (bytes_read < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No data available (non-blocking mode)
            return 0;
        }
        LOG_ERROR("platform", "Read failed: %s", strerror(errno));
        return FW_ERROR_IO;
    }
    
    return (int)bytes_read;
}

int platform_write_nonblocking(int fd, const char* data, size_t size) {
    if (fd < 0 || !data || size == 0) {
        return FW_ERROR_INVALID_ARG;
    }
    
    ssize_t bytes_written = write(fd, data, size);
    
    if (bytes_written < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // Buffer full (non-blocking mode)
            return 0;
        }
        LOG_ERROR("platform", "Write failed: %s", strerror(errno));
        return FW_ERROR_IO;
    }
    
    return (int)bytes_written;
}

int platform_set_nonblocking(int fd) {
    if (fd < 0) {
        return FW_ERROR_INVALID_ARG;
    }
    
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        LOG_ERROR("platform", "fcntl F_GETFL failed: %s", strerror(errno));
        return FW_ERROR_IO;
    }
    
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        LOG_ERROR("platform", "fcntl F_SETFL failed: %s", strerror(errno));
        return FW_ERROR_IO;
    }
    
    return FW_OK;
}
