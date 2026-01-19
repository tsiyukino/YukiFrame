# Testing and Debugging Guide

## Overview

Yuki-Frame includes a comprehensive test suite and debugging infrastructure to ensure code quality and facilitate development.

## Test Suite Structure

```
tests/
├── unit/                      # Unit tests
│   ├── test_event.c          # Event bus tests
│   ├── test_config.c         # Configuration tests
│   ├── test_tool.c           # Tool management tests
│   └── CMakeLists.txt        # Build configuration
├── integration/               # Integration tests
│   ├── test_integration.py   # Full workflow tests
│   └── CMakeLists.txt        # Build configuration
└── CMakeLists.txt            # Main test configuration
```

## Running Tests

### Quick Start

**Windows:**
```batch
# Build and run all tests
run-tests.bat

# Or manually
cd build
ctest --output-on-failure
```

**Linux/macOS:**
```bash
# Build and run all tests
./run-tests.sh

# Or manually
cd build
ctest --output-on-failure
```

### Running Specific Tests

```bash
# Run only unit tests
cd build
ctest -R ".*_tests" --output-on-failure

# Run specific test
cd build/tests/unit
./test_event
./test_config
./test_tool

# Run integration tests
cd tests/integration
python3 test_integration.py
```

### Verbose Output

```bash
# Maximum verbosity
cd build
ctest -VV

# Show test output even on success
ctest --verbose
```

## Unit Tests

### Event Module Tests

Tests for event bus functionality:
- Event bus initialization
- Event publishing (valid/invalid)
- Event parsing (valid/invalid formats)
- Event formatting
- Null pointer handling
- Buffer overflow protection

**Run:**
```bash
cd build/tests/unit
./test_event
```

### Config Module Tests

Tests for configuration management:
- Config file loading
- Value retrieval (string, int, bool)
- Default values
- Tool configuration parsing
- Missing file handling

**Run:**
```bash
cd build/tests/unit
./test_config
```

### Tool Module Tests

Tests for tool lifecycle management:
- Tool registry operations
- Tool registration/unregistration
- Duplicate name handling
- Tool lookup
- Event subscription
- Status checking

**Run:**
```bash
cd build/tests/unit
./test_tool
```

## Integration Tests

End-to-end workflow tests:
- Framework version check
- Help system
- Configuration loading
- Invalid config handling
- (More tests can be added)

**Run:**
```bash
cd tests/integration
python3 test_integration.py
```

## Writing New Tests

### Unit Test Template

```c
#include "yuki_frame/your_module.h"
#include <stdio.h>
#include <assert.h>

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
            printf("FAIL\n"); \
            tests_failed++; \
            tests_passed--; \
            return; \
        } \
    } while(0)

TEST(your_test_name) {
    // Your test code
    ASSERT(1 + 1 == 2);
}

int main(void) {
    printf("\n=== Your Module Tests ===\n\n");
    
    run_test_your_test_name();
    
    printf("\n=== Test Summary ===\n");
    printf("  Total:  %d\n", tests_run);
    printf("  Passed: %d\n", tests_passed);
    printf("  Failed: %d\n", tests_failed);
    
    return tests_failed == 0 ? 0 : 1;
}
```

### Adding Test to CMakeLists.txt

```cmake
# In tests/unit/CMakeLists.txt
add_executable(test_your_module test_your_module.c ${FRAMEWORK_LIB_SOURCES})
target_include_directories(test_your_module PRIVATE ${CMAKE_SOURCE_DIR}/include)
if(WIN32)
    target_link_libraries(test_your_module PRIVATE ws2_32)
else()
    target_link_libraries(test_your_module PRIVATE pthread rt)
endif()
add_test(NAME your_module_tests COMMAND test_your_module)
```

## Debugging

### Debug Build

```bash
# Linux/macOS
mkdir build-debug && cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .

# Windows
mkdir build-debug && cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --config Debug
```

### Debug Flags

The debug build includes:
- `-g` - Debug symbols
- `-O0` - No optimization
- `-DDEBUG` - Debug macro defined

### Using GDB (Linux/macOS)

```bash
# Start with debugger
cd build-debug
gdb ./yuki-frame

# In GDB:
(gdb) break main
(gdb) run -c ../yuki-frame.conf.example
(gdb) next          # Step over
(gdb) step          # Step into
(gdb) print var     # Print variable
(gdb) backtrace     # Show call stack
(gdb) continue      # Continue execution
```

### Using LLDB (macOS)

```bash
# Start with debugger
cd build-debug
lldb ./yuki-frame

# In LLDB:
(lldb) breakpoint set --name main
(lldb) run -c ../yuki-frame.conf.example
(lldb) thread step-over
(lldb) thread step-in
(lldb) print var
(lldb) bt          # Backtrace
(lldb) continue
```

### Using Visual Studio Debugger (Windows)

