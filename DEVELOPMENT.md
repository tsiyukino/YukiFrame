# Yuki-Frame Development Guide

## Development Setup

### Prerequisites

**Linux/Unix:**
```bash
sudo apt-get install build-essential cmake git
```

**macOS:**
```bash
xcode-select --install
brew install cmake
```

**Windows:**
- Visual Studio 2019 or later (with C++ tools)
- CMake 3.10+
- Git

### Clone and Build

```bash
git clone https://github.com/your-org/yuki-frame.git
cd yuki-frame

# Create build directory
mkdir build && cd build

# Configure (Debug build for development)
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Build
cmake --build .

# You should now have:
#   - yuki-frame (main framework)
#   - yuki-control (control utility)
```

## Project Structure

```
yuki-frame/
├── include/yuki_frame/    # Public headers
│   ├── framework.h         # Core framework types
│   ├── tool.h             # Tool management
│   ├── event.h            # Event bus
│   ├── logger.h           # Logging
│   ├── config.h           # Configuration
│   ├── control.h          # Remote control
│   ├── debug.h            # Debug/diagnostics
│   └── platform.h         # Platform abstraction
├── src/
│   ├── core/              # Core implementation
│   │   ├── main.c           # Framework entry point
│   │   ├── tool.c           # Tool lifecycle management
│   │   ├── event.c          # Event bus
│   │   ├── logger.c         # Logging system
│   │   ├── config.c         # Config parser
│   │   ├── control.c        # Control API
│   │   ├── debug.c          # Debug system
│   │   └── cli_control.c    # Control utility (NEW!)
│   └── platform/          # Platform-specific code
│       ├── platform_linux.c   # Linux/Unix implementation
│       └── platform_windows.c # Windows implementation
├── tests/
│   ├── unit/              # Unit tests
│   └── integration/       # Integration tests
├── examples/              # Usage examples
├── tools/                 # Example tool implementations
└── docs/                  # Additional documentation
```

## Coding Standards

### Naming Conventions

**Files:**
- Headers: `snake_case.h` (e.g., `event_bus.h`)
- Sources: `snake_case.c` (e.g., `event_bus.c`)
- Platform: `platform_name.c` (e.g., `platform_linux.c`)

**Code:**
```c
// Types: PascalCase
typedef struct {
    int value;
} EventData;

// Functions: snake_case with module prefix
int event_bus_publish(const Event* event);
int config_load(const char* path);

// Variables: snake_case
int retry_count = 0;
Event* current_event = NULL;

// Constants/Macros: SCREAMING_SNAKE_CASE
#define MAX_BUFFER_SIZE 1024
#define DEFAULT_TIMEOUT_MS 5000

// Globals: prefix with 'g_'
extern FrameworkConfig g_config;
extern bool g_running;
```

### Code Style

**K&R Style (Mandatory):**
```c
int function_name(int arg1, const char* arg2) {
    if (condition) {
        // code
    } else {
        // code
    }
    
    for (int i = 0; i < count; i++) {
        // code
    }
}
```

**Rules:**
- 4 spaces for indentation (NO TABS)
- Opening brace on same line (except function definitions)
- Space after keywords: `if (`, `for (`, `while (`
- Space around operators: `x = y + z`
- No trailing whitespace
- Unix line endings (LF)

### Header Guards

```c
#ifndef YUKI_FRAME_MODULE_H
#define YUKI_FRAME_MODULE_H

// Header content

#endif  // YUKI_FRAME_MODULE_H
```

Pattern: `YUKI_FRAME_PATH_UPPERCASE_H`

### Error Handling

```c
// Return 0 on success, negative error codes on failure
int function_that_can_fail(void) {
    if (error_condition) {
        return FW_ERROR_INVALID_ARG;
    }
    return FW_OK;
}

// Usage
int result = function_that_can_fail();
if (result != FW_OK) {
    LOG_ERROR("core", "Function failed: %d", result);
    return result;
}
```

### Documentation

```c
/**
 * Brief function description
 *
 * Detailed explanation explaining why, not what.
 * The code itself should explain what.
 *
 * @param config Configuration pointer (cannot be NULL)
 * @param timeout_ms Timeout in milliseconds (0 = no timeout)
 * @return 0 on success, negative error code on failure
 */
int init_system(const FrameworkConfig* config, uint32_t timeout_ms);
```

## Building and Testing

