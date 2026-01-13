# Tool Development Guide - Yuki-Frame v2.0

Complete guide for developing tools that work with Yuki-Frame.

## Overview

Tools are independent processes that:
1. Can be written in **any language**
2. Communicate via **stdin/stdout/stderr**
3. Emit and receive **events**
4. Are managed by the **framework**

## Quick Example

```python
#!/usr/bin/env python3
import sys, signal, time

running = True
def stop(sig, frame):
    global running
    running = False
signal.signal(signal.SIGTERM, stop)

print("[INFO] Started", file=sys.stderr)

while running:
    print("STATUS|my_tool|Running")
    sys.stdout.flush()
    time.sleep(10)

print("[INFO] Stopped", file=sys.stderr)
```

## I/O Conventions

### stdout: Events
Send events to other tools:
```
EVENT_TYPE|sender|data
```

### stderr: Logs
Send logs to framework:
```
[LEVEL] Message
```

### stdin: Receive Events
Read events from subscribed sources:
```python
for line in sys.stdin:
    event_type, sender, data = line.strip().split('|', 2)
    # Process event
```

## Complete Examples

See README.md and examples in tools/ directory.

## What Changed from v1.0

Tools work **exactly the same**!

The only difference is framework internals. Your tools don't need changes.

## Questions?

See MODULE_DEVELOPMENT.md for the complete original guide.
