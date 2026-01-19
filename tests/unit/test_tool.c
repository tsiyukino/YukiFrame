/**
 * @file test_tool.c
 * @brief Unit tests for tool management module
 */

#include "yuki_frame/tool.h"
#include "yuki_frame/framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Global state (required by framework modules)
FrameworkConfig g_config;
bool g_running = true;

static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

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
#define ASSERT_NULL(ptr) ASSERT((ptr) == NULL)
#define ASSERT_NOT_NULL(ptr) ASSERT((ptr) != NULL)

// Tests
TEST(tool_registry_init_success) {
    int result = tool_registry_init();
    ASSERT_EQ(result, FW_OK);
    tool_registry_shutdown();
}

TEST(tool_register_valid_tool) {
    tool_registry_init();
    
    int result = tool_register("test_tool", "echo test");
    ASSERT_EQ(result, FW_OK);
    
    tool_registry_shutdown();
}

TEST(tool_register_null_name_fails) {
    tool_registry_init();
    
    int result = tool_register(NULL, "echo test");
    ASSERT_EQ(result, FW_ERROR_INVALID_ARG);
    
    tool_registry_shutdown();
}

TEST(tool_register_null_command_fails) {
    tool_registry_init();
    
    int result = tool_register("test_tool", NULL);
    ASSERT_EQ(result, FW_ERROR_INVALID_ARG);
    
    tool_registry_shutdown();
}

TEST(tool_register_duplicate_name_fails) {
    tool_registry_init();
    
    tool_register("test_tool", "echo test1");
    int result = tool_register("test_tool", "echo test2");
    
    ASSERT_EQ(result, FW_ERROR_ALREADY_EXISTS);
    
    tool_registry_shutdown();
}

TEST(tool_find_existing_tool) {
    tool_registry_init();
    
    tool_register("test_tool", "echo test");
    Tool* tool = tool_find("test_tool");
    
    ASSERT_NOT_NULL(tool);
    ASSERT_STR_EQ(tool->name, "test_tool");
    
    tool_registry_shutdown();
}

TEST(tool_find_nonexistent_tool_returns_null) {
    tool_registry_init();
    
    Tool* tool = tool_find("nonexistent_tool");
    ASSERT_NULL(tool);
    
    tool_registry_shutdown();
}

TEST(tool_unregister_existing_tool) {
    tool_registry_init();
    
    tool_register("test_tool", "echo test");
    int result = tool_unregister("test_tool");
    
    ASSERT_EQ(result, FW_OK);
    
    Tool* tool = tool_find("test_tool");
    ASSERT_NULL(tool);
    
    tool_registry_shutdown();
}

TEST(tool_unregister_nonexistent_tool_fails) {
    tool_registry_init();
    
    int result = tool_unregister("nonexistent_tool");
    ASSERT_EQ(result, FW_ERROR_NOT_FOUND);
    
    tool_registry_shutdown();
}

TEST(tool_subscribe_valid_event) {
    tool_registry_init();
    
    tool_register("test_tool", "echo test");
    int result = tool_subscribe("test_tool", "test.event");
    
    ASSERT_EQ(result, FW_OK);
    
    tool_registry_shutdown();
}

TEST(tool_subscribe_nonexistent_tool_fails) {
    tool_registry_init();
    
    int result = tool_subscribe("nonexistent_tool", "test.event");
    ASSERT_EQ(result, FW_ERROR_NOT_FOUND);
    
    tool_registry_shutdown();
}

TEST(tool_is_running_stopped_tool) {
    tool_registry_init();
    
    tool_register("test_tool", "echo test");
    bool running = tool_is_running("test_tool");
    
    ASSERT(running == false);
    
    tool_registry_shutdown();
}

TEST(tool_is_running_nonexistent_tool) {
    tool_registry_init();
    
    bool running = tool_is_running("nonexistent_tool");
    ASSERT(running == false);
    
    tool_registry_shutdown();
}

// Test runner
int main(void) {
    printf("\n=== Tool Module Unit Tests ===\n\n");
    
    run_test_tool_registry_init_success();
    run_test_tool_register_valid_tool();
    run_test_tool_register_null_name_fails();
    run_test_tool_register_null_command_fails();
    run_test_tool_register_duplicate_name_fails();
    run_test_tool_find_existing_tool();
    run_test_tool_find_nonexistent_tool_returns_null();
    run_test_tool_unregister_existing_tool();
    run_test_tool_unregister_nonexistent_tool_fails();
    run_test_tool_subscribe_valid_event();
    run_test_tool_subscribe_nonexistent_tool_fails();
    run_test_tool_is_running_stopped_tool();
    run_test_tool_is_running_nonexistent_tool();
    
    printf("\n=== Test Summary ===\n");
    printf("  Total:  %d\n", tests_run);
    printf("  Passed: %d\n", tests_passed);
    printf("  Failed: %d\n", tests_failed);
    printf("\n");
    
    return tests_failed == 0 ? 0 : 1;
}
