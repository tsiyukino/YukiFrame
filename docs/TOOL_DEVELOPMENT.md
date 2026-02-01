# Tool Development Guide for Yuki-Frame

## Overview

All tools in Yuki-Frame communicate with the framework through **stdin/stdout pipes**. This document describes the protocol and how to create your own tools.

---

## Tool Protocol

### Communication Model

```
Framework ←─ stdout ─→ Tool
          ←─ stderr ─→ (logs only)
          ─→ stdin  ─→
```

- **Tool stdout** → Framework reads events and control messages
- **Tool stdin** → Framework sends events the tool subscribed to
- **Tool stderr** → Framework logs (appears in framework logs with `[toolname]` prefix)

---

## Message Format

All messages use pipe-delimited format:

```
TYPE|sender|data
```

**Example:**
```
PING|ping_tool|Ping #1 at 14:30:00
```

---

## Control Messages (Tool → Framework)

These are special messages that configure tool behavior:

### 1. TOOL_READY

**Format:** `TOOL_READY|toolname|description`

**Purpose:** Signals that the tool has started successfully

**Example:**
```python
print("TOOL_READY|my_tool|My tool started", flush=True)
```

**Required:** Recommended but not required

---

### 2. SUBSCRIBE

**Format:** `SUBSCRIBE|toolname|event_type`

**Purpose:** Register to receive events of a specific type

**Example:**
```python
# Subscribe to PING events
print("SUBSCRIBE|ping|PING", flush=True)

# Subscribe to multiple event types (send multiple SUBSCRIBE messages)
print("SUBSCRIBE|monitor|PING", flush=True)
print("SUBSCRIBE|monitor|PONG", flush=True)
print("SUBSCRIBE|monitor|ERROR", flush=True)

# Subscribe to ALL events
print("SUBSCRIBE|logger|*", flush=True)
```

**Important:**
- Must be sent **before** the tool can receive events
- Can send multiple SUBSCRIBE messages for different event types
- Use `*` to subscribe to all events

---

### 3. COMMAND (Special)

**Format:** `COMMAND|toolname|command_text`

**Purpose:** Send control command to framework (used by console tools)

**Example:**
```python
print("COMMAND|console|list", flush=True)
print("COMMAND|console|start fetcher", flush=True)
```

**Note:** Only use this if you're building a console-like tool

---

## Events (Tool → Framework & Framework → Tool)

Regular events that get routed to subscribed tools:

**Format:** `EVENT_TYPE|sender|data`

**Example - Sending:**
```python
# Tool sends PING event
print("PING|ping_tool|Ping #1", flush=True)
```

**Example - Receiving:**
```python
# Tool receives events via stdin
line = sys.stdin.readline().strip()

# Parse: TYPE|sender|data
parts = line.split('|', 2)
if len(parts) == 3:
    event_type, sender, data = parts
    print(f"Received {event_type} from {sender}: {data}", file=sys.stderr)
```

---

## Tool Lifecycle

### Startup Sequence

```python
#!/usr/bin/env python3
import sys

# 1. Send TOOL_READY (optional but recommended)
print("TOOL_READY|my_tool|My tool v1.0", flush=True)

# 2. Subscribe to events you want to receive
print("SUBSCRIBE|my_tool|PING", flush=True)
print("SUBSCRIBE|my_tool|DATA_REQUEST", flush=True)

# 3. Enter main loop
while True:
    line = sys.stdin.readline()
    if not line:
        break
    
    # Process events
    # ...
```

### Event Processing Loop

```python
def main():
    print("TOOL_READY|processor|Event processor started", flush=True)
    print("SUBSCRIBE|processor|PROCESS", flush=True)
    
    while True:
        try:
            # Read event from framework
            line = sys.stdin.readline()
            if not line:
                break
            
            line = line.strip()
            if not line:
                continue
            
            # Parse: TYPE|sender|data
            parts = line.split('|', 2)
            if len(parts) < 3:
                continue
            
            event_type, sender, data = parts
            
            # Handle event
            if event_type == "PROCESS":
                result = process_data(data)
                
                # Send result event
                print(f"RESULT|processor|{result}", flush=True)
        
        except KeyboardInterrupt:
            break
        except Exception as e:
            print(f"Error: {e}", file=sys.stderr, flush=True)
    
    print("Shutting down", file=sys.stderr, flush=True)

if __name__ == '__main__':
    main()
```

---

## Tool Patterns

### 1. Responder Pattern

Tool that responds to specific events:

