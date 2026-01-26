/**
 * @file control_socket.c
 * @brief Control Socket Server Implementation (Windows)
 * 
 * This is PART OF THE FRAMEWORK CORE!
 * Provides socket interface to Control API functions.
 * 
 * Architecture:
 * - Runs in separate thread within framework process
 * - Listens on TCP socket (default port 9999)
 * - Accepts connections from console/scripts
 * - Executes Control API commands
 * - Returns responses to clients
 */

#include "yuki_frame/framework.h"
#include "yuki_frame/control_socket.h"
#include "yuki_frame/control_api.h"
#include "yuki_frame/logger.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
#include <string.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

// Socket server state
static SOCKET g_listen_socket = INVALID_SOCKET;
static HANDLE g_server_thread = NULL;
static bool g_socket_running = false;
static int g_listen_port = 0;
static CRITICAL_SECTION g_socket_lock;

// Forward declarations
static unsigned int __stdcall socket_server_thread(void* arg);
static void handle_client_connection(SOCKET client_socket);

/**
 * Initialize control socket system
 */
int control_socket_init(void) {
    // Initialize Winsock
    WSADATA wsa_data;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != 0) {
        LOG_ERROR("control_socket", "WSAStartup failed: %d", result);
        return FW_ERROR_GENERIC;
    }
    
    // Initialize critical section for thread safety
    InitializeCriticalSection(&g_socket_lock);
    
    LOG_INFO("control_socket", "Control socket system initialized");
    return FW_OK;
}

/**
 * Start control socket server
 */
int control_socket_start(int port) {
    if (g_socket_running) {
        LOG_WARN("control_socket", "Control socket already running");
        return FW_OK;
    }
    
    if (port <= 0 || port > 65535) {
        LOG_ERROR("control_socket", "Invalid port: %d", port);
        return FW_ERROR_INVALID_ARG;
    }
    
    g_listen_port = port;
    
    // Create socket
    g_listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (g_listen_socket == INVALID_SOCKET) {
        LOG_ERROR("control_socket", "Failed to create socket: %d", WSAGetLastError());
        return FW_ERROR_GENERIC;
    }
    
    // Set socket options
    int opt = 1;
    setsockopt(g_listen_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    
    // Bind socket
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);  // localhost only
    server_addr.sin_port = htons((u_short)port);
    
    if (bind(g_listen_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        LOG_ERROR("control_socket", "Failed to bind socket: %d", WSAGetLastError());
        closesocket(g_listen_socket);
        g_listen_socket = INVALID_SOCKET;
        return FW_ERROR_GENERIC;
    }
    
    // Listen
    if (listen(g_listen_socket, SOMAXCONN) == SOCKET_ERROR) {
        LOG_ERROR("control_socket", "Failed to listen: %d", WSAGetLastError());
        closesocket(g_listen_socket);
        g_listen_socket = INVALID_SOCKET;
        return FW_ERROR_GENERIC;
    }
    
    // Start server thread
    g_socket_running = true;
    g_server_thread = (HANDLE)_beginthreadex(
        NULL,                   // Security
        0,                      // Stack size
        socket_server_thread,   // Thread function
        NULL,                   // Argument
        0,                      // Flags
        NULL                    // Thread ID
    );
    
    if (g_server_thread == NULL) {
        LOG_ERROR("control_socket", "Failed to create server thread");
        g_socket_running = false;
        closesocket(g_listen_socket);
        g_listen_socket = INVALID_SOCKET;
        return FW_ERROR_GENERIC;
    }
    
    LOG_INFO("control_socket", "Control socket server started on port %d", port);
    LOG_INFO("control_socket", "Clients can connect to localhost:%d", port);
    
    return FW_OK;
}

/**
 * Stop control socket server
 */
void control_socket_stop(void) {
    if (!g_socket_running) {
        return;
    }
    
    LOG_INFO("control_socket", "Stopping control socket server");
    
    // Signal thread to stop
    g_socket_running = false;
    
    // Close listening socket (this will unblock accept())
    if (g_listen_socket != INVALID_SOCKET) {
        closesocket(g_listen_socket);
        g_listen_socket = INVALID_SOCKET;
    }
    
    // Wait for thread to finish (with timeout)
    if (g_server_thread != NULL) {
        WaitForSingleObject(g_server_thread, 2000);
        CloseHandle(g_server_thread);
        g_server_thread = NULL;
    }
    
    LOG_INFO("control_socket", "Control socket server stopped");
}

/**
 * Shutdown control socket system
 */
void control_socket_shutdown(void) {
    control_socket_stop();
    
    // Cleanup Winsock
    WSACleanup();
    
    // Cleanup critical section
    DeleteCriticalSection(&g_socket_lock);
    
    LOG_INFO("control_socket", "Control socket system shutdown");
}

/**
 * Check if control socket is running
 */
bool control_socket_is_running(void) {
    return g_socket_running;
}

/**
 * Get the port the socket is listening on
 */
int control_socket_get_port(void) {
    return g_listen_port;
}

/**
 * Socket server thread function
 * 
 * This runs in the framework process, in a separate thread.
 * It continuously accepts connections and handles them.
 */
static unsigned int __stdcall socket_server_thread(void* arg) {
    (void)arg;  // Unused
    
    LOG_INFO("control_socket", "Socket server thread started");
    
    while (g_socket_running) {
        // Accept connection
        struct sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);
        
        SOCKET client_socket = accept(
            g_listen_socket,
            (struct sockaddr*)&client_addr,
            &client_addr_len
        );
        
        if (client_socket == INVALID_SOCKET) {
            if (g_socket_running) {
                // Only log error if we're still supposed to be running
                int error = WSAGetLastError();
                if (error != WSAEINTR) {
                    LOG_ERROR("control_socket", "Accept failed: %d", error);
                }
            }
            break;
        }
        
        // Get client info
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        LOG_INFO("control_socket", "Client connected from %s:%d",
                 client_ip, ntohs(client_addr.sin_port));
        
        // Handle client in same thread (simple approach)
        // For production, consider thread pool or async I/O
        handle_client_connection(client_socket);
        
        // Close client socket
        closesocket(client_socket);
        
        LOG_INFO("control_socket", "Client disconnected");
    }
    
    LOG_INFO("control_socket", "Socket server thread stopped");
    return 0;
}

