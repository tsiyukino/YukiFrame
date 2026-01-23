# Yuki-Frame v2.0

**Event-driven tool orchestration framework with unified pipe architecture**

Yuki-Frame is a lightweight framework for orchestrating multiple tools through events. **Everything is a tool** - including the console! All tools communicate via stdin/stdout pipes for a uniform, elegant architecture.

## What's New in v2.0 🎉

### Unified Pipe Architecture
- **Everything is a tool** - Console, monitor, alerter, all use the same pattern
- **Consistent communication** - All tools use stdin/stdout (no special cases!)
- **Simple and elegant** - One communication method for everything
- **Easy to test** - Just pipes, no complex IPC

### Key Features
- ✅ Event-driven inter-tool communication
- ✅ Interactive console (as a tool!)
- ✅ Automatic tool restart on crash
- ✅ Health monitoring and statistics
- ✅ Cross-platform (Linux, Windows, macOS)
- ✅ Language-agnostic (Python, Bash, C, any language!)

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

# Console is a tool!
[tool:console]
command = python yuki-console.py
autostart = yes           # Start automatically

[tool:monitor]
command = python tools/monitor.py
autostart = yes
restart_on_crash = yes
subscribe_to = ALERT
```

### 3. Run

```bash
# Just start the framework - console starts automatically!
./yuki-frame -c yuki-frame.conf

# Console appears:
============================================================
  Yuki-Frame Interactive Console v2.0.0
  Type 'help' for commands, 'quit' to exit
============================================================

yuki> list
yuki> start backup
yuki> help
```

## Architecture

```
┌─────────────────────────────────────────┐
│         Yuki-Frame (Hub)                │
│                                         │
│  Event Router + Tool Manager            │
│  • Routes events between tools          │
│  • Manages tool lifecycle               │
│  • Handles commands from console        │
└─────────────────────────────────────────┘
      ▲       ▲        ▲         ▲
      │       │        │         │
  stdin/    stdin/   stdin/   stdin/
  stdout    stdout   stdout   stdout
      │       │        │         │
   Console  Monitor  Alerter  Your Tool
   (tool)   (tool)   (tool)   (tool)
```

**Everything uses the same pattern!** ✅

## Tool Development

### Basic Tool (Python)

```python
#!/usr/bin/env python3
import sys
import signal

running = True
signal.signal(signal.SIGTERM, lambda s,f: globals().__setitem__('running', False))

print("[INFO] Started", file=sys.stderr)

# Main loop
for line in sys.stdin:
    if not running:
        break
    
    # Parse event: TYPE|sender|data
    parts = line.strip().split('|', 2)
    if len(parts) >= 3:
        event_type, sender, data = parts
        
        # Process event
        print(f"[INFO] Got {event_type} from {sender}", file=sys.stderr)
        
        # Send response
        print(f"RESPONSE|my_tool|Processed {event_type}")
        sys.stdout.flush()

print("[INFO] Stopped", file=sys.stderr)
```

### Console Tool

The console is just a tool that:
- Receives user input from terminal
- Sends COMMAND events to framework
- Receives RESPONSE events back
- Displays responses to user

**See `yuki-console.py` for full implementation!**

## Console Commands

```
list                 - List all tools and their status
start <tool>         - Start a tool
stop <tool>          - Stop a tool
restart <tool>       - Restart a tool
status <tool>        - Show detailed tool status
uptime               - Show framework uptime
version              - Show framework version
shutdown             - Shutdown the framework
help                 - Show this help message
quit                 - Exit console (framework continues)
```

## Event Protocol

All tools communicate using simple text events:

```
Format: TYPE|sender|data

Examples:
ALERT|monitor|High CPU usage: 95%
STATUS|backup|Backup completed successfully
ERROR|database|Connection failed
COMMAND|console|start backup
RESPONSE|framework|Tool 'backup' started PID 12345
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

## Command Line Options

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

## Documentation

| Document | Description |
|----------|-------------|
| **README.md** | This file - quick start and overview |
| **[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)** | Design philosophy and architecture |
| **[docs/TOOL_DEVELOPMENT.md](docs/TOOL_DEVELOPMENT.md)** | Guide to writing tools |
| **[docs/DEVELOPMENT.md](docs/DEVELOPMENT.md)** | Framework development guide |
| **[docs/TESTING.md](docs/TESTING.md)** | Testing and debugging |
| **[docs/CHANGELOG.md](docs/CHANGELOG.md)** | Version history |
| **LICENSE** | MIT License |

## Platform Support

- ✅ **Linux** (tested on Ubuntu 20.04+)
- ✅ **Windows** (tested on Windows 10/11)
- ✅ **macOS** (tested on macOS 12+)

## Examples

See `tools/` directory for example tools:
- `monitor.py` - System resource monitoring
- `alerter.py` - Alert processing
- `echo.py` - Simple echo tool
- `sender.py` / `receiver.py` - Message passing example
- **`yuki-console.py`** - Interactive console (reference implementation)

## Philosophy

### Everything is a Tool

**Why this design?**

1. **Simplicity** - One communication pattern for everything
2. **Consistency** - No special cases, no exceptions
3. **Testability** - Just pipes, easy to test
4. **Flexibility** - Add/remove tools without changing framework
5. **Language-agnostic** - Works with any language

**Even the console is a tool!**

```
Traditional:                 Yuki-Frame:
Framework ← socket ← Console    Framework ← pipes ← Console (tool)
Framework ← pipes ← Tools       Framework ← pipes ← Other tools

(Asymmetric)                    (Symmetric!) ✅
```

## Migration from v1.0

**No breaking changes for existing tools!**

Tools using stdin/stdout continue to work as-is.

**What's new:**
- Console is now configured as a tool
- Remove `-i` flag (not needed anymore)
- Add `[tool:console]` section to config

**Old way:**
```bash
./yuki-frame -c config.conf -i
```

**New way:**
```bash
# Just add console to config
[tool:console]
command = python yuki-console.py
autostart = yes

# Then run
./yuki-frame -c config.conf
# Console starts automatically!
```

## Troubleshooting

### Console not appearing?
```bash
# Check if console tool is in config
[tool:console]
command = python yuki-console.py  # ← Must point to yuki-console.py
autostart = yes                   # ← Must be yes

# Check logs
tail -f logs/yuki-frame.log
```

### Tools not starting?
```bash
# Enable debug mode
./yuki-frame -c config.conf -d

# Check tool paths are correct
[tool:my_tool]
command = /full/path/to/tool.py  # ← Use absolute path
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
./yuki-frame -c yuki-frame.conf
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
quit                    # Exit console
```

### Create a Tool
```python
#!/usr/bin/env python3
import sys

print("[INFO] Started", file=sys.stderr)

for line in sys.stdin:
    event = line.strip().split('|', 2)
    # Process event
    print("RESPONSE|tool|OK")
    sys.stdout.flush()
```

### Add to Config
```ini
[tool:my_tool]
command = /path/to/my_tool.py
autostart = yes
restart_on_crash = yes
subscribe_to = EVENT_TYPE
```

**Everything is a tool. Everything uses pipes. Simple!** ✅