```python
#!/usr/bin/env python3
"""Echo tool - responds to ECHO events"""
import sys

print("TOOL_READY|echo|Echo responder", flush=True)
print("SUBSCRIBE|echo|ECHO", flush=True)

while True:
    line = sys.stdin.readline()
    if not line:
        break
    
    parts = line.strip().split('|', 2)
    if len(parts) == 3:
        event_type, sender, data = parts
        
        if event_type == "ECHO":
            # Send response
            print(f"ECHO_RESPONSE|echo|{data}", flush=True)
            print(f"Echoed: {data}", file=sys.stderr, flush=True)
```

---

### 2. Generator Pattern

Tool that generates events periodically:

```python
#!/usr/bin/env python3
"""Monitor - generates status events"""
import sys
import time
import threading

def monitor_loop():
    """Generate events periodically"""
    count = 0
    while True:
        count += 1
        
        # Generate event
        status = f"cpu=10% mem=50% count={count}"
        print(f"STATUS|monitor|{status}", flush=True)
        
        time.sleep(10)  # Every 10 seconds

def main():
    print("TOOL_READY|monitor|System monitor", flush=True)
    
    # Start monitoring thread
    thread = threading.Thread(target=monitor_loop, daemon=True)
    thread.start()
    
    # Keep main thread alive
    while True:
        time.sleep(1)

if __name__ == '__main__':
    main()
```

---

### 3. Single-Use Pattern

Tool that runs once and exits:

```python
#!/usr/bin/env python3
"""Fetcher - single-use data fetcher"""
import sys
import time

print("TOOL_READY|fetcher|Data fetcher", flush=True)
print("SUBSCRIBE|fetcher|FETCH_DATA", flush=True)

# Wait for FETCH_DATA event (with timeout)
timeout = 30
start = time.time()

while time.time() - start < timeout:
    line = sys.stdin.readline()
    if not line:
        time.sleep(0.1)
        continue
    
    parts = line.strip().split('|', 2)
    if len(parts) == 3:
        event_type, sender, data = parts
        
        if event_type == "FETCH_DATA":
            # Fetch data
            result = fetch_from_api(data)
            
            # Publish result
            print(f"DATA_RESULT|fetcher|{result}", flush=True)
            
            # Exit after completing task
            print("Task complete, exiting", file=sys.stderr, flush=True)
            sys.exit(0)

# Timeout - no request received
print("Timeout, exiting", file=sys.stderr, flush=True)
sys.exit(0)
```

---

### 4. Logger Pattern

Tool that logs all events:

```python
#!/usr/bin/env python3
"""Logger - logs all events to file"""
import sys
import time

print("TOOL_READY|logger|Event logger", flush=True)
print("SUBSCRIBE|logger|*", flush=True)  # Subscribe to ALL events

with open("events.log", "a") as f:
    while True:
        line = sys.stdin.readline()
        if not line:
            break
        
        parts = line.strip().split('|', 2)
        if len(parts) == 3:
            event_type, sender, data = parts
            
            # Log to file
            timestamp = time.strftime('%Y-%m-%d %H:%M:%S')
            f.write(f"[{timestamp}] {event_type} from {sender}: {data}\n")
            f.flush()
```

---

## Important Rules

### 1. Always flush stdout

```python
# WRONG
print("EVENT|tool|data")

# CORRECT
print("EVENT|tool|data", flush=True)
```

**Why:** Python buffers stdout by default. Without flush, events may be delayed or lost.

---

### 2. Use stderr for logging

```python
# Event - goes to framework
print("PING|mytool|data", flush=True)

# Log - appears in framework logs as [mytool]
print("Processing data...", file=sys.stderr, flush=True)
```

**Why:** Framework reads stdout for events, stderr for logs.

---

### 3. Subscribe before using events

```python
# WRONG - will never receive PING events
print("TOOL_READY|mytool|Started", flush=True)
# (missing SUBSCRIBE)
while True:
    line = sys.stdin.readline()  # Will never get PING events!

# CORRECT
print("TOOL_READY|mytool|Started", flush=True)
print("SUBSCRIBE|mytool|PING", flush=True)  # ← Subscribe first!
while True:
    line = sys.stdin.readline()  # Now will receive PING events
```

---

### 4. Tool name must match everywhere

```python
# The tool name MUST be consistent:
print("TOOL_READY|mytool|Started", flush=True)     # mytool
print("SUBSCRIBE|mytool|PING", flush=True)         # mytool
print("RESPONSE|mytool|Result", flush=True)        # mytool
```

---

## Configuration

