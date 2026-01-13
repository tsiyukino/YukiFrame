#!/usr/bin/env python3
"""
Echo Tool - Example for Yuki-Frame v2.0
Echoes received events back
"""

import sys
import signal

running = True

def signal_handler(sig, frame):
    global running
    running = False

signal.signal(signal.SIGTERM, signal_handler)

print("[INFO] Echo tool started", file=sys.stderr)

# Read events from stdin
for line in sys.stdin:
    if not running:
        break
    
    try:
        parts = line.strip().split('|', 2)
        if len(parts) >= 2:
            event_type = parts[0]
            sender = parts[1]
            data = parts[2] if len(parts) > 2 else ""
            
            print(f"[INFO] Received {event_type} from {sender}: {data}", file=sys.stderr)
            
            # Echo back
            print(f"ECHO|echo|{event_type} from {sender}")
            sys.stdout.flush()
    except Exception as e:
        print(f"[ERROR] {e}", file=sys.stderr)

print("[INFO] Echo tool stopped", file=sys.stderr)
