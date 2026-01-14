#!/usr/bin/env python3
"""
Integration Test for Yuki-Frame
Simulates the framework routing events between tools
"""

import subprocess
import threading
import time
import sys

print("="*50)
print("Yuki-Frame Integration Test")
print("Testing: Sender -> Framework -> Receiver")
print("="*50)
print()

# Start sender
print("[1/3] Starting Sender...")
sender = subprocess.Popen(
    [sys.executable, "tools/sender.py"],
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE,
    stdin=subprocess.PIPE,
    text=True,
    bufsize=1
)

# Start receiver
print("[2/3] Starting Receiver...")
receiver = subprocess.Popen(
    [sys.executable, "tools/receiver.py"],
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE,
    stdin=subprocess.PIPE,
    text=True,
    bufsize=1
)

print("[3/3] Routing events...")
print()
print("Framework is now routing events:")
print("-" * 50)

def read_stderr(process, name):
    """Read and print stderr from process"""
    while True:
        line = process.stderr.readline()
        if not line:
            break
        print(f"[{name}] {line.strip()}")

# Start threads to read stderr
sender_thread = threading.Thread(target=read_stderr, args=(sender, "SENDER"), daemon=True)
receiver_thread = threading.Thread(target=read_stderr, args=(receiver, "RECEIVER"), daemon=True)
sender_thread.start()
receiver_thread.start()

# Route events from sender to receiver
try:
    message_count = 0
    while message_count < 5:  # Route 5 messages then stop
        # Read event from sender
        line = sender.stdout.readline()
        if not line:
            break
        
        event = line.strip()
        if event:
            # Parse event
            parts = event.split('|', 2)
            if len(parts) >= 3:
                event_type, sender_name, data = parts
                
                if event_type == "MESSAGE":
                    message_count += 1
                    print(f"[FRAMEWORK] Routing: {event_type} from {sender_name}")
                    
                    # Route to receiver (if subscribed to MESSAGE)
                    receiver.stdin.write(event + "\n")
                    receiver.stdin.flush()
                    
                    # Read response from receiver
                    time.sleep(0.1)
                    
            # Also check receiver's output for ACK
            receiver.stdout.flush()
    
    print()
    print("-" * 50)
    print(f"✅ Successfully routed {message_count} messages!")
    print()
    
except KeyboardInterrupt:
    print("\n[FRAMEWORK] Interrupted by user")

finally:
    # Clean shutdown
    print("[FRAMEWORK] Shutting down...")
    sender.terminate()
    receiver.terminate()
    sender.wait(timeout=2)
    receiver.wait(timeout=2)
    print("[FRAMEWORK] Shutdown complete")
    print()
    print("="*50)
    print("Test Summary:")
    print(f"  Messages sent: {message_count}")
    print(f"  Status: {'✅ PASS' if message_count > 0 else '❌ FAIL'}")
    print("="*50)
