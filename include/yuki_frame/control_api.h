/**
 * @file control_api.h
 * @brief Framework Control API - Public interface for tool management
 * 
 * This API allows tools to control other tools in the framework.
 * Any tool can use these functions to start, stop, query status, etc.
 * 
 * @copyright Copyright (c) 2024
 * @license MIT License
 */

#ifndef YUKI_FRAME_CONTROL_API_H
#define YUKI_FRAME_CONTROL_API_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Tool status information structure
 */
typedef struct {
    char name[256];              /**< Tool name */
    char command[1024];          /**< Command line */
    char description[256];       /**< Tool description */
    int status;                  /**< Tool status (TOOL_STOPPED, TOOL_RUNNING, etc.) */
    uint32_t pid;                /**< Process ID (0 if not running) */
    bool autostart;              /**< Auto-start on framework startup */
    bool restart_on_crash;       /**< Restart if crashed */
    int max_restarts;            /**< Maximum restart attempts */
    int restart_count;           /**< Current restart count */
    uint64_t events_sent;        /**< Number of events sent */
    uint64_t events_received;    /**< Number of events received */
    int subscription_count;      /**< Number of subscriptions */
} ControlToolInfo;

/**
 * List callback function type
 * 
 * @param info Tool information
 * @param user_data User-provided data
 * @return true to continue iteration, false to stop
 */
typedef bool (*control_list_callback_t)(const ControlToolInfo* info, void* user_data);

/* ============================================================================
 * Tool Management API
 * ============================================================================ */

/**
 * Start a tool by name
 * 
 * @param tool_name Name of the tool to start
 * @return 0 on success, negative error code on failure
 *         -1 (FW_ERROR_NOT_FOUND) if tool not found
 *         -2 (FW_ERROR_GENERIC) if start failed
 */
int control_start_tool(const char* tool_name);

/**
 * Stop a tool by name
 * 
 * @param tool_name Name of the tool to stop
 * @return 0 on success, negative error code on failure
 *         -1 (FW_ERROR_NOT_FOUND) if tool not found
 *         -2 (FW_ERROR_GENERIC) if stop failed
 */
int control_stop_tool(const char* tool_name);

/**
 * Restart a tool by name (stop then start)
 * 
 * @param tool_name Name of the tool to restart
 * @return 0 on success, negative error code on failure
 *         -1 (FW_ERROR_NOT_FOUND) if tool not found
 *         -2 (FW_ERROR_GENERIC) if restart failed
 */
int control_restart_tool(const char* tool_name);

/**
 * Get status information for a specific tool
 * 
 * @param tool_name Name of the tool
 * @param info Output buffer for tool information (caller-allocated)
 * @return 0 on success, negative error code on failure
 *         -1 (FW_ERROR_NOT_FOUND) if tool not found
 */
int control_get_tool_status(const char* tool_name, ControlToolInfo* info);

/**
 * List all tools using a callback function
 * 
 * The callback is called once for each tool. Return false from callback
 * to stop iteration early.
 * 
 * @param callback Function to call for each tool
 * @param user_data User data to pass to callback
 * @return Number of tools processed, or negative error code
 */
int control_list_tools(control_list_callback_t callback, void* user_data);

/**
 * Get total number of registered tools
 * 
 * @return Number of tools, or 0 if none registered
 */
int control_get_tool_count(void);

/**
 * Check if a tool exists in the registry
 * 
 * @param tool_name Name of the tool
 * @return true if tool exists, false otherwise
 */
bool control_tool_exists(const char* tool_name);

/* ============================================================================
 * Framework Control API
 * ============================================================================ */

/**
 * Request framework shutdown
 * 
 * Signals the framework to perform graceful shutdown.
 * All tools will be stopped before framework exits.
 * 
 * @return 0 on success
 */
int control_shutdown_framework(void);

/**
 * Get framework uptime in seconds
 * 
 * @return Uptime in seconds since framework started
 */
uint64_t control_get_uptime(void);

/**
 * Get framework version string
 * 
 * @return Version string (e.g., "2.0.0")
 */
const char* control_get_version(void);

/* ============================================================================
 * Interactive Console API (Optional)
 * ============================================================================ */

/**
 * Process a control command string
 * 
 * This is a convenience function that parses and executes commands.
 * Useful for building interactive consoles or remote control interfaces.
 * 
 * @param command Command string (e.g., "start my_tool", "list", "status my_tool")
 * @param response Output buffer for response message (caller-allocated)
 * @param response_size Size of response buffer
 * @return 0 on success, negative error code on failure
 */
int control_execute_command(const char* command, char* response, size_t response_size);

#endif  // YUKI_FRAME_CONTROL_API_H