### Build Types

```bash
# Debug build (with symbols, no optimization)
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release build (optimized, no debug symbols)
cmake -DCMAKE_BUILD_TYPE=Release ..

# RelWithDebInfo build (optimized + debug symbols)
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
```

### Running Tests

```bash
# Build and run all tests
cmake --build . --target test

# Run specific test
./tests/unit/test_event_bus

# Run with verbose output
ctest --verbose

# Run with test output on failure
ctest --output-on-failure
```

See `TESTING.md` for comprehensive testing guide.

### Memory Checking

```bash
# Linux: Valgrind
valgrind --leak-check=full ./yuki-frame config.conf

# AddressSanitizer (recommended)
cmake -DCMAKE_C_FLAGS="-fsanitize=address -g" ..
cmake --build .
./yuki-frame config.conf
```

## Core Components

### 1. Process Spawning (Platform Layer)

**Linux** (`platform_linux.c`):
```c
ProcessHandle platform_spawn_process(const char* command,
                                     int* stdin_fd,
                                     int* stdout_fd,
                                     int* stderr_fd) {
    // Uses fork() + execl() to spawn process
    // Creates pipes for stdin/stdout/stderr
    // Returns PID as handle
}
```

**Windows** (`platform_windows.c`):
```c
ProcessHandle platform_spawn_process(const char* command,
                                     int* stdin_fd,
                                     int* stdout_fd,
                                     int* stderr_fd) {
    // Uses CreateProcess() to spawn process
    // Creates pipes with CreatePipe()
    // Returns HANDLE to process
}
```

**Key points:**
- Must create bidirectional pipes (stdin/stdout/stderr)
- Must set non-blocking I/O on parent-side file descriptors
- Must properly close unused pipe ends
- Must handle errors gracefully

### 2. Tool Management (`tool.c`)

```c
int tool_start(const char* name) {
    // 1. Find tool in registry
    // 2. Spawn process using platform_spawn_process()
    // 3. Set non-blocking I/O on pipes
    // 4. Update tool status and PID
}

int tool_stop(const char* name) {
    // 1. Find tool in registry
    // 2. Send SIGTERM (or TerminateProcess on Windows)
    // 3. Wait for process to exit
    // 4. Close pipe file descriptors
    // 5. Update tool status
}

void tool_check_health(void) {
    // 1. Check if each tool's process is still running
    // 2. If crashed and restart_on_crash=yes, restart it
    // 3. Track restart count vs max_restarts
}
```

**Important:**
- Always check if tool is already running before starting
- Always close file descriptors when stopping tools
- Handle restart logic with backoff to prevent tight loops

### 3. Control Interface (`control.c`, `cli_control.c`)

**Control API** (in-process):
```c
int control_start_tool(const char* tool_name) {
    return tool_start(tool_name);
}

int control_list_tools(char* buffer, size_t buffer_size) {
    // Iterate through tool registry
    // Format tool status into buffer
}
```

**CLI Control** (separate executable):
```c
// src/core/cli_control.c
int main(int argc, char* argv[]) {
    // 1. Load same config as framework
    // 2. Initialize tool registry
    // 3. Parse command (start/stop/list/status)
    // 4. Call control API functions
    // 5. Display results
}
```

**Note:** CLI control initializes its own registry from config but doesn't start a full framework. It only uses the control API to manage tools.

### 4. Event System (`event.c`)

```c
int event_publish(const char* type, const char* sender, const char* data) {
    // 1. Create event structure
    // 2. Add to event queue
    // 3. Will be processed in main loop
}

void event_process_queue(void) {
    // 1. Dequeue events
    // 2. Check tool subscriptions
    // 3. Send to subscribed tools via stdin
}
```

**Event flow:**
1. Tool writes to stdout: `"EVENT_TYPE|sender|data\n"`
2. Framework reads from tool's stdout pipe
3. Framework parses event
4. Framework checks which tools subscribed to `EVENT_TYPE`
5. Framework writes event to subscribed tools' stdin pipes

## Debugging

### Quick Reference

```bash
# Build debug version
mkdir build-debug && cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .

# Run with GDB
gdb ./yuki-frame
(gdb) run -c ../yuki-frame.conf.example

# Common GDB commands
(gdb) break tool_start        # Set breakpoint
(gdb) next                    # Step over (n)
(gdb) step                    # Step into (s)
(gdb) continue                # Continue (c)
(gdb) print tool->pid         # Print value (p)
(gdb) backtrace              # Show stack (bt)
(gdb) info locals            # Show local variables
```

