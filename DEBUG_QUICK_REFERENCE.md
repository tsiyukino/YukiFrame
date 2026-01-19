# Yuki-Frame Debug Quick Reference

## Quick Test Commands

```bash
# Build and run all tests
./run-tests.sh          # Linux/macOS
run-tests.bat           # Windows

# Run specific test
cd build/tests/unit
./test_event
./test_config
./test_tool

# Integration tests
cd tests/integration
python3 test_integration.py

# Verbose test output
cd build
ctest -VV
```

## Debug Build

```bash
# Create debug build
mkdir build-debug && cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .

# Run with debug symbols
./yuki-frame -c ../yuki-frame.conf.example --debug
```

## GDB Cheat Sheet

```bash
# Start debugging
gdb ./yuki-frame

# Common commands
(gdb) break main              # Set breakpoint
(gdb) run -c config.conf      # Run with args
(gdb) next                    # Step over (n)
(gdb) step                    # Step into (s)
(gdb) continue                # Continue (c)
(gdb) print variable          # Print value (p)
(gdb) backtrace              # Show stack (bt)
(gdb) info locals            # Show local vars
(gdb) list                   # Show code
(gdb) quit                   # Exit (q)
```

## Memory Debugging

```bash
# Valgrind
valgrind --leak-check=full ./yuki-frame -c config.conf

# AddressSanitizer
cmake -DCMAKE_C_FLAGS="-fsanitize=address" ..
cmake --build .
./yuki-frame -c config.conf
```

## Logging Levels

```ini
[framework]
log_level = TRACE    # Most verbose
log_level = DEBUG    # Debug info
log_level = INFO     # Normal
log_level = WARN     # Warnings only
log_level = ERROR    # Errors only
log_level = FATAL    # Critical only
```

## Debug API

```c
// In your code
#include "yuki_frame/debug.h"

// Log debug events
debug_log(DEBUG_TOOL_START, "tool_name", "Starting tool: %s", name);
debug_log(DEBUG_ERROR, NULL, "Error: %s", error_msg);

// Dump state
debug_dump_state();

// Get events
DebugEvent events[100];
int count = debug_get_events(events, 100);
```

## Common Issues

| Problem | Solution |
|---------|----------|
| Segfault | Run with gdb, check NULL pointers |
| Memory leak | Run valgrind, check malloc/free pairs |
| Test fails | Run with `ctest -VV` for details |
| Can't find config | Use `-c` flag: `./yuki-frame -c config.conf` |
| Build error | Check TESTING.md, clean and rebuild |

## Log File Locations

| Platform | Default Location |
|----------|-----------------|
| Linux | `/var/log/yuki-frame/yuki-frame.log` |
| macOS | `/var/log/yuki-frame/yuki-frame.log` |
| Windows | `C:\ProgramData\yuki-frame\logs\yuki-frame.log` |

## Performance Profiling

```bash
# Build with profiling
cmake -DCMAKE_C_FLAGS="-pg" ..

# Run and generate profile
./yuki-frame -c config.conf
gprof yuki-frame gmon.out > profile.txt
```

## Test Coverage

```bash
# Build with coverage
cmake -DCMAKE_C_FLAGS="--coverage" ..
cmake --build .

# Run tests
ctest

# Generate HTML report
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage-html
```

## Environment Variables

```bash
# Enable debug mode
export YUKI_FRAME_DEBUG=1

# Set custom log file
export YUKI_FRAME_LOG=/tmp/my-debug.log

# Set log level
export YUKI_FRAME_LOG_LEVEL=DEBUG
```

## Useful Flags

```bash
# Show version
./yuki-frame --version

# Show help
./yuki-frame --help

# Specify config
./yuki-frame --config /path/to/config.conf

# Enable debug mode
./yuki-frame --debug -c config.conf
```

## Quick Diagnostics

```bash
# Check if executable exists
ls -l build/yuki-frame*

# Check if config is valid
./yuki-frame -c config.conf --version

# Test with minimal config
echo "[framework]
log_level = DEBUG" > test.conf
./yuki-frame -c test.conf

# Check log output
tail -f /var/log/yuki-frame/yuki-frame.log
```

## See Full Documentation

- **TESTING.md** - Complete testing and debugging guide
- **DEVELOPMENT.md** - Development setup and workflow
- **README.md** - User guide and quick start
