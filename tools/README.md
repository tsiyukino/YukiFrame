# Example Tools for Yuki-Frame v2.0

This directory contains example tools demonstrating Yuki-Frame capabilities.

## Available Tools

### monitor.py
System resource monitor that emits STATUS and ALERT events.

**Features:**
- Monitors CPU, memory, and disk usage
- Emits STATUS events every 10 seconds
- Emits ALERT events when thresholds exceeded
- Uses psutil library (optional)

**Usage:**
```bash
python monitor.py
```

### echo.py
Simple echo tool that receives events and echoes them back.

**Features:**
- Reads events from stdin
- Logs received events
- Echoes events back with ECHO type

**Usage:**
```bash
echo "TEST|sender|data" | python echo.py
```

### alerter.py
Alert handler that processes ALERT, ERROR, and WARNING events.

**Features:**
- Subscribes to ALERT, ERROR, WARNING events
- Logs alerts with emoji indicators
- Can be extended to send notifications

**Usage:**
```bash
python alerter.py
```

## Creating Your Own Tools

See TOOL_DEVELOPMENT.md for complete guide.

Quick template:

```python
#!/usr/bin/env python3
import sys, signal

running = True
def stop(sig, frame):
    global running
    running = False
signal.signal(signal.SIGTERM, stop)

print("[INFO] Started", file=sys.stderr)

while running:
    # Your work here
    print("EVENT_TYPE|my_tool|data")
    sys.stdout.flush()

print("[INFO] Stopped", file=sys.stderr)
```

## Installing Dependencies

For monitor.py:
```bash
pip install psutil
```

Other tools have no dependencies.

## Testing Tools

Test tools independently before adding to framework:

```bash
# Run directly
./monitor.py

# Test with simulated events
echo "ALERT|test|Test message" | ./echo.py
```

## Adding to Framework

1. Make executable: `chmod +x your_tool.py`
2. Add to yuki-frame.conf:
   ```ini
   [tool:your_tool]
   command = /full/path/to/your_tool.py
   autostart = yes
   restart_on_crash = yes
   subscribe_to = EVENT_TYPES
   ```
3. Reload framework: `kill -HUP $(cat yuki-frame.pid)`