### Debug Logging

```bash
# In config file
[core]
log_level = DEBUG
enable_debug = yes

# Or command line
./yuki-frame -c config.conf --debug
```

### Testing Process Spawning

```bash
# Test tool starts correctly
./yuki-frame -c config.conf -d
# Check logs for "Tool X started with PID Y"

# Test manual control
./yuki-control start my_tool
# Should show "Tool 'my_tool' started successfully"

# Verify process is running
ps aux | grep my_tool    # Linux
tasklist | findstr python  # Windows
```

### Common Issues

**Tool won't spawn:**
- Check command path is correct (use absolute paths)
- Test command manually: `python tools/my_tool.py`
- Check logs for error messages from `platform_spawn_process()`

**Pipes not working:**
- Verify non-blocking I/O is set on parent-side descriptors
- Check both ends of pipes are closed properly
- On Windows, ensure handles are converted to file descriptors correctly

**Tool crashes immediately:**
- Test tool standalone first
- Check Python/interpreter is in PATH
- Look for stderr output in framework logs

See `TESTING.md` for complete debugging guide.

## Contributing

### Workflow

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/my-feature`
3. Make changes following coding standards
4. Run tests: `cmake --build . --target test`
5. Commit: `git commit -m "Add feature X"`
6. Push: `git push origin feature/my-feature`
7. Open Pull Request

### Commit Messages

```
type(scope): Short description

Longer explanation if needed. Explain why, not what.

Fixes #123
```

**Types:** feat, fix, docs, style, refactor, test, chore

**Examples:**
```
feat(event): Add event filtering support
fix(tool): Fix memory leak in tool_stop()
docs(readme): Update installation instructions
test(config): Add unit tests for config parser
feat(control): Add yuki-control CLI utility
```

### Pull Request Guidelines

- **Size:** 200-400 lines is ideal
- **Scope:** One feature or fix per PR
- **Tests:** Include tests for new features
- **Documentation:** Update docs for API changes
- **Review:** Address review comments promptly
- **CI:** Ensure all tests pass

## Release Process

### Versioning (Semantic Versioning)

```
MAJOR.MINOR.PATCH

MAJOR: Breaking changes (incompatible API changes)
MINOR: New features (backward compatible)
PATCH: Bug fixes (backward compatible)
```

**Examples:**
- `2.0.0` → `2.0.1`: Bug fix (PATCH bump)
- `2.0.1` → `2.1.0`: New feature (MINOR bump)
- `2.1.0` → `3.0.0`: Breaking change (MAJOR bump)

### Release Checklist

1. **Update version** in `CMakeLists.txt`
2. **Update version** in `include/yuki_frame/framework.h`
3. **Update** `CHANGELOG.md`
4. **Run full test suite**
5. **Build release packages**
6. **Test on all platforms** (Linux, Windows, macOS)
7. **Create git tag**: `git tag -a v2.0.0 -m "Release 2.0.0"`
8. **Push tag**: `git push origin v2.0.0`
9. **Create GitHub release** with binaries
10. **Update documentation**

## Architecture

### Component Overview

**Framework Core:**
- `main.c`: Entry point, main event loop
- `tool.c`: Tool lifecycle management (spawn/stop/restart/health)
- `event.c`: Event bus implementation
- `logger.c`: Logging subsystem
- `config.c`: Configuration parser
- `control.c`: Control API (start/stop/status)
- `debug.c`: Debug/diagnostics
- `cli_control.c`: CLI control utility

**Platform Layer:**
- `platform_linux.c`: POSIX/Linux implementation (fork/exec, pipes)
- `platform_windows.c`: Windows API implementation (CreateProcess, pipes)

### Threading Model

Yuki-Frame uses a single-threaded event-driven architecture:
- Main thread handles all events and I/O
- Tools run as separate processes
- No mutexes or locks needed
- Simple and predictable behavior

### Event Flow

```
Tool stdout → Framework reads → Parse event → 
→ Check subscriptions → Send to subscribers → 
→ Subscribers' stdin
```

### Process Management Flow

```
tool_start() → platform_spawn_process() → fork/exec (Linux) or CreateProcess (Windows)
                                        → Create pipes (stdin/stdout/stderr)
                                        → Set non-blocking I/O
                                        → Return process handle + file descriptors
