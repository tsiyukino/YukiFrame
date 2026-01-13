# Changelog

## [2.0.0] - 2026-01-13

### Major Changes - Core Integration

#### Integrated Control System
- **BREAKING**: Removed separate control module
- Control functionality now built into framework core
- Direct API: `control_start_tool()`, `control_stop_tool()`, etc.
- No need to configure control as a tool
- Eliminated IPC overhead between framework and control

#### Integrated Logging System  
- Enhanced logging with log levels (TRACE, DEBUG, INFO, WARN, ERROR, FATAL)
- Log rotation support
- Per-component logging
- Tool output aggregation
- Timestamped entries

#### Integrated Debug System
- Built-in debug mode (`-d` flag)
- Event tracing and tracking
- Debug event buffer (last 1000 events)
- State dumping: `debug_dump_state()`
- Performance profiling hooks

### New Features

#### Core Features
- Health monitoring for all tools
- Automatic restart with backoff
- Tool statistics (events sent/received, uptime)
- Heartbeat monitoring
- Graceful shutdown handling

#### API Improvements
- Control API for external integration
- Debug API for troubleshooting
- Unified error handling
- Better return codes

#### Configuration
- `enable_debug` option in [core]
- `enable_remote_control` option
- Simplified tool configuration
- Better validation and error messages

### Improvements

#### Performance
- Reduced IPC overhead (control is now internal)
- Faster tool startup
- More efficient event routing
- Lower memory footprint

#### Reliability
- Core features can't crash independently
- Better error recovery
- Atomic operations
- Thread-safe logging

#### Usability
- Simpler architecture (fewer components)
- Unified control interface
- Better error messages
- Clearer documentation

### Migration Guide

#### Remove These from Config
```ini
# DELETE - No longer needed
[tool:control]
command = modules/control/control_module.exe
...

# DELETE - No longer needed
[tool:config_sender]
command = python control_config_sender.py
...
```

#### Update Core Config
```ini
[core]
log_file = logs/yuki-frame.log
log_level = INFO          # NEW: Set log level
enable_debug = no         # NEW: Enable debug mode
enable_remote_control = no  # NEW: Remote control API
```

#### Tool Config (No Changes)
```ini
# Tool configuration remains the same
[tool:my_tool]
command = /path/to/tool
autostart = yes
...
```

### Deprecations

- ❌ Separate control module (removed)
- ❌ Config sender module (removed)
- ❌ `COMMAND` event type (use API instead)
- ❌ `CONFIG` event type (use API instead)

### API Changes

#### Old Way (v1.0)
```python
# Send event to control module
print("START_TOOL|control|my_tool")

# Wait for response event
for line in sys.stdin:
    if line.startswith("RESPONSE|control|"):
        # Handle response
```

#### New Way (v2.0)
```c
// Direct API call
int result = control_start_tool("my_tool");

// Or use control protocol
ControlRequest req;
req.command = CMD_START_TOOL;
strcpy(req.tool_name, "my_tool");

ControlResponse res;
control_process_command(&req, &res);
```

### File Structure Changes

#### Removed
- `modules/control/` directory
- `control_module.exe`
- `control_config_sender.py`

#### Added
- `include/framework.h` - Enhanced with control/debug
- `src/core/control.c` - Integrated control
- `src/core/debug.c` - Integrated debug

#### Modified
- `src/core/main.c` - Integrated initialization
- `include/tool.h` - Enhanced tool structure
- `CMakeLists.txt` - Simplified build

### Platform Support

- ✅ Linux (tested)
- ✅ Windows (tested)
- ✅ macOS (should work)
- ✅ BSD (should work)

### Known Issues

None at this time.

### Upgrade Instructions

1. **Backup your config**
   ```bash
   cp yuki-frame.conf yuki-frame.conf.backup
   ```

2. **Remove old modules from config**
   - Delete `[tool:control]` section
   - Delete `[tool:config_sender]` section

3. **Update core section**
   ```ini
   [core]
   log_level = INFO
   enable_debug = no
   ```

4. **Rebuild**
   ```bash
   mkdir build && cd build
   cmake ..
   cmake --build .
   ```

5. **Test**
   ```bash
   ./yuki-frame -c yuki-frame.conf -d
   ```

6. **Check logs**
   ```bash
   tail -f logs/yuki-frame.log
   ```

### Thanks

Special thanks to all users who provided feedback on v1.0!

---

## [1.0.0] - 2026-01-10

Initial release with separate control module architecture.
