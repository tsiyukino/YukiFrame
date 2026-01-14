#!/usr/bin/env python3
"""
Receiver Tool - Receives and prints messages from Yuki-Frame
"""

import sys
import signal

running = True

def signal_handler(sig, frame):
    """Handle SIGTERM for graceful shutdown"""
    global running
    print("[INFO] Receiver shutting down", file=sys.stderr)
    running = False

# Setup signal handler
signal.signal(signal.SIGTERM, signal_handler)

def main():
    print("[INFO] Receiver tool started", file=sys.stderr)
    print("[INFO] Waiting for MESSAGE events...", file=sys.stderr)
    
    # Read events from stdin
    for line in sys.stdin:
        if not running:
            break
        
        try:
            # Parse event: EVENT_TYPE|sender|data
            parts = line.strip().split('|', 2)
            
            if len(parts) >= 3:
                event_type = parts[0]
                sender = parts[1]
                data = parts[2]
                
                if event_type == "MESSAGE":
                    # Print the received message
                    print(f"[INFO] ✅ RECEIVED MESSAGE from {sender}: '{data}'", file=sys.stderr)
                    
                    # Optionally send acknowledgment back
                    print(f"ACK|receiver|Received: {data}")
                    sys.stdout.flush()
                else:
                    print(f"[DEBUG] Received event: {event_type} from {sender}", file=sys.stderr)
            
        except Exception as e:
            print(f"[ERROR] Failed to process event: {e}", file=sys.stderr)
    
    print("[INFO] Receiver stopped", file=sys.stderr)

if __name__ == "__main__":
    main()
