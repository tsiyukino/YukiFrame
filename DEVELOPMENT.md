# Yuki-Frame Development Guide

## Development Setup

### Prerequisites

**Linux/Unix:**
```bash
sudo apt-get install build-essential cmake git
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

# Configure (Debug build)
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Build
cmake --build .

# Run
./yuki-frame ../yuki-frame.conf.example
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
│   │   ├── main.c
│   │   ├── tool.c
│   │   ├── event.c
│   │   ├── logger.c
│   │   ├── config.c
│   │   ├── control.c
│   │   └── debug.c
│   └── platform/          # Platform-specific code
│       ├── platform_linux.c
│       └── platform_windows.c
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
- Platform: `module_platform.c` (e.g., `platform_linux.c`)

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
```

### Memory Checking

```bash
# Linux: Valgrind
valgrind --leak-check=full ./yuki-frame config.conf

# AddressSanitizer
cmake -DCMAKE_C_FLAGS="-fsanitize=address" ..
cmake --build .
./yuki-frame config.conf
```

## Debugging

### GDB (Linux)

```bash
# Start with GDB
gdb ./yuki-frame

# In GDB
(gdb) run config.conf
(gdb) break main
(gdb) continue
(gdb) print variable_name
(gdb) backtrace
```

### Visual Studio (Windows)

1. Open `yuki-frame.sln` in Visual Studio
2. Set breakpoints
3. Press F5 to debug

### Logging

```c
// Set log level in code
logger_set_level(LOG_DEBUG);

// Or in config file
[framework]
log_level = DEBUG
```

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
```

### Pull Request Guidelines

- **Size:** 200-400 lines is ideal
- **Scope:** One feature or fix per PR
- **Tests:** Include tests for new features
- **Documentation:** Update docs for API changes
- **Review:** Address review comments promptly

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

1. Update version in `CMakeLists.txt`
2. Update `CHANGELOG.md`
3. Update version defines in `framework.h`
4. Run full test suite
5. Build release packages
6. Tag release: `git tag -a v2.0.0 -m "Release 2.0.0"`
7. Push tag: `git push origin v2.0.0`
8. Create GitHub release with binaries

## Architecture

### Component Overview

**Framework Core:**
- `main.c`: Entry point, main event loop
- `tool.c`: Tool lifecycle management
- `event.c`: Event bus implementation
- `logger.c`: Logging subsystem
- `config.c`: Configuration parser
- `control.c`: Remote control interface
- `debug.c`: Debug/diagnostics

**Platform Layer:**
- `platform_linux.c`: POSIX/Linux implementation
- `platform_windows.c`: Windows API implementation

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
→ Subscribers process event
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

## Troubleshooting

### Common Issues

**Build fails on Windows:**
- Ensure Visual Studio C++ tools are installed
- Use "x64 Native Tools Command Prompt"

**Tests fail:**
- Check port conflicts (control port 9999)
- Ensure Python 3.x is installed (for integration tests)

**Tool doesn't start:**
- Check tool path in config
- Verify tool has execute permissions
- Check logs: `tail -f /var/log/yuki-frame/yuki-frame.log`

**Memory leaks:**
- Run with Valgrind or AddressSanitizer
- Check all `malloc()` have corresponding `free()`
- Review tool cleanup in `tool_unregister()`

## Resources

- **Issue Tracker:** https://github.com/your-org/yuki-frame/issues
- **Discussions:** https://github.com/your-org/yuki-frame/discussions
- **API Documentation:** `docs/api/`
- **Examples:** `examples/`

## Contact

- **Maintainer:** maintainer@example.com
- **Chat:** #yuki-frame on Discord
- **Mailing List:** yuki-frame@lists.example.com
