# Yuki-Frame v2.0

**Event-driven tool orchestration framework with integrated control and debug**

## What's New in v2.0

### 🎯 Core Integration
- **Control Module → Core Feature**: Tool management is now built into the framework
- **Logging → Core Feature**: Advanced logging with rotation and filtering
- **Debug → Core Feature**: Integrated debugging and event tracing
- **Simplified Architecture**: Fewer moving parts, easier to understand

### ✨ New Features
- Built-in tool lifecycle management (start/stop/restart)
- Real-time tool status monitoring
- Event tracing and debugging
- Health checks and automatic restart
- Control API for external tools
- Better error handling and reporting

### 🚀 Benefits
- **Simpler**: No separate control module needed
- **Faster**: Direct integration, no IPC overhead  
- **Reliable**: Core features can't crash independently
- **Easier**: Unified configuration and control

## Quick Start

### 1. Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### 2. Configure

Edit `yuki-frame.conf`:

```ini
[core]
log_file = logs/yuki-frame.log
log_level = INFO
enable_debug = yes

[tool:my_tool]
command = python tools/my_tool.py
autostart = yes
restart_on_crash = yes
subscribe_to = EVENT_TYPE
```

### 3. Run

```bash
./yuki-frame -c yuki-frame.conf
```

## Architecture

```
┌─────────────────────────────────────────┐
│         Yuki-Frame Core v2.0            │
├─────────────────────────────────────────┤
│  • Control (integrated)                 │
│  • Logging (integrated)                 │
│  • Debug (integrated)                   │
│  • Event Bus                            │
│  • Tool Registry                        │
│  • Process Manager                      │
└─────────────────────────────────────────┘
          ↓           ↓           ↓
    [Tool A]      [Tool B]      [Tool C]
```

## Core Features

### Control System (Built-in)
```c
// Start/stop/restart tools
control_start_tool("my_tool");
control_stop_tool("my_tool");
control_restart_tool("my_tool");

// Get status
char status[1024];
control_get_status("my_tool", status, sizeof(status));

// List all tools
char list[4096];
control_list_tools(list, sizeof(list));
```

### Debug System (Built-in)
```c
// Enable debug mode
./yuki-frame -d

// Debug logging
debug_log(DEBUG_TOOL_START, "my_tool", "Started with PID %d", pid);
debug_log(DEBUG_EVENT_PUBLISH, "my_tool", "Event: %s", event_type);

// Dump debug state
debug_dump_state();
```

### Logging (Built-in)
```c
LOG_INFO("component", "Message");
LOG_ERROR("component", "Error: %s", error);
LOG_DEBUG("component", "Details: %d", value);
```

## Tool Development

Tools work the same as before:

```python
#!/usr/bin/env python3
import sys

# Log to stderr
print("[INFO] Tool started", file=sys.stderr)

# Emit events to stdout
print("EVENT_TYPE|my_tool|data")
sys.stdout.flush()

# Read events from stdin
for line in sys.stdin:
    event_type, sender, data = line.strip().split('|', 2)
    # Process event
```

See `TOOL_DEVELOPMENT.md` for complete guide.

## Configuration

```ini
[core]
# Logging
log_file = /var/log/yuki-frame/yuki-frame.log
log_level = INFO        # TRACE, DEBUG, INFO, WARN, ERROR, FATAL

# Process
pid_file = /var/run/yuki-frame.pid
max_tools = 100
message_queue_size = 1000

# Debug
enable_debug = no       # Enable debug tracing
enable_remote_control = no
control_port = 9999

[tool:my_tool]
command = /path/to/tool
description = Tool description
autostart = yes
restart_on_crash = yes
max_restarts = 3
subscribe_to = EVENT1,EVENT2
```

## Migration from v1.0

### What Changed

**v1.0 (Old)**:
```ini
[tool:control]
command = modules/control/control_module.exe
autostart = yes
subscribe_to = COMMAND,CONFIG
```

**v2.0 (New)**:
```ini
# Control is built-in, no configuration needed!
# Just use the core features directly
```

### Migration Steps

1. **Remove control module** from config
2. **Remove config_sender** from config  
3. **Update log paths** if needed
4. **Rebuild** with new CMakeLists.txt
5. **Test** - everything else works the same!

### API Changes

**Old (v1.0)**:
```
Send "START_TOOL|control|tool_name" event
Wait for "RESPONSE|control|..." event
```

**New (v2.0)**:
```c
// Direct API call
control_start_tool("tool_name");

// Or use control protocol (compatible)
ControlRequest req = {.command = CMD_START_TOOL, ...};
ControlResponse res;
control_process_command(&req, &res);
```

## Command Line

```bash
# Start framework
yuki-frame -c config.conf

# With debug mode
yuki-frame -c config.conf -d

# Show version
yuki-frame -v

# Show help
yuki-frame -h
```

## Building

### Linux/macOS

```bash
mkdir build && cd build
cmake ..
cmake --build .
sudo cmake --install .
```

### Windows

```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

## Documentation

- **README.md** - This file (overview)
- **GETTING_STARTED.md** - Quick start guide
- **TOOL_DEVELOPMENT.md** - How to write tools
- **ARCHITECTURE.md** - System design
- **CHANGELOG.md** - Version history

## Troubleshooting

### Control module not found
✅ **Fixed in v2.0** - Control is built-in, no separate module!

### Tools not starting
Check logs: `tail -f logs/yuki-frame.log`

### Events not routing
1. Check subscription: `subscribe_to = EVENT_TYPE`
2. Check event format: `TYPE|sender|data`
3. Enable debug: `./yuki-frame -d`

## License

MIT License - See LICENSE file

## Support

- GitHub Issues: [Report bugs]
- Documentation: See `docs/` folder
- Examples: See `examples/` folder
