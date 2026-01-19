/**
 * @file test_event.c
 * @brief Unit tests for event bus module
 */

#include "yuki_frame/event.h"
#include "yuki_frame/framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Global state (required by framework modules)
FrameworkConfig g_config;
bool g_running = true;

// Test counter
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

// Test macros
#define TEST(name) \
    static void test_##name(void); \
    static void run_test_##name(void) { \
        printf("  Running: %s ... ", #name); \
        tests_run++; \
        test_##name(); \
        tests_passed++; \
        printf("PASS\n"); \
    } \
    static void test_##name(void)

#define ASSERT(condition) \
    do { \
        if (!(condition)) { \
            printf("FAIL\n    Assertion failed: %s\n    at %s:%d\n", \
                   #condition, __FILE__, __LINE__); \
            tests_failed++; \
            tests_passed--; \
            return; \
        } \
    } while(0)

#define ASSERT_EQ(a, b) ASSERT((a) == (b))
#define ASSERT_NE(a, b) ASSERT((a) != (b))
#define ASSERT_STR_EQ(a, b) ASSERT(strcmp((a), (b)) == 0)

// Tests
TEST(event_bus_init_success) {
    int result = event_bus_init();
    ASSERT_EQ(result, FW_OK);
    event_bus_shutdown();
}

TEST(event_publish_valid_event) {
    event_bus_init();
    
    int result = event_publish("test.event", "test_tool", "{\"key\":\"value\"}");
    ASSERT_EQ(result, FW_OK);
    
    event_bus_shutdown();
}

TEST(event_publish_null_type_fails) {
    event_bus_init();
    
    int result = event_publish(NULL, "test_tool", "{}");
    ASSERT_EQ(result, FW_ERROR_INVALID_ARG);
    
    event_bus_shutdown();
}

TEST(event_publish_null_sender_fails) {
    event_bus_init();
    
    int result = event_publish("test.event", NULL, "{}");
    ASSERT_EQ(result, FW_ERROR_INVALID_ARG);
    
    event_bus_shutdown();
}

TEST(event_parse_valid_format) {
    Event event;
    const char* line = "test.event|sender_tool|{\"data\":\"value\"}";
    
    int result = event_parse(line, &event);
    ASSERT_EQ(result, FW_OK);
    ASSERT_STR_EQ(event.type, "test.event");
    ASSERT_STR_EQ(event.sender, "sender_tool");
}

TEST(event_parse_invalid_format) {
    Event event;
    const char* line = "invalid_format_no_pipes";
    
    int result = event_parse(line, &event);
    ASSERT_NE(result, FW_OK);
}

TEST(event_parse_null_line_fails) {
    Event event;
    
    int result = event_parse(NULL, &event);
    ASSERT_EQ(result, FW_ERROR_INVALID_ARG);
}

TEST(event_parse_null_event_fails) {
    const char* line = "test|sender|data";
    
    int result = event_parse(line, NULL);
    ASSERT_EQ(result, FW_ERROR_INVALID_ARG);
}

TEST(event_format_valid_event) {
    Event event;
    strncpy(event.type, "test.event", MAX_EVENT_TYPE);
    strncpy(event.sender, "test_sender", MAX_TOOL_NAME);
    strncpy(event.data, "{\"key\":\"value\"}", MAX_EVENT_DATA);
    
    char buffer[1024];
    int result = event_format(&event, buffer, sizeof(buffer));
    
    ASSERT_EQ(result, FW_OK);
    ASSERT(strstr(buffer, "test.event") != NULL);
    ASSERT(strstr(buffer, "test_sender") != NULL);
}

TEST(event_format_null_event_fails) {
    char buffer[1024];
    
    int result = event_format(NULL, buffer, sizeof(buffer));
    ASSERT_EQ(result, FW_ERROR_INVALID_ARG);
}

TEST(event_format_null_buffer_fails) {
    Event event;
    strncpy(event.type, "test", MAX_EVENT_TYPE);
    strncpy(event.sender, "sender", MAX_TOOL_NAME);
    strncpy(event.data, "data", MAX_EVENT_DATA);
    
    int result = event_format(&event, NULL, 1024);
    ASSERT_EQ(result, FW_ERROR_INVALID_ARG);
}

TEST(event_format_buffer_too_small) {
    Event event;
    strncpy(event.type, "test.event", MAX_EVENT_TYPE);
    strncpy(event.sender, "test_sender", MAX_TOOL_NAME);
    strncpy(event.data, "{\"key\":\"value\"}", MAX_EVENT_DATA);
    
    char buffer[10];
    int result = event_format(&event, buffer, sizeof(buffer));
    
    // Should handle gracefully (truncate or error)
    ASSERT(result == FW_OK || result < 0);
}

// Test runner
int main(void) {
    printf("\n=== Event Module Unit Tests ===\n\n");
    
    run_test_event_bus_init_success();
    run_test_event_publish_valid_event();
    run_test_event_publish_null_type_fails();
    run_test_event_publish_null_sender_fails();
    run_test_event_parse_valid_format();
    run_test_event_parse_invalid_format();
    run_test_event_parse_null_line_fails();
    run_test_event_parse_null_event_fails();
    run_test_event_format_valid_event();
    run_test_event_format_null_event_fails();
    run_test_event_format_null_buffer_fails();
    run_test_event_format_buffer_too_small();
    
    printf("\n=== Test Summary ===\n");
    printf("  Total:  %d\n", tests_run);
    printf("  Passed: %d\n", tests_passed);
    printf("  Failed: %d\n", tests_failed);
    printf("\n");
    
    return tests_failed == 0 ? 0 : 1;
}
