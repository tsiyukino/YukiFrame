/**
 * @file console.c
 * @brief Interactive console for framework control
 * 
 * This provides an interactive console that can be enabled in the framework.
 * It runs in a separate thread and accepts user commands.
 */

#include "yuki_frame/framework.h"
#include "yuki_frame/control_api.h"
#include "yuki_frame/logger.h"
#include "yuki_frame/platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

static pthread_t g_console_thread;
static bool g_console_running = false;
static bool g_console_enabled = false;

// Console thread function
static void* console_thread_func(void* arg) {
    (void)arg;  // Unused
    
    char command[1024];
    char response[4096];
    
    printf("\n");
    printf("============================================================\n");
    printf("  Yuki-Frame Interactive Console v%s\n", YUKI_FRAME_VERSION_STRING);
    printf("  Type 'help' for commands, 'quit' to exit console\n");
    printf("============================================================\n");
    printf("\n");
    
    while (g_console_running) {
        printf("yuki> ");
        fflush(stdout);
        
        if (!fgets(command, sizeof(command), stdin)) {
            break;  // EOF or error
        }
        
        // Remove newline
        command[strcspn(command, "\n")] = 0;
        
        // Skip empty commands
        if (strlen(command) == 0) {
            continue;
        }
        
        // Handle local quit command
        if (strcmp(command, "quit") == 0 || strcmp(command, "exit") == 0) {
            printf("Exiting console mode (framework continues running)...\n");
            g_console_running = false;
            break;
        }
        
        // Execute command through Control API
        int result = control_execute_command(command, response, sizeof(response));
        
        // Display response
        printf("%s", response);
        
        // If shutdown was requested, exit console
        if (strcmp(command, "shutdown") == 0 && result == FW_OK) {
            g_console_running = false;
            break;
        }
    }
    
    return NULL;
}

// Initialize console
int console_init(void) {
    if (g_console_enabled) {
        LOG_WARN("console", "Console already initialized");
        return FW_OK;
    }
    
    g_console_enabled = true;
    LOG_INFO("console", "Interactive console initialized");
    return FW_OK;
}

// Start console thread
int console_start(void) {
    if (!g_console_enabled) {
        LOG_ERROR("console", "Console not initialized");
        return FW_ERROR_GENERIC;
    }
    
    if (g_console_running) {
        LOG_WARN("console", "Console already running");
        return FW_OK;
    }
    
    g_console_running = true;
    
    // Create console thread
    int ret = pthread_create(&g_console_thread, NULL, console_thread_func, NULL);
    if (ret != 0) {
        LOG_ERROR("console", "Failed to create console thread");
        g_console_running = false;
        return FW_ERROR_GENERIC;
    }
    
    LOG_INFO("console", "Interactive console started");
    return FW_OK;
}

// Stop console
void console_stop(void) {
    if (!g_console_running) {
        return;
    }
    
    g_console_running = false;
    
    // Wait for console thread to finish
    pthread_join(g_console_thread, NULL);
    
    LOG_INFO("console", "Interactive console stopped");
}

// Shutdown console
void console_shutdown(void) {
    if (!g_console_enabled) {
        return;
    }
    
    console_stop();
    g_console_enabled = false;
    
    LOG_INFO("console", "Interactive console shutdown");
}

// Check if console is running
bool console_is_running(void) {
    return g_console_running;
}
