# Yuki-Frame Tool Development Guide

## Table of Contents

1. [Introduction](#introduction)
2. [Quick Start](#quick-start)
3. [Tool Basics](#tool-basics)
4. [Event Protocol](#event-protocol)
5. [Language Examples](#language-examples)
6. [Best Practices](#best-practices)
7. [Debugging Tools](#debugging-tools)
8. [Advanced Topics](#advanced-topics)
9. [API Reference](#api-reference)

## Introduction

Yuki-Frame tools are standalone programs that communicate with the framework through:
- **stdin** - Receive events from framework
- **stdout** - Send events to framework
- **stderr** - Log messages (captured by framework)

Tools can be written in any language that supports standard I/O.

## Quick Start

### Minimal Tool Template (Python)

```python
#!/usr/bin/env python3
import sys
import signal

running = True

def signal_handler(sig, frame):
    global running
    running = False

signal.signal(signal.SIGTERM, signal_handler)

# Startup log
print("[INFO] My tool started", file=sys.stderr)

# Main loop
for line in sys.stdin:
    if not running:
        break
    
    # Parse event: TYPE|sender|data
    parts = line.strip().split('|', 2)
    if len(parts) >= 2:
        event_type = parts[0]
        sender = parts[1]
        data = parts[2] if len(parts) > 2 else ""
        
        # Process event
        print(f"[INFO] Received {event_type}: {data}", file=sys.stderr)
        
        # Send response
        print(f"RESPONSE|my_tool|Processed {event_type}")
        sys.stdout.flush()

# Shutdown log
print("[INFO] My tool stopped", file=sys.stderr)
```

### Add to Configuration

```ini
[tool:my_tool]
command = /path/to/my_tool.py
description = My custom tool
autostart = yes
restart_on_crash = yes
max_restarts = 3
subscribe_to = EVENT_TYPE1,EVENT_TYPE2
```

### Make Executable

```bash
chmod +x my_tool.py
```

## Tool Basics

### Tool Lifecycle

1. **Registration** - Tool added to framework config
2. **Startup** - Framework spawns tool process
3. **Running** - Tool processes events from stdin
4. **Shutdown** - Tool receives SIGTERM and exits gracefully
5. **Restart** - Framework can auto-restart on crash

### Communication Channels

#### stdin (Input)
- Framework sends events to your tool
- One event per line
- Format: `TYPE|sender|data\n`

#### stdout (Output)
- Tool sends events to framework
- Must flush after each event
- Format: `TYPE|sender|data\n`

#### stderr (Logging)
- Tool logging captured by framework
- Use format: `[LEVEL] message`
- Levels: INFO, DEBUG, WARN, ERROR

### Signal Handling

Tools must handle SIGTERM for graceful shutdown:

```python
import signal

def signal_handler(sig, frame):
    # Set flag to exit main loop
    global running
    running = False
    
signal.signal(signal.SIGTERM, signal_handler)
```

## Event Protocol

### Event Format

```
EVENT_TYPE|sender_name|event_data
```

**Components:**
- `EVENT_TYPE` - Identifies the event (e.g., ALERT, STATUS, MESSAGE)
- `sender_name` - Tool name that sent the event
- `event_data` - Optional payload (JSON, plain text, etc.)

### Example Events

```
STATUS|monitor|CPU:45% MEM:60% DISK:30%
ALERT|monitor|High CPU usage: 85%
MESSAGE|sender|Hello World
COMMAND|control|START_TOOL logger
ERROR|backup|Backup failed: disk full
```

### Subscribing to Events

In your tool's config:

```ini
subscribe_to = ALERT,ERROR,STATUS
```

Only subscribed events will be sent to your tool's stdin.

### Publishing Events

```python
def emit_event(event_type, data):
    print(f"{event_type}|my_tool|{data}")
    sys.stdout.flush()  # CRITICAL: Must flush!

# Usage
emit_event("STATUS", "System OK")
emit_event("ALERT", "Temperature high: 85°C")
```

### Event Data Formats

#### Plain Text
```
MESSAGE|sender|Simple text message
```

#### JSON (Recommended for Complex Data)
```python
import json

data = {"status": "ok", "value": 42, "timestamp": 1234567890}
emit_event("STATUS", json.dumps(data))
```

#### Structured Format
```
METRIC|monitor|cpu=45.2,mem=60.1,disk=30.5
```

## Language Examples

### Python

```python
#!/usr/bin/env python3
import sys
import signal
import time

running = True

def signal_handler(sig, frame):
    global running
    running = False

signal.signal(signal.SIGTERM, signal_handler)

print("[INFO] Tool started", file=sys.stderr)

# Option 1: Event loop (blocking)
for line in sys.stdin:
    if not running:
        break
    
    parts = line.strip().split('|', 2)
    event_type = parts[0]
    sender = parts[1]
    data = parts[2] if len(parts) > 2 else ""
    
    # Process event
    print(f"RESPONSE|tool|Handled {event_type}")
    sys.stdout.flush()

# Option 2: Periodic task with event check
import select

while running:
    # Check for events (non-blocking)
    ready = select.select([sys.stdin], [], [], 0.1)
    if ready[0]:
        line = sys.stdin.readline()
        if line:
            # Process event
            pass
    
    # Do periodic work
    print("STATUS|tool|Working...")
    sys.stdout.flush()
    time.sleep(5)

print("[INFO] Tool stopped", file=sys.stderr)
```

### Bash/Shell

```bash
#!/bin/bash

trap 'echo "[INFO] Shutting down" >&2; exit 0' SIGTERM

echo "[INFO] Shell tool started" >&2

while IFS='|' read -r event_type sender data; do
    echo "[INFO] Received $event_type: $data" >&2
    
    # Process event
    case "$event_type" in
        ALERT)
            # Handle alert
            echo "RESPONSE|shell_tool|Alert processed" ;;
        STATUS)
            # Handle status
            echo "STATUS|shell_tool|OK" ;;
    esac
done

echo "[INFO] Shell tool stopped" >&2
```

### Node.js

```javascript
#!/usr/bin/env node

const readline = require('readline');

let running = true;

process.on('SIGTERM', () => {
    console.error('[INFO] Shutting down');
    running = false;
    process.exit(0);
});

const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout,
    terminal: false
});

console.error('[INFO] Node tool started');

rl.on('line', (line) => {
    const [eventType, sender, ...dataParts] = line.split('|');
    const data = dataParts.join('|');
    
    console.error(`[INFO] Received ${eventType}: ${data}`);
    
    // Send response
    console.log(`RESPONSE|node_tool|Processed ${eventType}`);
});

rl.on('close', () => {
    console.error('[INFO] Node tool stopped');
});
```

### C/C++

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

volatile sig_atomic_t running = 1;

void signal_handler(int sig) {
    running = 0;
}

void emit_event(const char* type, const char* data) {
    printf("%s|c_tool|%s\n", type, data);
    fflush(stdout);
}

int main(void) {
    signal(SIGTERM, signal_handler);
    
    fprintf(stderr, "[INFO] C tool started\n");
    
    char line[4096];
    while (running && fgets(line, sizeof(line), stdin)) {
        char event_type[64], sender[64], data[4000];
        
        if (sscanf(line, "%63[^|]|%63[^|]|%3999[^\n]", 
                   event_type, sender, data) >= 2) {
            fprintf(stderr, "[INFO] Received %s: %s\n", event_type, data);
            
            // Process and respond
            emit_event("RESPONSE", "Processed");
        }
    }
    
    fprintf(stderr, "[INFO] C tool stopped\n");
    return 0;
}
```

## Best Practices

### 1. Always Flush stdout

```python
# BAD - Events may be buffered
print("EVENT|tool|data")

# GOOD - Event sent immediately
print("EVENT|tool|data")
sys.stdout.flush()
```

### 2. Handle SIGTERM Gracefully

```python
# Save state, close files, cleanup
def signal_handler(sig, frame):
    cleanup()
    sys.exit(0)
```

### 3. Log to stderr Only

```python
# GOOD - Framework captures this
print("[INFO] Status update", file=sys.stderr)

# BAD - Conflicts with event protocol
print("Status update")  # Goes to stdout!
```

### 4. Validate Event Data

```python
parts = line.strip().split('|', 2)
if len(parts) < 2:
    print("[WARN] Invalid event format", file=sys.stderr)
    continue
```

### 5. Use Structured Data for Complex Events

```python
import json

# Complex data
data = {
    "metric": "cpu",
    "value": 45.2,
    "unit": "percent",
    "threshold": 80,
    "timestamp": time.time()
}

emit_event("METRIC", json.dumps(data))
```

### 6. Include Error Handling

```python
try:
    # Process event
    result = process_event(event_type, data)
    emit_event("RESPONSE", f"Success: {result}")
except Exception as e:
    print(f"[ERROR] {e}", file=sys.stderr)
    emit_event("ERROR", str(e))
```

### 7. Keep Tools Focused

- One tool = one responsibility
- Don't create monolithic tools
- Use events to coordinate between tools

### 8. Test Tools Independently

```bash
# Test tool without framework
echo "TEST|sender|data" | ./my_tool.py

# Test with multiple events
cat <<EOF | ./my_tool.py
EVENT1|sender|data1
EVENT2|sender|data2
EOF
```

## Debugging Tools

### Console Logging

```python
import sys

def log(level, message):
    print(f"[{level}] {message}", file=sys.stderr)

log("DEBUG", f"Processing event: {event_type}")
log("ERROR", f"Failed to parse: {line}")
```

### Debug Mode

Check if framework debug mode is enabled:

```python
import os

debug = os.environ.get('YUKI_FRAME_DEBUG', '0') == '1'

if debug:
    log("DEBUG", f"Detailed debug info: {data}")
```

### Event Logging

```python
def log_event(direction, event_type, sender, data):
    print(f"[DEBUG] {direction} {event_type}|{sender}|{data[:50]}...", 
          file=sys.stderr)

# Usage
log_event("RECV", event_type, sender, data)
log_event("SEND", "RESPONSE", "my_tool", response_data)
```

### Testing with Pipes

```bash
# Create named pipes for testing
mkfifo /tmp/tool_in /tmp/tool_out

# Run tool
./my_tool.py < /tmp/tool_in > /tmp/tool_out &

# Send test events
echo "TEST|sender|data" > /tmp/tool_in

# Read responses
cat /tmp/tool_out
```

### Performance Profiling

```python
import time

start_time = time.time()

# Process event
process_event(data)

elapsed = time.time() - start_time
if elapsed > 1.0:
    print(f"[WARN] Slow processing: {elapsed:.2f}s", file=sys.stderr)
```

## Advanced Topics

### Non-Blocking Event Reading

```python
import select
import sys

def has_event(timeout=0):
    """Check if event available without blocking"""
    ready = select.select([sys.stdin], [], [], timeout)
    return bool(ready[0])

# Main loop with periodic work
while running:
    if has_event(timeout=0.1):
        line = sys.stdin.readline()
        process_event(line)
    else:
        # Do periodic work
        do_background_task()
```

### Event Buffering

```python
import queue
import threading

event_queue = queue.Queue()

def event_reader():
    """Read events in separate thread"""
    for line in sys.stdin:
        event_queue.put(line.strip())

# Start reader thread
thread = threading.Thread(target=event_reader, daemon=True)
thread.start()

# Process events
while running:
    try:
        line = event_queue.get(timeout=1)
        process_event(line)
    except queue.Empty:
        # Do periodic work
        pass
```

### State Management

```python
import json
import os

STATE_FILE = "/tmp/my_tool_state.json"

def save_state(state):
    with open(STATE_FILE, 'w') as f:
        json.dump(state, f)

def load_state():
    if os.path.exists(STATE_FILE):
        with open(STATE_FILE, 'r') as f:
            return json.load(f)
    return {}

# Use in signal handler
def signal_handler(sig, frame):
    save_state(current_state)
    sys.exit(0)
```

### Event Filtering

```python
# Subscribe to specific events in config
# subscribe_to = ALERT,ERROR,CRITICAL

# Additional filtering in code
def should_process(event_type, data):
    if event_type == "ALERT":
        # Only process high-priority alerts
        return "priority:high" in data
    return True

for line in sys.stdin:
    event_type, sender, data = parse_event(line)
    
    if should_process(event_type, data):
        process_event(event_type, data)
```

### Periodic Tasks

```python
import time
import threading

def periodic_task():
    """Run every N seconds"""
    while running:
        emit_event("HEARTBEAT", f"alive:{time.time()}")
        time.sleep(30)

# Start in background
thread = threading.Thread(target=periodic_task, daemon=True)
thread.start()

# Continue processing events
for line in sys.stdin:
    process_event(line)
```

## API Reference

### Event Types (Common Conventions)

| Event Type | Purpose | Example Data |
|------------|---------|--------------|
| `STATUS` | Status updates | `"CPU:45% MEM:60%"` |
| `ALERT` | High-priority alerts | `"High CPU: 85%"` |
| `WARNING` | Warnings | `"Disk space low"` |
| `ERROR` | Error conditions | `"Connection failed"` |
| `MESSAGE` | General messages | `"Hello World"` |
| `COMMAND` | Commands to other tools | `"START_TOOL logger"` |
| `RESPONSE` | Responses to events | `"Acknowledged"` |
| `METRIC` | Numerical metrics | `"cpu=45.2,mem=60.1"` |
| `LOG` | Log messages | `"Application started"` |
| `HEARTBEAT` | Keep-alive signals | `"alive:1234567890"` |

### Configuration Options

```ini
[tool:my_tool]
command = /path/to/executable       # Required: Full path to tool
description = Human-readable desc   # Optional: Tool description
autostart = yes|no                  # Start with framework?
restart_on_crash = yes|no          # Auto-restart if crashes?
max_restarts = N                   # Max restart attempts (0 = unlimited)
subscribe_to = TYPE1,TYPE2         # Event subscriptions (comma-separated)
```

### Environment Variables

Tools may receive these environment variables:

- `YUKI_FRAME_DEBUG` - "1" if debug mode enabled
- `YUKI_FRAME_LOG_LEVEL` - Current log level
- `YUKI_FRAME_VERSION` - Framework version

### Exit Codes

- `0` - Normal exit
- `1` - Error exit
- Other - Custom error codes

Framework behavior:
- Exit code 0: Normal shutdown, no restart
- Exit code != 0: Error, may restart if `restart_on_crash = yes`

### Signal Handling

Tools should handle:
- `SIGTERM` - Graceful shutdown request
- `SIGINT` - Interrupt (optional, for manual testing)

### Logging Format

Recommended stderr format:

```
[LEVEL] Component: Message
```

Examples:
```
[INFO] Database: Connected to db.example.com
[WARN] Cache: Memory usage at 85%
[ERROR] Network: Connection timeout after 30s
[DEBUG] Parser: Received 1234 bytes
```

## Tool Development Workflow

### 1. Plan Your Tool

- What events will it subscribe to?
- What events will it emit?
- What external resources needed?
- How to handle errors?

### 2. Create Minimal Version

Start with template, add minimal functionality:

```python
#!/usr/bin/env python3
import sys
import signal

running = True
signal.signal(signal.SIGTERM, lambda s,f: sys.exit(0))

print("[INFO] Started", file=sys.stderr)

for line in sys.stdin:
    parts = line.strip().split('|', 2)
    print(f"[INFO] Got: {parts[0]}", file=sys.stderr)

print("[INFO] Stopped", file=sys.stderr)
```

### 3. Test Independently

```bash
chmod +x my_tool.py

# Test with echo
echo "TEST|sender|data" | ./my_tool.py

# Test signal handling
./my_tool.py &
PID=$!
kill -TERM $PID
```

### 4. Add to Framework

```ini
[tool:my_tool]
command = /full/path/to/my_tool.py
autostart = no  # Test manually first
```

```bash
# Start framework
./yuki-frame -c config.conf

# Monitor logs
tail -f logs/yuki-frame.log
```

### 5. Iterate and Improve

- Add error handling
- Implement actual functionality
- Test edge cases
- Enable autostart
- Add restart_on_crash

### 6. Production Deployment

- Set appropriate log levels
- Configure max_restarts
- Monitor resource usage
- Set up alerts for failures

## Examples

See `tools/` directory for complete examples:

- `monitor.py` - System monitoring with periodic events
- `echo.py` - Simple event echo tool
- `alerter.py` - Alert processing and notifications
- `sender.py` - Event publisher example
- `receiver.py` - Event subscriber example

## Troubleshooting

### Tool Not Starting

1. Check executable permissions: `chmod +x tool.py`
2. Verify command path in config is absolute
3. Check framework logs: `tail -f logs/yuki-frame.log`
4. Test tool manually: `./tool.py`

### Events Not Received

1. Check `subscribe_to` in config
2. Verify event format: `TYPE|sender|data`
3. Enable debug mode: `./yuki-frame -d`
4. Check tool is running: framework logs

### Events Not Sent

1. Flush stdout after each event
2. Verify event format correct
3. Check for stderr/stdout confusion
4. Test with echo: `echo "TEST|s|d" | ./tool.py`

### Tool Crashes

1. Add error handling in tool code
2. Enable `restart_on_crash`
3. Check tool logs in framework log
4. Set `max_restarts` to limit restart loops

### Performance Issues

1. Avoid blocking operations
2. Use non-blocking event reading
3. Profile critical sections
4. Check resource usage (CPU, memory)

## Resources

- **Main Documentation**: README.md
- **Framework Development**: DEVELOPMENT.md
- **Testing Guide**: TESTING.md
- **Change History**: CHANGELOG.md
- **Example Tools**: tools/ directory

## Contributing Tools

If you've created a useful tool for Yuki-Frame:

1. Ensure it follows best practices
2. Include documentation
3. Add example configuration
4. Test thoroughly
5. Submit pull request with tool in `tools/` directory

---

**Questions or Issues?**

Check the main documentation or open an issue on GitHub.
