# Testing Yuki-Frame Communication

## Overview

Two Python tools have been created to test event communication:
- **sender.py** - Sends "Hello World" messages
- **receiver.py** - Receives and prints messages

## Test Files Created

```
yuki-frame-restructured/
├── tools/
│   ├── sender.py              ← Sends MESSAGE events
│   └── receiver.py            ← Receives MESSAGE events
├── test-tools.conf            ← Configuration for both tools
├── test-integration.py        ← Python-based integration test
├── test-manual.bat            ← Manual testing script
└── test-standalone.bat        ← Standalone tool test
```

## Quick Test (Python Integration Test)

This simulates how the framework routes events:

```cmd
python test-integration.py
```

**Expected output:**
```
==================================================
Yuki-Frame Integration Test
Testing: Sender -> Framework -> Receiver
==================================================

[1/3] Starting Sender...
[2/3] Starting Receiver...
[3/3] Routing events...

Framework is now routing events:
--------------------------------------------------
[SENDER] [INFO] Sender tool started
[RECEIVER] [INFO] Receiver tool started
[RECEIVER] [INFO] Waiting for MESSAGE events...
[SENDER] [INFO] Sending 'Hello World' message...
[FRAMEWORK] Routing: MESSAGE from sender
[RECEIVER] [INFO] ✅ RECEIVED MESSAGE from sender: 'Hello World'
[SENDER] [INFO] Sending: Hello World #2
[FRAMEWORK] Routing: MESSAGE from sender
[RECEIVER] [INFO] ✅ RECEIVED MESSAGE from sender: 'Hello World #2'
...

✅ Successfully routed 5 messages!
```

## Manual Tests

### Test 1: Sender Only

```cmd
cd tools
python sender.py
```

**Output:**
```
[INFO] Sender tool started
[INFO] Sending 'Hello World' message...
MESSAGE|sender|Hello World
[INFO] Sending: Hello World #2
MESSAGE|sender|Hello World #2
...
```

Press Ctrl+C to stop.

### Test 2: Receiver Only

```cmd
cd tools
python receiver.py
```

Then type:
```
MESSAGE|sender|Hello World
```

**Output:**
```
[INFO] Receiver tool started
[INFO] Waiting for MESSAGE events...
[INFO] ✅ RECEIVED MESSAGE from sender: 'Hello World'
ACK|receiver|Received: Hello World
```

Press Ctrl+C to stop.

### Test 3: Manual Pipe Test

```cmd
cd tools
echo MESSAGE^|sender^|Hello World | python receiver.py
```

**Output:**
```
[INFO] Receiver tool started
[INFO] Waiting for MESSAGE events...
[INFO] ✅ RECEIVED MESSAGE from sender: 'Hello World'
ACK|receiver|Received: Hello World
```

## What Works Now

✅ **sender.py**
- Starts successfully
- Sends MESSAGE events to stdout
- Event format: `MESSAGE|sender|Hello World`
- Continues sending every 5 seconds
- Handles SIGTERM gracefully

✅ **receiver.py**
- Starts successfully
- Reads events from stdin
- Parses event format correctly
- Prints received messages
- Sends ACK events back
- Handles SIGTERM gracefully

✅ **test-integration.py**
- Spawns both tools
- Routes events between them
- Demonstrates full communication
- Verifies message delivery

## What Needs Implementation

⚠️ **Framework Integration**

To make this work with the actual framework, implement:

### 1. Process Spawning (platform_windows.c)

```c
ProcessHandle platform_spawn_process(const char* command, 
    int* stdin_fd, int* stdout_fd, int* stderr_fd) {
    // Use CreateProcess + CreatePipe
    // See TESTING.md for example
}
```

### 2. Event Routing (event.c)

```c
void event_process_queue(void) {
    // For each event in queue:
    //   1. Find subscribed tools
    //   2. Write event to tool's stdin
    //   3. Read tool's stdout for new events
}
```

### 3. Tool Integration (tool.c)

```c
int tool_start(const char* name) {
    // Call platform_spawn_process
    // Store file descriptors
    // Add to event loop
}
```

## Integration Test Output Explained

