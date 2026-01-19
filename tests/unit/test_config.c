/**
 * @file test_config.c
 * @brief Unit tests for configuration module
 */

#include "yuki_frame/config.h"
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

// Test helper - create temporary config file
static void create_test_config(const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) return;
    
    fprintf(f, "[framework]\n");
    fprintf(f, "log_file = /tmp/test.log\n");
    fprintf(f, "log_level = INFO\n");
    fprintf(f, "max_tools = 50\n");
    fprintf(f, "\n");
    fprintf(f, "[tool.test_tool]\n");
    fprintf(f, "command = /bin/echo test\n");
    fprintf(f, "description = Test tool\n");
    fprintf(f, "autostart = true\n");
    fprintf(f, "restart_on_crash = false\n");
    fprintf(f, "max_restarts = 3\n");
    fprintf(f, "subscriptions = test.event,system.start\n");
    
    fclose(f);
}

// Tests
TEST(config_load_nonexistent_file_fails) {
    int result = config_load("nonexistent_config_file.conf");
    ASSERT_NE(result, FW_OK);
}

TEST(config_load_valid_file_succeeds) {
    create_test_config("test_config.tmp");
    
    int result = config_load("test_config.tmp");
    ASSERT_EQ(result, FW_OK);
    
    remove("test_config.tmp");
}

TEST(config_get_existing_value) {
    create_test_config("test_config.tmp");
    config_load("test_config.tmp");
    
    const char* value = config_get("framework", "log_level");
    ASSERT_NOT_NULL(value);
    ASSERT_STR_EQ(value, "INFO");
    
    remove("test_config.tmp");
}

TEST(config_get_nonexistent_value_returns_null) {
    create_test_config("test_config.tmp");
    config_load("test_config.tmp");
    
    const char* value = config_get("framework", "nonexistent_key");
    ASSERT_NULL(value);
    
    remove("test_config.tmp");
}

TEST(config_get_int_valid_value) {
    create_test_config("test_config.tmp");
    config_load("test_config.tmp");
    
    int value = config_get_int("framework", "max_tools", 100);
    ASSERT_EQ(value, 50);
    
    remove("test_config.tmp");
}

TEST(config_get_int_default_value) {
    create_test_config("test_config.tmp");
    config_load("test_config.tmp");
    
    int value = config_get_int("framework", "nonexistent", 999);
    ASSERT_EQ(value, 999);
    
    remove("test_config.tmp");
}

TEST(config_get_bool_true_value) {
    create_test_config("test_config.tmp");
    config_load("test_config.tmp");
    
    bool value = config_get_bool("tool.test_tool", "autostart", false);
    ASSERT(value == true);
    
    remove("test_config.tmp");
}

TEST(config_get_bool_false_value) {
    create_test_config("test_config.tmp");
    config_load("test_config.tmp");
    
    bool value = config_get_bool("tool.test_tool", "restart_on_crash", true);
    ASSERT(value == false);
    
    remove("test_config.tmp");
}

TEST(config_get_bool_default_value) {
    create_test_config("test_config.tmp");
    config_load("test_config.tmp");
    
    bool value = config_get_bool("framework", "nonexistent", true);
    ASSERT(value == true);
    
    remove("test_config.tmp");
}

TEST(config_get_tools_returns_tools) {
    create_test_config("test_config.tmp");
    config_load("test_config.tmp");
    
    ToolConfig* tools = NULL;
    int count = 0;
    
    int result = config_get_tools(&tools, &count);
    ASSERT_EQ(result, FW_OK);
    ASSERT_NOT_NULL(tools);
    ASSERT(count > 0);
    
    if (tools) {
        config_free_tools(tools, count);
    }
    
    remove("test_config.tmp");
}

// Test runner
int main(void) {
    printf("\n=== Config Module Unit Tests ===\n\n");
    
    run_test_config_load_nonexistent_file_fails();
    run_test_config_load_valid_file_succeeds();
    run_test_config_get_existing_value();
    run_test_config_get_nonexistent_value_returns_null();
    run_test_config_get_int_valid_value();
    run_test_config_get_int_default_value();
    run_test_config_get_bool_true_value();
    run_test_config_get_bool_false_value();
    run_test_config_get_bool_default_value();
    run_test_config_get_tools_returns_tools();
    
    printf("\n=== Test Summary ===\n");
    printf("  Total:  %d\n", tests_run);
    printf("  Passed: %d\n", tests_passed);
    printf("  Failed: %d\n", tests_failed);
    printf("\n");
    
    return tests_failed == 0 ? 0 : 1;
}
