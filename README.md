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
- **Manual tool control** with `yuki-control` utility
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
- **Flexible**: Start tools automatically or manually on-demand

## Quick Start

### 1. Build

```bash
# Linux/macOS
./build.sh

# Windows
build.bat

# Or manually
mkdir build && cd build
cmake ..
cmake --build .
```

This creates two executables:
- `yuki-frame` - Main framework
- `yuki-control` - Control utility for manual tool management

### 2. Configure

Edit `yuki-frame.conf`:

```ini
[core]
log_file = logs/yuki-frame.log
log_level = INFO
enable_debug = yes

[tool:monitor]
command = python tools/monitor.py
autostart = yes              # Starts automatically with framework
restart_on_crash = yes
subscribe_to = ALERT

[tool:backup]
command = python tools/backup.py
autostart = no               # Start manually when needed
restart_on_crash = no
subscribe_to = 
```

### 3. Run Framework

```bash
# Linux/macOS
./build/yuki-frame -c yuki-frame.conf

# Windows
build\Release\yuki-frame.exe -c yuki-frame.conf
```

### 4. Control Tools Manually

In another terminal:

```bash
# Start a tool
yuki-control start backup

# Stop a tool
yuki-control stop backup

# Restart a tool
yuki-control restart monitor

# List all tools
yuki-control list

# Get detailed status
yuki-control status monitor
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
         ↑                           ↑
         └───── yuki-control ────────┘
         (manual start/stop/restart)
```

## Core Features

### Manual Tool Control (NEW!)

Control tools while framework is running:

```bash
# Start framework
yuki-frame -c config.conf

# In another terminal, control tools:
yuki-control start my_tool      # Start tool on-demand
yuki-control stop my_tool       # Stop running tool
yuki-control restart my_tool    # Restart tool
yuki-control list              # List all tools and status
yuki-control status my_tool    # Show detailed tool status
```

**Example output:**
```
$ yuki-control list
Registered Tools:
-----------------
monitor              RUNNING    PID: 1234
sender               RUNNING    PID: 5678
backup               STOPPED    PID: 0

$ yuki-control status monitor
Tool Status:
------------
Tool: monitor
Status: RUNNING
PID: 1234
Events Sent: 42
Events Received: 0
Restart Count: 0
```

### Automatic Tool Management

Tools with `autostart = yes` start automatically:

```ini
[tool:monitor]
command = python tools/monitor.py
autostart = yes              # Starts with framework
restart_on_crash = yes       # Auto-restarts if crashes
max_restarts = 5            # Max restart attempts
```

### Debug System

```bash
# Enable debug mode
./yuki-frame -d -c config.conf

# Debug logging shows:
# - Tool start/stop events
# - Event routing
# - Process spawning
# - Health checks
```

### Logging

```c
LOG_INFO("component", "Message");
LOG_ERROR("component", "Error: %s", error);
LOG_DEBUG("component", "Details: %d", value);
```

All tool stdout/stderr captured in framework logs.

## Tool Development

Tools communicate via standard I/O:

```python
#!/usr/bin/env python3
import sys
import signal

running = True

def signal_handler(sig, frame):
    global running
    running = False

signal.signal(signal.SIGTERM, signal_handler)

# Log to stderr (captured by framework)
print("[INFO] Tool started", file=sys.stderr)

# Emit events to stdout
print("STATUS|my_tool|System OK")
sys.stdout.flush()

# Read events from stdin
for line in sys.stdin:
    if not running:
        break
    
    event_type, sender, data = line.strip().split('|', 2)
    # Process event
    print(f"[INFO] Received {event_type}", file=sys.stderr)

print("[INFO] Tool stopped", file=sys.stderr)
```

**Complete guide**: See `TOOL_DEVELOPMENT.md`

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
autostart = yes         # Start automatically (yes/no)
restart_on_crash = yes  # Auto-restart on crash (yes/no)
max_restarts = 3       # Max restart attempts (0 = unlimited)
subscribe_to = EVENT1,EVENT2  # Event subscriptions
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
# Use yuki-control utility for manual control
```

### Migration Steps

1. **Remove control module** from config
2. **Remove config_sender** from config  
3. **Update log paths** if needed
4. **Rebuild** with new CMakeLists.txt
5. **Test** - everything else works the same!

See `CHANGELOG.md` for complete migration guide.

## Command Line

### Framework

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

### Control Utility

```bash
# Start a tool
yuki-control start tool_name

# Stop a tool
yuki-control stop tool_name

# Restart a tool
yuki-control restart tool_name

# List all tools
yuki-control list

# Show tool status
yuki-control status tool_name

# Help
yuki-control help
```

## Building

### Linux/macOS

```bash
./build.sh              # Release build
./build.sh Debug        # Debug build

# Or manually
mkdir build && cd build
cmake ..
cmake --build .
sudo cmake --install .
```

### Windows

```bash
build.bat              # Release build
build.bat Debug        # Debug build

# Or manually
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

**Output:**
- `build/yuki-frame` (or `build\Release\yuki-frame.exe`)
- `build/yuki-control` (or `build\Release\yuki-control.exe`)

## Testing

