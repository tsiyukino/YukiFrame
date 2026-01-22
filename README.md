# Yuki-Frame v2.0

**Event-driven tool orchestration framework with integrated control**

Yuki-Frame is a lightweight framework for orchestrating multiple tools through events and direct control. Tools communicate via stdin/stdout, and can manage each other using the integrated Control API.

## What's New in v2.0 🎉

### Integrated Control API
- **Control is built into the framework** - no separate executable needed
- **Any tool can manage other tools** - start, stop, restart, query status
- **Interactive console mode** - type commands directly with `-i` flag
- **Simpler architecture** - one executable, easier to understand

### Key Features
- ✅ Event-driven inter-tool communication
- ✅ Integrated Control API for tool management
- ✅ Interactive console for quick operations
- ✅ Automatic tool restart on crash
- ✅ Health monitoring and statistics
- ✅ Cross-platform (Linux, Windows, macOS)

## Quick Start

### 1. Build

```bash
# Linux/macOS
mkdir build && cd build
cmake ..
cmake --build .

# Windows
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### 2. Configure

Edit `yuki-frame.conf`:

```ini
[core]
log_file = logs/yuki-frame.log
log_level = INFO

[tool:monitor]
command = python tools/monitor.py
autostart = yes
restart_on_crash = yes
subscribe_to = ALERT

[tool:alerter]
command = python tools/alerter.py
autostart = yes
restart_on_crash = yes
subscribe_to = ALERT,ERROR
```

### 3. Run

```bash
# Basic mode
./yuki-frame -c yuki-frame.conf

# Interactive console mode (recommended!)
./yuki-frame -c yuki-frame.conf -i
```

### 4. Use Interactive Console

```
yuki> list
Tools Status:
Name                 Status     PID       
------------------------------------------------------------
monitor              RUNNING    12345     
alerter              RUNNING    12346     

yuki> start backup
Success: Tool 'backup' started

yuki> status monitor
Tool Status:
  Name: monitor
  Status: RUNNING
  PID: 12345
  Events sent: 42

yuki> help
Available commands:
  list, start <tool>, stop <tool>, status <tool>, ...

yuki> quit
```

## Architecture

```
┌─────────────────────────────────────────┐
│         Yuki-Frame Process              │
│                                         │
│  ┌───────────────────────────────────┐ │
│  │  Control API (Integrated!)        │ │
│  │  • control_start_tool()           │ │
│  │  • control_stop_tool()            │ │
│  │  • control_list_tools()           │ │
│  └───────────────────────────────────┘ │
│                 ▲                       │
│                 │                       │
│  ┌──────────────┴────────────────────┐ │
│  │  Tool Manager                     │ │
│  │  • monitor      (PID: 12345)      │ │
│  │  • alerter      (PID: 12346)      │ │
│  │  • backup       (STOPPED)         │ │
│  └───────────────────────────────────┘ │
│                                         │
└─────────────────────────────────────────┘
```

**Any tool can use the Control API** to manage other tools!

## Tool Development

Tools communicate via stdin/stdout:

```python
#!/usr/bin/env python3
import sys
import signal

running = True

def signal_handler(sig, frame):
    global running
    running = False

signal.signal(signal.SIGTERM, signal_handler)

# Log to stderr
print("[INFO] Tool started", file=sys.stderr)

# Send events to stdout
print("STATUS|my_tool|System OK")
sys.stdout.flush()  # CRITICAL!

# Read events from stdin
for line in sys.stdin:
    if not running:
        break
    
    event_type, sender, data = line.strip().split('|', 2)
    print(f"[INFO] Received {event_type}", file=sys.stderr)

print("[INFO] Tool stopped", file=sys.stderr)
```

**C/C++ tools can use the Control API:**

```c
#include "yuki_frame/control_api.h"

// Start another tool
control_start_tool("backup");

// Get tool status
ControlToolInfo info;
control_get_tool_status("monitor", &info);
printf("Monitor PID: %u\n", info.pid);

// List all tools
control_list_tools(my_callback, NULL);
```

## Use Cases

### Always-On Monitoring
```ini
[tool:monitor]
autostart = yes
restart_on_crash = yes
max_restarts = 10
```

### On-Demand Tools
```ini
[tool:backup]
autostart = no          # Start manually when needed
```

```bash
# In console:
yuki> start backup
# ... backup runs ...
yuki> stop backup
```

### Event-Driven Workflows
```ini
[tool:watcher]
autostart = yes
# Publishes FILE_CHANGED events