Tools are registered in `yuki-frame.conf`:

```ini
[tool:mytool]
command = python tools\mytool.py
autostart = yes
restart_on_crash = yes
restart_policy = always
restart_max_delay_sec = 5
subscriptions = PING, PONG

# Note: 'subscriptions' in config is OPTIONAL
# Tools can dynamically subscribe via SUBSCRIBE messages
```

**Important:** The `subscriptions` field in config is **for reference only** and currently not used by the framework. Tools **must** send `SUBSCRIBE` messages at runtime.

---

## Complete Example

```python
#!/usr/bin/env python3
"""
Complete example tool - ping/pong responder
"""
import sys
import time

def main():
    # 1. Announce we're ready
    print("TOOL_READY|pong|Pong responder v1.0", flush=True)
    
    # 2. Subscribe to events we want to receive
    print("SUBSCRIBE|pong|PING", flush=True)
    
    # Log to stderr (appears in framework logs)
    print("Started, waiting for PING events", file=sys.stderr, flush=True)
    
    pong_count = 0
    
    # 3. Main event loop
    while True:
        try:
            # Read from stdin (framework sends events here)
            line = sys.stdin.readline()
            
            if not line:
                # EOF - framework is shutting down
                break
            
            line = line.strip()
            if not line:
                continue
            
            # Parse: TYPE|sender|data
            parts = line.split('|', 2)
            if len(parts) < 3:
                continue
            
            event_type, sender, data = parts
            
            # Handle PING events
            if event_type == "PING":
                pong_count += 1
                
                # Send PONG response
                response = f"PONG|pong|Pong #{pong_count} to: {data}"
                print(response, flush=True)
                
                # Log (appears as [pong] in framework logs)
                print(f"Sent PONG #{pong_count}", file=sys.stderr, flush=True)
        
        except KeyboardInterrupt:
            break
        except Exception as e:
            print(f"Error: {e}", file=sys.stderr, flush=True)
    
    # Clean shutdown
    print(f"Shutting down after {pong_count} pongs", file=sys.stderr, flush=True)

if __name__ == '__main__':
    main()
```

---

## Testing Your Tool

### Manual Test

```cmd
REM 1. Test tool standalone (pipe simulator)
echo PING^|test^|Hello | python tools\mytool.py

REM Should output:
TOOL_READY|mytool|...
SUBSCRIBE|mytool|PING
PONG|mytool|Response

REM 2. Test in framework
yuki-frame.exe -c test.conf -d
```

### Debug Configuration

```ini
# test.conf - minimal config for testing
[core]
log_file = test.log
log_level = DEBUG
enable_debug = yes

[tool:mytool]
command = python tools\mytool.py
autostart = yes
restart_on_crash = yes
```

---

## Common Mistakes

### ❌ Forgetting flush

```python
print("EVENT|tool|data")  # May be buffered!
```

**Fix:**
```python
print("EVENT|tool|data", flush=True)  # ✅
```

---

### ❌ Not subscribing

```python
# Tool never receives events - no SUBSCRIBE!
while True:
    line = sys.stdin.readline()
```

**Fix:**
```python
print("SUBSCRIBE|mytool|PING", flush=True)  # ✅
while True:
    line = sys.stdin.readline()
```

---

### ❌ Wrong message format

```python
print("PING from mytool: Hello")  # Wrong format!
```

**Fix:**
```python
print("PING|mytool|Hello", flush=True)  # ✅ TYPE|sender|data
```

---

### ❌ Using print for events AND logs

```python
print("Processing...")  # Wrong - goes to stdout as event!
```

**Fix:**
```python
print("Processing...", file=sys.stderr, flush=True)  # ✅ Logs to stderr
```

---

## Summary

**Tool Protocol Checklist:**
- ✅ Send `TOOL_READY` on startup (optional)
- ✅ Send `SUBSCRIBE` for each event type you want to receive
- ✅ Always `flush=True` on stdout prints
- ✅ Use stderr for logging
- ✅ Parse stdin events: `TYPE|sender|data`
- ✅ Send events with same format: `TYPE|sender|data`
- ✅ Handle EOF (stdin closed) gracefully

**Control Messages (stdout):**
- `TOOL_READY|name|description` - Tool startup
- `SUBSCRIBE|name|event_type` - Register for events
- `COMMAND|name|command` - Send framework command (console only)

**Regular Events (stdout/stdin):**
- `TYPE|sender|data` - Any other message

**Logs (stderr):**
- Any text - appears in framework logs with `[toolname]` prefix
