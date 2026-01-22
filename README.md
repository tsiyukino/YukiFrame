# Yuki-Frame v2.0

**Event-driven tool orchestration framework with integrated control**

## What's New in v2.0

### 🎯 Integrated Control System
- **No separate control executable needed** - Control is built into the framework
- **Simple file-based commands** - Just write to `yuki-frame.cmd` file
- **Cross-platform control scripts** - `yuki-control.sh` (Linux) / `yuki-control.bat` (Windows)
- **Real-time tool status** - Accurate PID and status information
- **No IPC complexity** - Simple, reliable file-based communication

### ✨ Core Features
- Built-in tool lifecycle management (start/stop/restart)
- Real-time tool status monitoring
- Event tracing and debugging
- Health checks and automatic restart
- Better error handling and reporting

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

### 2. Configure

Edit `yuki-frame.conf`:

```ini
[core]
log_file = logs/yuki-frame.log
log_level = INFO
enable_debug = yes

[tool:monitor]
command = python tools/monitor.py
autostart = yes              # Starts automatically
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

### 4. Control Tools

In another terminal:

```bash
# Linux/macOS
./yuki-control.sh list
./yuki-control.sh start backup
./yuki-control.sh status backup
./yuki-control.sh stop backup

# Windows
yuki-control.bat list
yuki-control.bat start backup
yuki-control.bat status backup
yuki-control.bat stop backup
```

## How Control Works

### File-Based Command System

Yuki-Frame v2.0 uses a simple file-based control system:

1. **Write command** → The control script writes to `yuki-frame.cmd`
2. **Framework reads** → Main loop detects the command file
3. **Execute command** → Framework performs the requested action
4. **Write response** → Result is written to `yuki-frame.response`
5. **Display result** → Control script shows the response

### Available Commands

```bash
# List all tools with their current status
yuki-control list

# Start a tool
yuki-control start <tool_name>

# Stop a running tool
yuki-control stop <tool_name>

# Restart a tool
yuki-control restart <tool_name>

# Show detailed tool status
yuki-control status <tool_name>

# Shutdown the framework
yuki-control shutdown
```

### Example Output

```bash
$ yuki-control list

Tools Status:
Name                 Status     PID       
------------------------------------------------------------
monitor              RUNNING    12345     
sender               RUNNING    12346     
backup               STOPPED    0         

$ yuki-control status monitor

Tool Status:
  Name: monitor
  Command: python tools/monitor.py
  Description: System resource monitor
  Status: RUNNING
  PID: 12345
  Autostart: yes
  Restart on crash: yes
  Max restarts: 5
  Restart count: 0
  Events sent: 42
  Events received: 0
  Subscriptions:
    - ALERT
```

## Architecture

```
┌─────────────────────────────────────────┐
│         Yuki-Frame Process              │
│                                         │
│  ┌───────────────────────────────────┐ │
│  │  Main Loop                        │ │
│  │  • Process events                 │ │
│  │  • Monitor tool output            │ │
│  │  • Check health                   │ │
│  │  • Check for commands ◄───────────┼─┼── yuki-frame.cmd
│  └───────────────────────────────────┘ │
│                                         │
│  ┌───────────────────────────────────┐ │
│  │  Tool Manager                     │ │
│  │  • test_echo      (PID: 12345)    │ │
│  │  • test_status    (PID: 12346)    │ │
│  │  • test_hello     (STOPPED)       │ │
│  └───────────────────────────────────┘ │
│                                         │
└─────────────────────────────────────────┘
                    │
                    └──────────────────────► yuki-frame.response
```

## Configuration

### Core Settings

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
```

### Tool Configuration

```ini
[tool:my_tool]
command = /path/to/tool
description = Tool description
autostart = yes         # Start automatically (yes/no)
restart_on_crash = yes  # Auto-restart on crash (yes/no)
max_restarts = 3       # Max restart attempts (0 = unlimited)
subscribe_to = EVENT1,EVENT2  # Event subscriptions
```

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
sys.stdout.flush()  # CRITICAL: Must flush!

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

## Command Line Options

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

### Control Script

```bash
# List all tools
yuki-control list

# Control specific tool
yuki-control start tool_name
yuki-control stop tool_name
yuki-control restart tool_name

# Get detailed status
yuki-control status tool_name

# Shutdown framework
yuki-control shutdown

# Help
yuki-control help
```

## Use Cases

### On-Demand Tools

Start tools only when needed:

```ini
[tool:backup]
command = python tools/backup.py
autostart = no          # Don't start automatically
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

### Control commands timeout

```bash
# Check if framework is running
ps aux | grep yuki-frame    # Linux
tasklist | findstr yuki     # Windows

# Check for command file issues
ls -la yuki-frame.cmd       # Should not exist when idle
ls -la yuki-frame.response  # Should appear briefly during commands
```

### Events not routing

1. Check subscription: `subscribe_to = EVENT_TYPE`
2. Check event format: `TYPE|sender|data`
3. Enable debug: `./yuki-frame -d`
4. Check logs for event routing

## Platform Support

- ✅ **Linux** (tested on Ubuntu 20.04+)
- ✅ **Windows** (tested on Windows 10/11)
- ✅ **macOS** (should work, tested on macOS 12+)
- ✅ **BSD** (should work)

## Documentation

| Document | Description |
|----------|-------------|
| `README.md` | This file (overview and quick start) |
| `TOOL_DEVELOPMENT.md` | **Complete guide to writing tools** |
| `DEVELOPMENT.md` | Framework development guide |
| `TESTING.md` | Testing and debugging guide |
| `CHANGELOG.md` | Version history |
| `LICENSE` | MIT License |

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

## Installation

### From Source

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
sudo cmake --install .
```

Installs:
- `/usr/local/bin/yuki-frame` - Main executable
- `/usr/local/bin/yuki-control` - Control script
- `/usr/local/include/yuki_frame/` - Headers
- `/etc/yuki-frame/yuki-frame.conf.example` - Example config

## License

MIT License - See `LICENSE` file for details.

## Contributing

We welcome contributions! See `DEVELOPMENT.md` for:
- Development setup
- Coding standards
- Testing requirements
- Pull request process

## Support

- **Documentation**: See markdown files in project root
- **Examples**: See `examples/` and `tools/` directories
- **Issues**: Report bugs via GitHub issues
- **Questions**: Open a discussion on GitHub

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
yuki-control list               # List all tools
yuki-control start tool_name    # Start tool
yuki-control stop tool_name     # Stop tool
yuki-control restart tool_name  # Restart tool
yuki-control status tool_name   # Show status
yuki-control shutdown           # Shutdown framework
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
autostart = no              # Manual start via yuki-control
subscribe_to = EVENT_TYPE
```

**See `TOOL_DEVELOPMENT.md` for complete guide!**