Yuki-Frame includes a comprehensive test suite:

```bash
# Run all tests
./run-tests.sh          # Linux/macOS
run-tests.bat           # Windows

# Or manually
cd build
ctest --output-on-failure
```

See `TESTING.md` for complete testing guide.

## Documentation

| Document | Description |
|----------|-------------|
| `README.md` | This file (overview and quick start) |
| `TOOL_DEVELOPMENT.md` | **Complete guide to writing tools** |
| `DEVELOPMENT.md` | Framework development and contribution guide |
| `TESTING.md` | Testing and debugging guide |
| `CHANGELOG.md` | Version history and migration guides |
| `LICENSE` | MIT License |

### Example Tools

See `tools/` directory for working examples:
- `monitor.py` - System monitoring with periodic events
- `echo.py` - Simple event echo
- `alerter.py` - Alert handling
- `sender.py` - Event publisher
- `receiver.py` - Event subscriber

## Use Cases

### On-Demand Tools

Tools that run only when needed:

```ini
[tool:backup]
command = python tools/backup.py
autostart = no          # Don't start automatically
restart_on_crash = no
```

```bash
# Run backup when needed
yuki-control start backup
# ... backup runs ...
yuki-control stop backup
```

### Always-Running Tools

Tools that should always be active:

```ini
[tool:monitor]
command = python tools/monitor.py
autostart = yes         # Start with framework
restart_on_crash = yes  # Restart if crashes
max_restarts = 10      # Up to 10 restart attempts
```

### Event-Driven Workflows

Tools communicate via events:

```ini
[tool:watcher]
command = python tools/file_watcher.py
autostart = yes
subscribe_to =          # Publishes FILE_CHANGED events

[tool:processor]
command = python tools/process_file.py
autostart = yes
subscribe_to = FILE_CHANGED  # Processes files when changed

[tool:notifier]
command = python tools/notifier.py
autostart = yes
subscribe_to = FILE_PROCESSED  # Sends notifications
```

## Troubleshooting

### Tools not starting

**Check logs:**
```bash
tail -f logs/yuki-frame.log
```

**Common issues:**
- Python not in PATH → Use full path: `C:\Python39\python.exe tools\tool.py`
- Tool file not found → Use absolute paths in config
- Permission denied → `chmod +x tools/tool.py` (Linux)

### yuki-control not found

```bash
# Use full path
./build/yuki-control start my_tool

# Or from build directory
cd build
./yuki-control start my_tool
```

### Events not routing

1. Check subscription: `subscribe_to = EVENT_TYPE`
2. Check event format: `TYPE|sender|data`
3. Enable debug: `./yuki-frame -d`
4. Check logs for event routing

### Tool crashes immediately

```bash
# Test tool manually first
python tools/my_tool.py

# Check for errors
# Then add to framework
```

## Platform Support

- ✅ **Linux** (tested on Ubuntu 20.04+)
- ✅ **Windows** (tested on Windows 10/11)
- ✅ **macOS** (should work, tested on macOS 12+)
- ✅ **BSD** (should work)

## Installation

### From Source

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
sudo cmake --install .
```

Installs:
- `/usr/local/bin/yuki-frame`
- `/usr/local/bin/yuki-control`
- `/usr/local/include/yuki_frame/`
- `/etc/yuki-frame/yuki-frame.conf.example`

### Package Installation

```bash
# Ubuntu/Debian
sudo dpkg -i yuki-frame_2.0.0_amd64.deb

# RPM-based
sudo rpm -i yuki-frame-2.0.0.x86_64.rpm

# Windows
# Run installer: yuki-frame-2.0.0-win64.exe
```

## License

MIT License - See `LICENSE` file for details.

## Contributing

We welcome contributions! See `DEVELOPMENT.md` for:
- Development setup
- Coding standards
- Testing requirements
- Pull request process

## Support

- **Documentation**: See `docs/` folder and markdown files
- **Examples**: See `examples/` and `tools/` directories
- **Issues**: Report bugs via GitHub issues
- **Questions**: Open a discussion on GitHub

## Links

- **Repository**: https://github.com/your-org/yuki-frame
- **Documentation**: https://yuki-frame.readthedocs.io
- **Releases**: https://github.com/your-org/yuki-frame/releases

## Version

Current version: **2.0.0** (Released January 2026)

See `CHANGELOG.md` for complete version history.

---

## Quick Reference

### Start Framework
```bash
yuki-frame -c yuki-frame.conf
```

### Control Tools
```bash
yuki-control start tool_name     # Start tool
yuki-control stop tool_name      # Stop tool
yuki-control restart tool_name   # Restart tool
yuki-control list               # List all tools
yuki-control status tool_name   # Show status
```

### Create Tool
```python
#!/usr/bin/env python3
import sys
print("EVENT|my_tool|data")  # Send event
sys.stdout.flush()            # MUST flush!
for line in sys.stdin:        # Receive events
    process(line)
```

### Add Tool to Config
```ini
[tool:my_tool]
command = python tools/my_tool.py
autostart = no              # Manual start
subscribe_to = EVENT_TYPE
```

**See `TOOL_DEVELOPMENT.md` for complete guide!**