```

## Performance

### Profiling

```bash
# gprof (Linux)
cmake -DCMAKE_C_FLAGS="-pg" ..
cmake --build .
./yuki-frame config.conf
gprof yuki-frame gmon.out > analysis.txt

# perf (Linux)
perf record ./yuki-frame config.conf
perf report
```

### Optimization Tips

- Use `-O3` for release builds
- Profile before optimizing
- Focus on hot paths (event processing, I/O)
- Minimize allocations in event loop
- Use efficient data structures (ring buffers)
- Keep pipe reads/writes non-blocking

## Troubleshooting

### Common Issues

**Build fails on Windows:**
- Ensure Visual Studio C++ tools are installed
- Use "x64 Native Tools Command Prompt"
- Check CMake version (3.10+)

**Build fails on Linux:**
- Install build-essential: `sudo apt install build-essential cmake`
- Check GCC version: `gcc --version` (7.0+ recommended)

**Tests fail:**
- Check port conflicts (control port 9999)
- Ensure Python 3.x is installed (for integration tests)
- Run with verbose: `ctest -VV`

**Tool doesn't start:**
- Check tool path in config (use absolute paths)
- Verify tool has execute permissions: `chmod +x tool`
- Check logs: `tail -f logs/yuki-frame.log`
- Test tool manually: `python tools/my_tool.py`

**Memory leaks:**
- Run with Valgrind or AddressSanitizer
- Check all `malloc()` have corresponding `free()`
- Review tool cleanup in `tool_stop()` and `tool_registry_shutdown()`
- Close all pipe file descriptors properly

**Segmentation fault:**
- Build with debug symbols: `CMAKE_BUILD_TYPE=Debug`
- Run with gdb: `gdb ./yuki-frame`
- Check for NULL pointer dereferences
- Verify buffer sizes
- Check pipe descriptor validity before use

**yuki-control can't find tools:**
- Ensure config file is in current directory
- Or use: `yuki-control -c /path/to/config.conf start tool`
- Check tool is registered in config

## Project Guidelines

### Adding New Features

1. **Design**: Discuss in GitHub issue first
2. **Implement**: Follow coding standards
3. **Test**: Write unit and integration tests
4. **Document**: Update relevant documentation
5. **Review**: Submit PR for review

### Code Review Checklist

- [ ] Follows coding standards (K&R, naming)
- [ ] No compiler warnings
- [ ] Memory leaks checked (Valgrind)
- [ ] Tests added/updated
- [ ] Documentation updated
- [ ] CHANGELOG.md updated
- [ ] Builds on Linux and Windows
- [ ] All tests pass
- [ ] Platform-specific code properly abstracted

### Documentation Updates

When updating docs, check:
- [ ] README.md (if user-facing change)
- [ ] DEVELOPMENT.md (if dev-facing change)
- [ ] TOOL_DEVELOPMENT.md (if tool API changed)
- [ ] CHANGELOG.md (always)
- [ ] Code comments (always)

## Development Tools

### Recommended IDE Setup

**VS Code:**
```json
{
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
    "cmake.configureOnOpen": true,
    "files.insertFinalNewline": true,
    "files.trimTrailingWhitespace": true,
    "editor.tabSize": 4,
    "editor.insertSpaces": true
}
```

**CLion:**
- Open CMakeLists.txt as project
- Set code style to "K&R"
- Enable Valgrind memcheck

**Vim/Neovim:**
```vim
set tabstop=4
set shiftwidth=4
set expandtab
set cindent
```

### Useful Scripts

```bash
# Format all C files (requires clang-format)
find src include -name '*.c' -o -name '*.h' | xargs clang-format -i

# Check for memory leaks in all tests
for test in build/tests/unit/test_*; do
    valgrind --leak-check=full $test
done

# Build and test in one command
cmake --build build && cd build && ctest
```

## Resources

- **Issue Tracker:** https://github.com/your-org/yuki-frame/issues
- **Discussions:** https://github.com/your-org/yuki-frame/discussions
- **Documentation:** See markdown files in repo
- **Examples:** `examples/` and `tools/` directories

## Contact

- **Maintainer:** maintainer@example.com
- **Chat:** #yuki-frame on Discord
- **Mailing List:** yuki-frame@lists.example.com

## License

MIT License - See `LICENSE` file
