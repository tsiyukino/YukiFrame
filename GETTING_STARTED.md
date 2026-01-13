# Getting Started with Yuki-Frame v2.0

## Quick Start (5 Minutes)

### 1. Build

**Linux/macOS:**
```bash
./build.sh
```

**Windows:**
```bash
build.bat
```

### 2. Configure

```bash
cp yuki-frame.conf.example yuki-frame.conf
```

The default configuration includes example tools. You can use these or add your own.

### 3. Run

**Linux/macOS:**
```bash
./build/yuki-frame -c yuki-frame.conf
```

**Windows:**
```bash
build\Release\yuki-frame.exe -c yuki-frame.conf
```

### 4. Enable Debug Mode (Optional)

```bash
./build/yuki-frame -c yuki-frame.conf -d
```

## What's New in v2.0

### No More Separate Modules!

**v1.0 (Old)** had separate modules:
- Control module (separate process)
- Config sender (separate process)
- Framework (main process)

**v2.0 (New)** integrates everything:
- Control built-in
- Logging built-in
- Debug built-in
- Framework (single process)

### Simpler Configuration

**v1.0 Required:**
```ini
[tool:control]
command = modules/control/control_module.exe
...

[tool:config_sender]
command = python control_config_sender.py
...
```

**v2.0 Just Works:**
```ini
# Control and logging are automatic!
# Just configure your actual tools

[tool:my_tool]
command = /path/to/tool
autostart = yes
```

## Migration from v1.0

### Step 1: Remove Old Modules

Delete these sections from your config:
- `[tool:control]`
- `[tool:config_sender]`

### Step 2: Add Core Options (Optional)

```ini
[core]
log_level = INFO
enable_debug = no
```

### Step 3: Rebuild

```bash
./build.sh  # or build.bat on Windows
```

### Step 4: Test

```bash
./build/yuki-frame -c yuki-frame.conf -d
```

That's it! Your tools work exactly as before.

## Core Features

### 1. Built-in Control

No need for separate control module. Use the API:

```c
// C API
control_start_tool("my_tool");
control_stop_tool("my_tool");
control_list_tools(buffer, size);
```

Or send control commands via events (compatible mode):

```python
# Still works for backwards compatibility
print("START_TOOL|control|my_tool")
```

### 2. Built-in Debug

Enable with `-d` flag:

```bash
./yuki-frame -c config.conf -d
```

Debug features:
- Event tracing
- Tool lifecycle tracking
- Performance metrics
- State dumping

### 3. Enhanced Logging

Log levels: TRACE, DEBUG, INFO, WARN, ERROR, FATAL

```ini
[core]
log_level = DEBUG  # See more details
```

Logs include:
- Framework events
- Tool output (stderr)
- Event routing
- Errors and warnings

## Tool Development

Tools work exactly the same as v1.0:

### 1. Create Tool

```python
#!/usr/bin/env python3
import sys
import signal

running = True

def signal_handler(sig, frame):
    global running
    running = False

signal.signal(signal.SIGTERM, signal_handler)

print("[INFO] Tool started", file=sys.stderr)

while running:
    # Do work
    print("STATUS|my_tool|Working...")
    sys.stdout.flush()
    time.sleep(10)

print("[INFO] Tool stopped", file=sys.stderr)
```

### 2. Make Executable

```bash
chmod +x my_tool.py
```

### 3. Add to Config

```ini
[tool:my_tool]
command = /full/path/to/my_tool.py
autostart = yes
restart_on_crash = yes
subscribe_to = EVENT_TYPE
```

### 4. Reload Framework

```bash
kill -HUP $(cat yuki-frame.pid)
```

## Examples

The framework includes example tools:

### monitor.py
System resource monitor. Emits STATUS and ALERT events.

```bash
python tools/monitor.py
```

### echo.py
Echoes received events back. Useful for testing.

```bash
python tools/echo.py
```

### Your Tools
Add your own tools to the `tools/` directory or anywhere else.

## Command Line Options

```bash
# Start with config
yuki-frame -c yuki-frame.conf

# Enable debug mode
yuki-frame -c yuki-frame.conf -d

# Show version
yuki-frame -v

# Show help
yuki-frame -h
```

## Signals

```bash
# Graceful shutdown
kill -TERM $(cat yuki-frame.pid)

# Reload configuration
kill -HUP $(cat yuki-frame.pid)

# Dump debug state
kill -USR1 $(cat yuki-frame.pid)
```

## Troubleshooting

### Framework won't start

Check the log file:
```bash
tail -f logs/yuki-frame.log
```

Common issues:
- Config file syntax error
- Tool command path incorrect
- Log directory doesn't exist

### Tool won't start

1. Test tool manually:
   ```bash
   /path/to/tool
   ```

2. Check permissions:
   ```bash
   chmod +x /path/to/tool
   ```

3. Check logs:
   ```bash
   grep "tool_name" logs/yuki-frame.log
   ```

### Events not routing

1. Check subscription:
   ```ini
   subscribe_to = EVENT_TYPE
   ```

2. Check event format:
   ```
   EVENT_TYPE|sender|data
   ```

3. Enable debug:
   ```bash
   ./yuki-frame -c config.conf -d
   ```

## Next Steps

1. ✅ Build and run framework
2. ✅ Test example tools
3. 📖 Read TOOL_DEVELOPMENT.md
4. 🔨 Write your first tool
5. 🚀 Deploy your system

## Resources

- **TOOL_DEVELOPMENT.md** - Complete tool development guide
- **ARCHITECTURE.md** - System design details
- **CHANGELOG.md** - Version history and changes
- **examples/** - More example tools

## Support

- Check logs: `logs/yuki-frame.log`
- Enable debug: `-d` flag
- GitHub Issues: [Report problems]

## Success Checklist

- [ ] Framework builds successfully
- [ ] Framework runs without errors
- [ ] Example tools start automatically
- [ ] Can see events in logs
- [ ] Can reload configuration
- [ ] Can stop framework gracefully
- [ ] Read tool development guide
- [ ] Written first custom tool
- [ ] Tools communicate via events

## Welcome to Yuki-Frame v2.0!

Simpler. Faster. More reliable. 🚀
