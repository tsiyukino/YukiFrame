# Yuki-Frame Examples

This directory contains usage examples for the Yuki-Frame framework.

## Basic Tool Example

See `../tools/` directory for example tool implementations:

- `echo.py` - Simple echo tool that demonstrates event publishing
- `receiver.py` - Tool that subscribes to and processes events
- `sender.py` - Tool that sends events to other tools
- `monitor.py` - System monitoring tool example
- `alerter.py` - Alert handling tool example

## Creating Your Own Tool

```python
#!/usr/bin/env python3
import sys
import json

def main():
    # Subscribe to events
    subscribe = {"type": "subscribe", "events": ["my.event"]}
    print(json.dumps(subscribe), flush=True)
    
    # Main loop
    for line in sys.stdin:
        event = json.loads(line)
        
        # Process event
        if event.get("type") == "my.event":
            # Do something with the event
            response = {
                "type": "my.response",
                "data": {"status": "processed"}
            }
            print(json.dumps(response), flush=True)

if __name__ == "__main__":
    main()
```

## Configuration Example

```ini
[framework]
log_file = /var/log/yuki-frame/yuki-frame.log
log_level = INFO
max_tools = 100

[tool.mytool]
command = python3 /path/to/mytool.py
description = My custom tool
autostart = true
restart_on_crash = true
max_restarts = 3
subscriptions = my.event,system.startup
```

## Running Examples

```bash
# Start framework with example configuration
./yuki-frame yuki-frame.conf.example

# Start a tool manually
./tools/echo.py

# Send test event
echo '{"type":"test.event","data":{"message":"Hello"}}' | nc localhost 9999
```

## API Usage

For embedding Yuki-Frame in your application:

```c
#include <yuki_frame/framework.h>
#include <yuki_frame/tool.h>
#include <yuki_frame/event.h>

int main(int argc, char* argv[]) {
    // Initialize framework
    if (framework_init("config.conf") != FW_OK) {
        fprintf(stderr, "Failed to initialize framework\n");
        return 1;
    }
    
    // Register a tool programmatically
    tool_register("my_tool", "python3 my_tool.py");
    tool_start("my_tool");
    
    // Publish an event
    event_publish("app.started", "main", "{\"version\":\"1.0\"}");
    
    // Run event loop
    framework_run();
    
    // Cleanup
    framework_shutdown();
    return 0;
}
```

Compile with:
```bash
gcc -o myapp myapp.c -lyuki-frame -I/usr/include
```