[tool:processor]
autostart = yes
subscribe_to = FILE_CHANGED  # Processes files when changed

[tool:notifier]
autostart = yes
subscribe_to = FILE_PROCESSED  # Sends notifications
```

### Tool Orchestration (NEW!)
```c
// Watchdog tool using Control API
#include "yuki_frame/control_api.h"

bool check_tool(const ControlToolInfo* info, void* data) {
    if (info->status == TOOL_CRASHED) {
        control_restart_tool(info->name);
    }
    return true;
}

int main() {
    while (1) {
        control_list_tools(check_tool, NULL);
        sleep(10);
    }
}
```

## Command Line Options

```bash
# Start framework
yuki-frame -c config.conf

# With interactive console
yuki-frame -c config.conf -i

# With debug mode
yuki-frame -c config.conf -d

# Show version
yuki-frame -v

# Show help
yuki-frame -h
```

## Documentation

| Document | Description |
|----------|-------------|
| **README.md** | This file - quick start and overview |
| **[docs/CONTROL_API.md](docs/CONTROL_API.md)** | Complete Control API reference |
| **[docs/TOOL_DEVELOPMENT.md](docs/TOOL_DEVELOPMENT.md)** | Guide to writing tools |
| **[docs/DEVELOPMENT.md](docs/DEVELOPMENT.md)** | Framework development guide |
| **[docs/TESTING.md](docs/TESTING.md)** | Testing and debugging |
| **[docs/CHANGELOG.md](docs/CHANGELOG.md)** | Version history and migration guide |
| **LICENSE** | MIT License |

## Platform Support

- ✅ **Linux** (tested on Ubuntu 20.04+)
- ✅ **Windows** (tested on Windows 10/11)
- ✅ **macOS** (tested on macOS 12+)
- ⚠️  Console mode: Unix/Linux only (uses pthread)

## Examples

See `tools/` directory for example tools:
- `monitor.py` - System resource monitoring
- `alerter.py` - Alert processing
- `echo.py` - Simple echo tool
- `sender.py` / `receiver.py` - Message passing example

## Migration from v1.0

**What changed:**
- Control is now integrated (no separate `yuki-control` executable)
- Interactive console mode available with `-i` flag
- C/C++ tools can use Control API directly
- Simpler configuration (no control module needed)

**Old way (v1.0):**
```bash
./yuki-control start my_tool
```

**New way (v2.0):**
```bash
./yuki-frame -c config.conf -i
yuki> start my_tool
```

Or from C/C++ tools:
```c
control_start_tool("my_tool");
```

See `docs/CHANGELOG.md` for complete migration guide.

## Troubleshooting

### Tools not starting?
```bash
# Check logs
tail -f logs/yuki-frame.log

# Enable debug mode
./yuki-frame -c config.conf -d
```

### Can't control tools?
```bash
# Use interactive console
./yuki-frame -c config.conf -i
yuki> list
yuki> start my_tool
```

### Events not routing?
1. Check `subscribe_to` in config
2. Verify event format: `TYPE|sender|data`
3. Make sure to flush stdout after printing events
4. Check logs for routing information

See `docs/TESTING.md` for complete debugging guide.

## Contributing

We welcome contributions! See `docs/DEVELOPMENT.md` for:
- Development setup
- Coding standards
- Testing requirements
- Pull request process

## License

MIT License - See `LICENSE` file for details.

## Version

Current version: **2.0.0** (Released January 2026)

See `docs/CHANGELOG.md` for complete version history.

---

## Quick Reference

### Start Framework
```bash
./yuki-frame -c yuki-frame.conf -i
```

### Console Commands
```
list                    # List all tools
start <tool>            # Start a tool
stop <tool>             # Stop a tool
restart <tool>          # Restart a tool
status <tool>           # Show detailed status
shutdown                # Shutdown framework
help                    # Show help
quit                    # Exit console (framework continues)
```

### Create a Tool
```python
#!/usr/bin/env python3
import sys
import signal

running = True
signal.signal(signal.SIGTERM, lambda s,f: globals().__setitem__('running', False))

print("[INFO] Started", file=sys.stderr)

for line in sys.stdin:
    if not running: break
    print("RESPONSE|tool|OK")
    sys.stdout.flush()

print("[INFO] Stopped", file=sys.stderr)
```

### Add to Config
```ini
[tool:my_tool]
command = /path/to/my_tool.py
autostart = yes
restart_on_crash = yes
subscribe_to = EVENT_TYPE
```

**See `docs/TOOL_DEVELOPMENT.md` for complete guide!**
