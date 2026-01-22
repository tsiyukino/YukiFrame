/**
 * @file console.h
 * @brief Interactive console interface
 */

#ifndef YUKI_FRAME_CONSOLE_H
#define YUKI_FRAME_CONSOLE_H

#include <stdbool.h>

/**
 * Initialize console system
 * 
 * @return 0 on success, negative error code on failure
 */
int console_init(void);

/**
 * Start interactive console thread
 * 
 * @return 0 on success, negative error code on failure
 */
int console_start(void);

/**
 * Stop console thread
 */
void console_stop(void);

/**
 * Shutdown console system
 */
void console_shutdown(void);

/**
 * Check if console is running
 * 
 * @return true if console is active, false otherwise
 */
bool console_is_running(void);

#endif  // YUKI_FRAME_CONSOLE_H
