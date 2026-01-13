#!/usr/bin/env python3
"""
System Monitor Tool - Example for Yuki-Frame v2.0
Monitors system resources and emits status events
"""

import sys
import time
import signal

# Global state
running = True

def signal_handler(sig, frame):
    """Handle SIGTERM for graceful shutdown"""
    global running
    print("[INFO] Received shutdown signal", file=sys.stderr)
    running = False

# Setup signal handler
signal.signal(signal.SIGTERM, signal_handler)

def emit_event(event_type, data):
    """Emit an event to the framework"""
    print(f"{event_type}|monitor|{data}")
    sys.stdout.flush()

def get_system_stats():
    """Get system statistics (simplified)"""
    try:
        import psutil
        cpu = psutil.cpu_percent(interval=1)
        mem = psutil.virtual_memory().percent
        disk = psutil.disk_usage('/').percent
        return {"cpu": cpu, "mem": mem, "disk": disk}
    except ImportError:
        # Fallback if psutil not available
        return {"cpu": 0, "mem": 0, "disk": 0}

def main():
    print("[INFO] System monitor started", file=sys.stderr)
    
    while running:
        try:
            # Get system stats
            stats = get_system_stats()
            
            # Emit status event
            emit_event("STATUS", f"CPU:{stats['cpu']}% MEM:{stats['mem']}% DISK:{stats['disk']}%")
            
            # Check for alerts
            if stats['cpu'] > 80:
                emit_event("ALERT", f"High CPU usage: {stats['cpu']}%")
            if stats['mem'] > 80:
                emit_event("ALERT", f"High memory usage: {stats['mem']}%")
            if stats['disk'] > 90:
                emit_event("ALERT", f"High disk usage: {stats['disk']}%")
            
            # Sleep
            time.sleep(10)
            
        except Exception as e:
            print(f"[ERROR] Monitor error: {e}", file=sys.stderr)
            time.sleep(5)
    
    print("[INFO] System monitor stopped", file=sys.stderr)

if __name__ == "__main__":
    main()
