# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/),
and this project adheres to [Semantic Versioning](https://semver.org/).

## [2.0.0] - 2026-01-22

### Major Changes - Integrated Control API

#### Integrated Control System
- **NEW**: Control API integrated into framework core (`control_api.h`, `control_api.c`)
- **NEW**: Any tool can manage other tools programmatically
- **NEW**: Interactive console mode (`-i` flag) for direct control
- **REMOVED**: Separate control executable (no longer needed)
- **REMOVED**: File-based control scripts (yuki-control.sh/.bat)
- **REMOVED**: IPC-based control module

The Control API provides functions like:
- `control_start_tool()` - Start a tool
- `control_stop_tool()` - Stop a tool  
- `control_restart_tool()` - Restart a tool
- `control_list_tools()` - List all tools
- `control_get_tool_status()` - Get tool information
- `control_shutdown_framework()` - Shutdown framework

#### Integrated Features
- Enhanced logging with log levels (TRACE, DEBUG, INFO, WARN, ERROR, FATAL)
- Log rotation support
- Built-in debug mode (`-d` flag)
- Tool health monitoring
- Automatic restart with exponential backoff

### New Features

#### Interactive Console
- Start framework with `-i` flag for interactive mode
- Type commands directly: `list`, `start <tool>`, `stop <tool>`, `status <tool>`
- Built-in help system
- Console runs in separate thread (Unix/Linux only)

#### Control API
- C/C++ tools can use Control API to manage other tools
- Build watchdog tools, schedulers, admin dashboards
- Simple function calls - no IPC overhead
- Thread-safe operation

#### Better Tool Management
- Real-time tool status
- Event statistics (sent/received counts)
- Restart count tracking
- Heartbeat monitoring

### Improvements

#### Architecture
- Simplified: Control is part of core, not a separate component
- More reliable: Direct function calls instead of events/IPC
- More flexible: Any tool can control others
- Better performance: No IPC overhead

#### Usability
- One executable instead of multiple
- Interactive console for quick operations
- Better error messages
- Clearer documentation

### Migration from v1.0

#### What Changed

**Old Way (v1.0):**
```bash
# Separate control utility
./yuki-control start my_tool

# Or file-based scripts
./yuki-control.sh start my_tool
```

**New Way (v2.0):**
```bash
# Interactive console
./yuki-frame -c config.conf -i
yuki> start my_tool
```

**For Tools (C/C++):**
```c
// Old: Send control events (v1.0)
event_publish("CONTROL", "my_tool", "START other_tool");

// New: Use Control API (v2.0)
control_start_tool("other_tool");
```

#### Configuration Changes

**Remove these sections** (no longer needed):
```ini
# DELETE - Control is now built-in
[tool:control]
...

# DELETE - Config sender not needed
[tool:config_sender]
...
```

**Update core section:**
```ini
[core]
log_level = INFO          # NEW: Set log level
enable_debug = no         # NEW: Enable debug mode
```

Tool configuration remains the same!

### Deprecations

- ❌ Separate `yuki-control` executable (removed)
- ❌ File-based control scripts (removed)  
- ❌ Control module as a tool (removed)
- ❌ IPC-based control (removed)

### Platform Support

- ✅ Linux (tested on Ubuntu 20.04+)
- ✅ Windows (tested on Windows 10/11)
- ✅ macOS (tested on macOS 12+)
- ⚠️  Console mode: Unix/Linux only (uses pthread)

### Documentation

- NEW: `docs/CONTROL_API.md` - Complete Control API reference
- Updated: `docs/DEVELOPMENT.md` - Framework development guide
- Updated: `docs/TOOL_DEVELOPMENT.md` - Tool development with Control API
- Updated: `README.md` - Quick start with new architecture

### Known Issues

- Console mode not available on Windows (pthread dependency)
- Some tools may need updates to use new Control API

### Upgrade Instructions

1. **Remove old control tools from config:**
   ```bash
   # Edit yuki-frame.conf
   # Delete [tool:control] section
   # Delete [tool:config_sender] section
   ```

2. **Rebuild framework:**
   ```bash
   mkdir build && cd build
   cmake ..
   cmake --build .
   ```

3. **Test interactive console:**
   ```bash
   ./yuki-frame -c config.conf -i
   yuki> help
   yuki> list
   ```

4. **Update C/C++ tools to use Control API:**
   ```c
   #include "yuki_frame/control_api.h"
   
   // Now you can control other tools!
   control_start_tool("backup");
   ```

### Breaking Changes

- Control commands via events no longer supported
- Separate yuki-control executable removed
- File-based control (yuki-frame.cmd) removed
- Tools expecting control module will need updates

### Thanks

Special thanks to all users who provided feedback on v1.0!

---

## [1.0.0] - 2026-01-10

Initial release with separate control module architecture.

### Features

- Event-driven tool orchestration
- Tool lifecycle management
- Configuration-based tool registry
- Separate control module
- Basic logging and debugging
- Platform support: Linux, Windows, macOS
