#!/usr/bin/env python3
"""
Alert Handler - Example for Yuki-Frame v2.0
Handles ALERT events and displays notifications
"""

import sys
import signal

running = True

def signal_handler(sig, frame):
    global running
    print("[INFO] Alerter shutting down", file=sys.stderr)
    running = False

signal.signal(signal.SIGTERM, signal_handler)

print("[INFO] Alert handler started", file=sys.stderr)

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
            
            if event_type == "ALERT":
                print(f"[WARN] 🚨 ALERT from {sender}: {data}", file=sys.stderr)
                # In real implementation: send notification, email, SMS, etc.
                
            elif event_type == "ERROR":
                print(f"[ERROR] ❌ ERROR from {sender}: {data}", file=sys.stderr)
                
            elif event_type == "WARNING":
                print(f"[WARN] ⚠️  WARNING from {sender}: {data}", file=sys.stderr)
    
    except Exception as e:
        print(f"[ERROR] Failed to process event: {e}", file=sys.stderr)

print("[INFO] Alert handler stopped", file=sys.stderr)