```
[SENDER] [INFO] Sender tool started
```
Sender process spawned successfully

```
[RECEIVER] [INFO] Receiver tool started
```
Receiver process spawned successfully

```
[FRAMEWORK] Routing: MESSAGE from sender
```
Framework reads MESSAGE event from sender's stdout

```
[RECEIVER] [INFO] ✅ RECEIVED MESSAGE from sender: 'Hello World'
```
Framework writes event to receiver's stdin, receiver processes it

```
ACK|receiver|Received: Hello World
```
Receiver sends acknowledgment back to framework

## Configuration

**test-tools.conf:**
```ini
[tool:sender]
command = python tools/sender.py
autostart = yes
subscribe_to = ACK            # Sender subscribes to ACK events

[tool:receiver]
command = python tools/receiver.py
autostart = yes
subscribe_to = MESSAGE        # Receiver subscribes to MESSAGE events
```

## Verifying Tools Work

### Sender Verification

```cmd
python tools/sender.py
```

✅ Should output:
- `[INFO] Sender tool started` to stderr
- `MESSAGE|sender|Hello World` to stdout
- New message every 5 seconds

### Receiver Verification

```cmd
echo MESSAGE^|sender^|Test | python tools/receiver.py
```

✅ Should output:
- `[INFO] Receiver tool started` to stderr
- `[INFO] ✅ RECEIVED MESSAGE from sender: 'Test'` to stderr
- `ACK|receiver|Received: Test` to stdout

## Event Flow

```
1. Sender runs:
   └─> Writes to stdout: "MESSAGE|sender|Hello World"

2. Framework reads sender's stdout
   └─> Parses event: type=MESSAGE, sender=sender, data=Hello World

3. Framework checks subscriptions
   └─> Receiver subscribed to MESSAGE

4. Framework writes to receiver's stdin
   └─> "MESSAGE|sender|Hello World\n"

5. Receiver reads from stdin
   └─> Prints: "✅ RECEIVED MESSAGE from sender: 'Hello World'"
   └─> Writes to stdout: "ACK|receiver|Received: Hello World"

6. Framework reads receiver's stdout
   └─> Routes ACK event to subscribers (sender subscribed to ACK)
```

## Success Criteria

✅ **Tools work standalone** - Can run sender.py and receiver.py independently
✅ **Event format correct** - `TYPE|sender|data` format works
✅ **Integration test works** - Python test routes events successfully
⚠️ **Framework integration** - Needs process spawning implementation

## Next Steps

1. ✅ Tools created and tested
2. ✅ Integration test created
3. ✅ Configuration created
4. ⚠️ Implement process spawning in platform_windows.c
5. ⚠️ Implement event routing in event.c
6. ⚠️ Test with actual framework

## Quick Commands Reference

```cmd
# Test integration (simulated framework)
python test-integration.py

# Test sender only
python tools/sender.py

# Test receiver only (type events manually)
python tools/receiver.py

# Pipe test
echo MESSAGE^|sender^|Test | python tools/receiver.py

# Run framework with test config (once implemented)
build\Release\yuki-frame.exe -c test-tools.conf
```

## Expected Final Result

Once process spawning is implemented:

```cmd
build\Release\yuki-frame.exe -c test-tools.conf
```

**Should show in logs:**
```
[INFO] [tool] Starting tool: sender
[INFO] [tool] Starting tool: receiver  
[INFO] [sender] Sender tool started
[INFO] [receiver] Receiver tool started
[INFO] [event] Published event: MESSAGE from sender
[INFO] [event] Routing MESSAGE to receiver
[INFO] [receiver] ✅ RECEIVED MESSAGE from sender: 'Hello World'
[INFO] [event] Published event: ACK from receiver
[INFO] [event] Routing ACK to sender
```

## Summary

- ✅ **sender.py**: Complete and working
- ✅ **receiver.py**: Complete and working
- ✅ **test-integration.py**: Complete and working
- ✅ **Configuration**: Complete
- ⚠️ **Framework**: Needs process spawning + event routing

**The tools are ready! They're waiting for the framework to spawn and connect them.** 🚀