1. Open project in Visual Studio
2. Set yuki-frame as startup project
3. Set command arguments in project properties
4. Press F5 to start debugging
5. Use F10 (step over), F11 (step into)

### Debug Logging

Enable debug logging in config:

```ini
[framework]
log_level = DEBUG
enable_debug = true
```

Or via command line:
```bash
./yuki-frame -c config.conf --debug
```

### Memory Debugging

**Valgrind (Linux):**
```bash
valgrind --leak-check=full --show-leak-kinds=all ./yuki-frame -c config.conf
```

**AddressSanitizer:**
```bash
# Build with ASan
cmake -DCMAKE_C_FLAGS="-fsanitize=address -g" ..
cmake --build .

# Run
./yuki-frame -c config.conf
```

### Debug API

Use the built-in debug functions:

```c
#include "yuki_frame/debug.h"

// Log debug event
debug_log(DEBUG_TOOL_START, "my_tool", "Tool starting with config: %s", config);

// Dump current state
debug_dump_state();

// Get recent debug events
DebugEvent events[100];
int count = debug_get_events(events, 100);
```

## Performance Profiling

### gprof (Linux)

```bash
# Build with profiling
cmake -DCMAKE_C_FLAGS="-pg" ..
cmake --build .

# Run program
./yuki-frame -c config.conf

# Generate profile
gprof yuki-frame gmon.out > analysis.txt
less analysis.txt
```

### perf (Linux)

```bash
# Record
perf record ./yuki-frame -c config.conf

# Report
perf report

# Annotate
perf annotate
```

### Instruments (macOS)

1. Open Instruments.app
2. Choose Time Profiler
3. Select yuki-frame executable
4. Click Record

## Continuous Integration

### Test in CI Pipeline

```yaml
# Example .github/workflows/test.yml
name: Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Build
        run: |
          mkdir build && cd build
          cmake ..
          cmake --build .
      - name: Test
        run: |
          cd build
          ctest --output-on-failure
```

## Test Coverage

### Generate Coverage Report (Linux)

```bash
# Build with coverage
mkdir build-coverage && cd build-coverage
cmake -DCMAKE_C_FLAGS="--coverage" ..
cmake --build .

# Run tests
ctest

# Generate report
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage-html

# View report
firefox coverage-html/index.html
```

## Common Issues

### Test Fails to Link

**Problem:** Undefined references during test compilation

**Solution:** Ensure all required source files are in `FRAMEWORK_LIB_SOURCES` in `tests/unit/CMakeLists.txt`

### Test Crashes

**Problem:** Segmentation fault or crash during test

**Solution:**
1. Run with debugger: `gdb ./test_module`
2. Check for NULL pointers
3. Verify initialization order
4. Enable AddressSanitizer

### Integration Test Can't Find Executable

**Problem:** `test_integration.py` reports executable not found

**Solution:**
1. Ensure project is built: `./build.sh` or `build.bat`
2. Check executable location matches script search paths
3. Run from correct directory

### Tests Pass Locally But Fail in CI

**Problem:** Tests work on local machine but fail in CI

**Solution:**
1. Check file paths (absolute vs relative)
2. Verify test doesn't depend on local config
3. Ensure all dependencies are installed
4. Check timing issues (add timeouts)

## Best Practices

### Unit Testing

1. **Test one thing** - Each test should verify one behavior
2. **Use descriptive names** - `test_config_load_invalid_file_fails`
3. **Test edge cases** - NULL pointers, empty strings, large values
4. **Keep tests fast** - Unit tests should run in milliseconds
5. **No external dependencies** - Unit tests shouldn't need network, files, etc.

### Integration Testing

1. **Test realistic workflows** - Simulate actual usage
2. **Clean up** - Remove temporary files, kill processes
3. **Handle timeouts** - Don't let tests hang forever
4. **Check both success and failure paths**
5. **Use fixtures** - Prepare test environment before running

### Debugging

1. **Reproduce first** - Ensure you can reliably trigger the bug
2. **Minimize test case** - Remove unrelated code
3. **Use debug build** - Always debug with `-g -O0`
4. **Check assumptions** - Verify your understanding with assertions
5. **Read error messages** - They often tell you exactly what's wrong

## Resources

- **CTest Documentation:** https://cmake.org/cmake/help/latest/manual/ctest.1.html
- **GDB Tutorial:** https://www.gnu.org/software/gdb/documentation/
- **Valgrind Manual:** https://valgrind.org/docs/manual/manual.html
- **AddressSanitizer:** https://github.com/google/sanitizers

## Getting Help

If tests fail or you need debugging help:

1. Check this guide for common solutions
2. Run with verbose output: `ctest -VV`
3. Enable debug logging: `--debug` flag
4. Check logs in `/var/log/yuki-frame/` or configured log location
5. Review DEVELOPMENT.md for project structure
6. Open an issue with test output and system details
