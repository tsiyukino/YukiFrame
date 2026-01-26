/**
 * @file control_socket.h
 * @brief Control Socket Server (Integrated into Framework)
 * 
 * This is PART OF THE FRAMEWORK - not an external tool!
 * Provides socket-based interface to Control API.
 * 
 * @copyright Copyright (c) 2026 Yuki-Frame Project
 * @license MIT License
 */

#ifndef YUKI_FRAME_CONTROL_SOCKET_H
#define YUKI_FRAME_CONTROL_SOCKET_H

#include <stdbool.h>

/**
 * Initialize control socket system
 * 
 * This prepares the socket system but doesn't start listening yet.
 * 
 * @return 0 on success, negative error code on failure
 */
int control_socket_init(void);

/**
 * Start control socket server
 * 
 * This starts the socket listener thread. The thread runs in the
 * framework process and listens for incoming connections.
 * 
 * @param port Port number to listen on (default: 9999)
 * @return 0 on success, negative error code on failure
 */
int control_socket_start(int port);

/**
 * Stop control socket server
 * 
 * Stops the socket listener thread and closes all connections.
 */
void control_socket_stop(void);

/**
 * Shutdown control socket system
 * 
 * Cleans up all socket resources.
 */
void control_socket_shutdown(void);

/**
 * Check if control socket is running
 * 
 * @return true if socket server is active, false otherwise
 */
bool control_socket_is_running(void);

/**
 * Get the port the socket is listening on
 * 
 * @return Port number, or 0 if not listening
 */
int control_socket_get_port(void);

#endif  // YUKI_FRAME_CONTROL_SOCKET_H
