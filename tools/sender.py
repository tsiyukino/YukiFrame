#!/usr/bin/env python3
"""
Sender Tool - Sends "Hello World" message through Yuki-Frame
"""

import sys
import time
import signal

running = True

def signal_handler(sig, frame):
    """Handle SIGTERM for graceful shutdown"""
    global running
    print("[INFO] Sender shutting down", file=sys.stderr)
    running = False

# Setup signal handler
signal.signal(signal.SIGTERM, signal_handler)

def emit_event(event_type, data):
    """Emit an event to the framework"""
    print(f"{event_type}|sender|{data}")
    sys.stdout.flush()

def main():
    print("[INFO] Sender tool started", file=sys.stderr)
    
    # Wait a bit for framework to be ready
    time.sleep(1)
    
    # Send "Hello World" message
    print("[INFO] Sending 'Hello World' message...", file=sys.stderr)
    emit_event("MESSAGE", "Hello World")
    
    # Keep sending every 5 seconds
    count = 1
    while running:
        time.sleep(5)
        count += 1
        message = f"Hello World #{count}"
        print(f"[INFO] Sending: {message}", file=sys.stderr)
        emit_event("MESSAGE", message)
    
    print("[INFO] Sender stopped", file=sys.stderr)

if __name__ == "__main__":
    main()
