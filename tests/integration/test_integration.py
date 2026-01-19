#!/usr/bin/env python3
"""
Integration test for Yuki-Frame
Tests the full framework workflow
"""

import subprocess
import time
import json
import sys
import os

class Colors:
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    END = '\033[0m'

def print_test(msg):
    print(f"{Colors.BLUE}[TEST]{Colors.END} {msg}")

def print_pass(msg):
    print(f"{Colors.GREEN}[PASS]{Colors.END} {msg}")

def print_fail(msg):
    print(f"{Colors.RED}[FAIL]{Colors.END} {msg}")

def print_info(msg):
    print(f"{Colors.YELLOW}[INFO]{Colors.END} {msg}")

class IntegrationTest:
    def __init__(self):
        self.tests_run = 0
        self.tests_passed = 0
        self.tests_failed = 0
        
    def test_framework_version(self, executable):
        """Test --version flag"""
        print_test("Testing version flag")
        self.tests_run += 1
        
        try:
            result = subprocess.run(
                [executable, '--version'],
                capture_output=True,
                text=True,
                timeout=5
            )
            
            if result.returncode == 0 and 'Yuki-Frame' in result.stdout:
                print_pass("Version flag works")
                print_info(f"Output: {result.stdout.strip()}")
                self.tests_passed += 1
                return True
            else:
                print_fail("Version flag failed")
                self.tests_failed += 1
                return False
                
        except Exception as e:
            print_fail(f"Exception: {e}")
            self.tests_failed += 1
            return False
    
    def test_framework_help(self, executable):
        """Test --help flag"""
        print_test("Testing help flag")
        self.tests_run += 1
        
        try:
            result = subprocess.run(
                [executable, '--help'],
                capture_output=True,
                text=True,
                timeout=5
            )
            
            if result.returncode == 0 and 'Usage:' in result.stdout:
                print_pass("Help flag works")
                self.tests_passed += 1
                return True
            else:
                print_fail("Help flag failed")
                self.tests_failed += 1
                return False
                
        except Exception as e:
            print_fail(f"Exception: {e}")
            self.tests_failed += 1
            return False
    
    def test_config_loading(self, executable, config_file):
        """Test configuration file loading"""
        print_test("Testing config file loading")
        self.tests_run += 1
        
        # Create minimal test config
        test_config = """[framework]
log_file = /tmp/yuki-frame-test.log
log_level = INFO
max_tools = 10

[tool.echo_test]
command = echo "test"
description = Test tool
autostart = false
"""
        
        test_config_path = "test_integration.conf"
        try:
            with open(test_config_path, 'w') as f:
                f.write(test_config)
            
            # Start framework with test config (should initialize and exit quickly)
            # We'll just verify it doesn't crash on startup
            result = subprocess.run(
                [executable, '--config', test_config_path],
                capture_output=True,
                text=True,
                timeout=2
            )
            
            # Check if it loaded config (should run briefly or we kill it)
            if 'Failed to load configuration' not in result.stderr:
                print_pass("Config loading works")
                self.tests_passed += 1
                return True
            else:
                print_fail(f"Config loading failed: {result.stderr}")
                self.tests_failed += 1
                return False
                
        except subprocess.TimeoutExpired:
            # Framework started successfully and is running
            print_pass("Config loaded and framework started")
            self.tests_passed += 1
            return True
        except Exception as e:
            print_fail(f"Exception: {e}")
            self.tests_failed += 1
            return False
        finally:
            if os.path.exists(test_config_path):
                os.remove(test_config_path)
    
    def test_invalid_config(self, executable):
        """Test handling of invalid config file"""
        print_test("Testing invalid config handling")
        self.tests_run += 1
        
        try:
            result = subprocess.run(
                [executable, '--config', 'nonexistent_config.conf'],
                capture_output=True,
                text=True,
                timeout=5
            )
            
            # Should fail gracefully
            if result.returncode != 0 and 'Failed' in result.stderr:
                print_pass("Invalid config handled correctly")
                self.tests_passed += 1
                return True
            else:
                print_fail("Invalid config not handled properly")
                self.tests_failed += 1
                return False
                
        except Exception as e:
            print_fail(f"Exception: {e}")
            self.tests_failed += 1
            return False
    
    def print_summary(self):
        """Print test summary"""
        print("\n" + "="*50)
        print("Integration Test Summary")
        print("="*50)
        print(f"Total tests:  {self.tests_run}")
        print(f"Passed:       {Colors.GREEN}{self.tests_passed}{Colors.END}")
        print(f"Failed:       {Colors.RED}{self.tests_failed}{Colors.END}")
        
        if self.tests_failed == 0:
            print(f"\n{Colors.GREEN}[PASS] All tests passed!{Colors.END}")
            return 0
        else:
            print(f"\n{Colors.RED}[FAIL] Some tests failed{Colors.END}")
            return 1

def main():
    print("\n" + "="*50)
    print("Yuki-Frame Integration Tests")
    print("="*50 + "\n")
    
    # Find executable
    executable = None
    search_paths = [
        '../build/yuki-frame.exe',
        '../build/yuki-frame',
        '../../build/yuki-frame.exe',
        '../../build/yuki-frame',
        './yuki-frame.exe',
        './yuki-frame'
    ]
    
    for path in search_paths:
        if os.path.exists(path):
            executable = path
            break
    
    if not executable:
        print_fail("Could not find yuki-frame executable")
        print_info("Please build the project first")
        return 1
    
    print_info(f"Using executable: {executable}")
    print()
    
    # Run tests
    tester = IntegrationTest()
    
    tester.test_framework_version(executable)
    tester.test_framework_help(executable)
    tester.test_config_loading(executable, 'test.conf')
    tester.test_invalid_config(executable)
    
    # Print summary
    return tester.print_summary()

if __name__ == '__main__':
    sys.exit(main())