/**
 * Handle client connection
 * 
 * Keeps connection open and handles multiple commands until client disconnects.
 */
static void handle_client_connection(SOCKET client_socket) {
    char buffer[4096];
    char response[8192];
    int bytes_received;
    
    // Set receive timeout (30 seconds of inactivity)
    int timeout = 30000;
    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    
    // Keep connection alive and handle multiple commands
    while (g_socket_running) {
        // Receive command
        bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                LOG_DEBUG("control_socket", "Client closed connection gracefully");
            } else {
                int error = WSAGetLastError();
                if (error == WSAETIMEDOUT) {
                    LOG_DEBUG("control_socket", "Client timeout");
                } else {
                    LOG_ERROR("control_socket", "Receive failed: %d", error);
                }
            }
            break;  // Exit loop, connection closed
        }
        
        buffer[bytes_received] = '\0';
        
        // Remove trailing newline/carriage return
        while (bytes_received > 0 && 
               (buffer[bytes_received - 1] == '\n' || buffer[bytes_received - 1] == '\r')) {
            buffer[--bytes_received] = '\0';
        }
        
        // Skip empty commands
        if (bytes_received == 0) {
            continue;
        }
        
        LOG_DEBUG("control_socket", "Received command: %s", buffer);
        
        // Execute command using Control API (FRAMEWORK FUNCTION!)
        EnterCriticalSection(&g_socket_lock);
        int result = control_execute_command(buffer, response, sizeof(response));
        LeaveCriticalSection(&g_socket_lock);
        
        if (result != FW_OK) {
            snprintf(response, sizeof(response), "Error: Command execution failed\n");
        }
        
        // Send response
        int bytes_sent = send(client_socket, response, (int)strlen(response), 0);
        
        if (bytes_sent == SOCKET_ERROR) {
            LOG_ERROR("control_socket", "Send failed: %d", WSAGetLastError());
            break;  // Connection broken
        } else {
            LOG_DEBUG("control_socket", "Sent %d bytes response", bytes_sent);
        }
        
        // Check if shutdown command was received
        if (strcmp(buffer, "shutdown") == 0) {
            LOG_INFO("control_socket", "Shutdown command received, closing connection");
            break;
        }
    }
}
