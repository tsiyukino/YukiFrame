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
#   - yuki-frame (main framework executable)
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
│   ├── control_api.h      # Control API (NEW in v2.0!)
│   ├── console.h          # Interactive console
│   ├── debug.h            # Debug/diagnostics
│   └── platform.h         # Platform abstraction
├── src/
│   ├── core/              # Core implementation
│   │   ├── main.c           # Framework entry point
│   │   ├── tool.c           # Tool lifecycle management
│   │   ├── event.c          # Event bus
│   │   ├── logger.c         # Logging system
│   │   ├── config.c         # Config parser
│   │   ├── control.c        # Legacy control wrapper
│   │   ├── control_api.c    # Control API implementation (NEW!)
│   │   ├── console.c        # Interactive console (NEW!)
│   │   └── debug.c          # Debug system
│   └── platform/          # Platform-specific code
│       ├── platform_linux.c   # Linux/Unix implementation
│       └── platform_windows.c # Windows implementation
├── tests/
│   ├── unit/              # Unit tests
│   └── integration/       # Integration tests
├── examples/              # Usage examples
├── tools/                 # Example tool implementations
└── docs/                  # Documentation (this file!)
```

## Coding Standards

See `docs/CODING_STANDARDS.md` for complete style guide.

### Quick Reference

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

// Constants/Macros: SCREAMING_SNAKE_CASE
#define MAX_BUFFER_SIZE 1024

// Globals: prefix with 'g_'
extern FrameworkConfig g_config;
```

**K&R Style (Mandatory):**
```c
int function_name(int arg1, const char* arg2) {
    if (condition) {
        // code
    } else {
        // code
    }
}
```

## Building and Testing

### Build Types

```bash
# Debug build (with symbols, no optimization)
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release build (optimized, no debug symbols)
cmake -DCMAKE_BUILD_TYPE=Release ..
```

### Running Tests

```bash
# Build and run all tests
cmake --build . --target test

# Run with verbose output
ctest --verbose

# Run specific test
./tests/unit/test_event_bus
```

See `docs/TESTING.md` for comprehensive testing guide.

## Core Components

### 1. Control API (NEW in v2.0!)

**The Control API is integrated into the framework core:**

```c
// In control_api.h
int control_start_tool(const char* tool_name);
int control_stop_tool(const char* tool_name);
int control_restart_tool(const char* tool_name);
int control_get_tool_status(const char* tool_name, ControlToolInfo* info);
int control_list_tools(control_list_callback_t callback, void* user_data);
int control_shutdown_framework(void);
```

**Any tool can use these functions!** They're part of the framework, not a separate module.

See `docs/CONTROL_API.md` for complete API reference.

### 2. Interactive Console (NEW in v2.0!)

The console is an optional feature that uses the Control API:

```bash
# Start framework with interactive console
./yuki-frame -c config.conf -i

# In console:
yuki> list
yuki> start my_tool
yuki> status my_tool
yuki> help
```

The console runs in a separate thread and is just a user interface to the Control API.

### 3. Process Spawning (Platform Layer)

**Linux** (`platform_linux.c`):
```c
ProcessHandle platform_spawn_process(const char* command,
                                     int* stdin_fd,
                                     int* stdout_fd,
                                     int* stderr_fd);
```

**Windows** (`platform_windows.c`):
- Uses CreateProcess() API
- Same interface as Linux

### 4. Tool Management (`tool.c`)

```c
int tool_start(const char* name);
int tool_stop(const char* name);
int tool_restart(const char* name);
void tool_check_health(void);
```

### 5. Event System (`event.c`)

```c
int event_publish(const char* type, const char* sender, const char* data);
void event_process_queue(void);
```

## Debugging

### Quick Reference

```bash
# Build debug version
mkdir build-debug && cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .

# Run with GDB
gdb ./yuki-frame
(gdb) run -c ../yuki-frame.conf

# Common GDB commands
(gdb) break tool_start
(gdb) next
(gdb) print tool->pid
(gdb) backtrace
```

### Debug Logging

```bash
# Enable debug mode
./yuki-frame -c config.conf -d

# Or in config file:
[core]
log_level = DEBUG
enable_debug = yes
```

See `docs/TESTING.md` for complete debugging guide.

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

Longer explanation if needed.

Fixes #123
```

**Types:** feat, fix, docs, style, refactor, test, chore

## Architecture

### Component Overview

**Framework Core:**
- `main.c`: Entry point, main event loop
- `tool.c`: Tool lifecycle management
- `event.c`: Event bus implementation
- `logger.c`: Logging subsystem
- `config.c`: Configuration parser
- `control_api.c`: Control API (NEW!)
- `console.c`: Interactive console (NEW!)
- `debug.c`: Debug/diagnostics

**Platform Layer:**
- `platform_linux.c`: POSIX implementation
- `platform_windows.c`: Windows API implementation

### Threading Model

- Single-threaded event-driven architecture
- Tools run as separate processes
- Console runs in separate thread (optional)
- No mutexes needed for main loop

## Resources

- **Issue Tracker:** https://github.com/your-org/yuki-frame/issues
- **Main Documentation:** See `README.md`
- **Tool Development:** See `docs/TOOL_DEVELOPMENT.md`
- **Testing Guide:** See `docs/TESTING.md`
- **Control API Reference:** See `docs/CONTROL_API.md`

## License

MIT License - See `LICENSE` file
