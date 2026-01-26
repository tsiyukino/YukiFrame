#!/usr/bin/env python3
"""
Yuki-Frame Console Client (External Tool)

This is NOT part of the framework - it's a separate tool that CONNECTS to
the framework's integrated control socket server.

Architecture:
- Framework runs: yuki-frame.exe (includes socket server)
- Console connects: python yuki-console.py
- Communication: TCP socket (localhost:9999)
"""

import socket
import sys
import argparse

class YukiConsole:
    """Console client that connects to Yuki-Frame control socket"""
    
    def __init__(self, host='localhost', port=9999):
        self.host = host
        self.port = port
        self.sock = None
        self.connected = False
    
    def connect(self):
        """Connect to framework's control socket"""
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect((self.host, self.port))
            self.connected = True
            return True
        except ConnectionRefusedError:
            print(f"Error: Could not connect to {self.host}:{self.port}")
            print("Is the framework running? (yuki-frame.exe)")
            return False
        except Exception as e:
            print(f"Error connecting: {e}")
            return False
    
    def disconnect(self):
        """Disconnect from framework"""
        if self.sock:
            try:
                self.sock.close()
            except:
                pass
            self.connected = False
    
    def send_command(self, command):
        """Send command to framework and receive response"""
        if not self.connected:
            return "Error: Not connected to framework"
        
        try:
            # Send command
            self.sock.sendall(command.encode() + b'\n')
            
            # Receive response
            response = b''
            self.sock.settimeout(5.0)  # 5 second timeout
            
            while True:
                try:
                    chunk = self.sock.recv(4096)
                    if not chunk:
                        # Connection closed
                        self.connected = False
                        return "Error: Connection closed by framework"
                    
                    response += chunk
                    
                    # Simple heuristic: if we got data and haven't received
                    # anything for a brief moment, assume response is complete
                    # Better approach: look for specific end marker or use length prefix
                    if len(response) > 0:
                        # Set very short timeout to check if more data is coming
                        self.sock.settimeout(0.1)
                        try:
                            extra = self.sock.recv(4096)
                            if extra:
                                response += extra
                            else:
                                break
                        except socket.timeout:
                            # No more data, response is complete
                            break
                except socket.timeout:
                    # Timeout - response is complete
                    break
            
            # Reset timeout
            self.sock.settimeout(5.0)
            
            return response.decode('utf-8', errors='replace')
        
        except ConnectionResetError:
            self.connected = False
            return "Error: Connection lost. Framework may have shut down."
        except Exception as e:
            return f"Error: {e}"
    
    def run_interactive(self):
        """Run interactive console loop"""
        print("=" * 60)
        print(f"  Yuki-Frame Console v2.0")
        print(f"  Connected to {self.host}:{self.port}")
        print(f"  Type 'help' for commands, 'quit' to exit")
        print("=" * 60)
        print()
        
        while True:
            try:
                # Prompt
                command = input("yuki> ").strip()
                
                if not command:
                    continue
                
                # Local commands
                if command in ['quit', 'exit']:
                    print("Exiting console (framework continues running)...")
                    break
                
                # Send to framework
                response = self.send_command(command)
                
                # Display response
                if response:
                    print(response, end='')
                    if not response.endswith('\n'):
                        print()
                
                # Check if framework shut down
                if 'Shutting down framework' in response:
                    print("Framework is shutting down...")
                    break
            
            except KeyboardInterrupt:
                print("\nUse 'quit' to exit console")
                continue
            except EOFError:
                print("\nExiting console...")
                break
            except Exception as e:
                print(f"Error: {e}")
                break
    
    def run_single_command(self, command):
        """Execute a single command and exit"""
        response = self.send_command(command)
        print(response, end='')
        if not response.endswith('\n'):
            print()

def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description='Yuki-Frame Console Client',
        epilog='This console connects to the framework\'s control socket.'
    )
    
    parser.add_argument(
        '-H', '--host',
        default='localhost',
        help='Framework host (default: localhost)'
    )
    
    parser.add_argument(
        '-p', '--port',
        type=int,
        default=9999,
        help='Framework control port (default: 9999)'
    )
    
    parser.add_argument(
        '-c', '--command',
        help='Execute single command and exit'
    )
    
    args = parser.parse_args()
    
    # Create console
    console = YukiConsole(host=args.host, port=args.port)
    
    # Connect to framework
    if not console.connect():
        return 1
    
    try:
        if args.command:
            # Single command mode
            console.run_single_command(args.command)
        else:
            # Interactive mode
            console.run_interactive()
    finally:
        console.disconnect()
    
    return 0

if __name__ == '__main__':
    sys.exit(main())
