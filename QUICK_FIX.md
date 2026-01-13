# Quick Fix: "Failed to open log file"

## Problem

```
Failed to open log file: logs/yuki-frame.log
Failed to initialize logger
Failed to initialize framework
```

## Solutions

### Option 1: Quick Fix (30 seconds)

Just create the logs directory:

```cmd
mkdir logs
.\yuki-frame.exe -c yuki-frame.conf
```

### Option 2: Use Setup Script (1 minute)

I've created a setup script that creates all needed directories:

```cmd
setup.bat
run.bat
```

### Option 3: Run from Project Root

Make sure you're running from the right directory:

```cmd
# Wrong (will fail)
cd build\Release
.\yuki-frame.exe -c .\yuki-frame.conf

# Right (will work)
cd E:\Git\yuki-frame
mkdir logs
build\Release\yuki-frame.exe -c yuki-frame.conf
```

### Option 4: Use Absolute Path in Config

Edit `yuki-frame.conf`:

```ini
[core]
# Change this:
log_file = logs/yuki-frame.log

# To this:
log_file = E:\Git\yuki-frame\logs\yuki-frame.log
```

## What Changed

The new logger.c now:
- ✅ Auto-creates log directory
- ✅ Falls back to current directory if path fails
- ✅ Shows helpful error messages

## Full Workflow

```cmd
# 1. Go to project root
cd E:\Git\yuki-frame

# 2. Run setup (creates directories and config)
setup.bat

# 3. Run framework
run.bat
```

Or manually:

```cmd
# 1. Create directories
mkdir logs

# 2. Copy config
copy yuki-frame.conf.example yuki-frame.conf

# 3. Run
build\Release\yuki-frame.exe -c yuki-frame.conf
```

## Verify It Works

You should see:

```
2026-01-13 20:00:00 [INFO] [main] Yuki-Frame v2.0.0 starting
2026-01-13 20:00:00 [INFO] [platform] Windows platform initialized
2026-01-13 20:00:00 [INFO] [event] Event bus initialized
2026-01-13 20:00:00 [INFO] [tool] Tool registry initialized
2026-01-13 20:00:00 [INFO] [control] Control system initialized
2026-01-13 20:00:00 [INFO] [main] Framework initialized successfully
2026-01-13 20:00:00 [INFO] [main] Entering main loop
```

Press Ctrl+C to stop.

Check the log:

```cmd
type logs\yuki-frame.log
```

## Why It Happened

The framework expects to run from the project root directory where:
- `yuki-frame.conf` is located
- `logs/` directory exists (or can be created)
- Relative paths in config work correctly

## New Features

The updated logger now:
1. Automatically creates the log directory
2. Falls back to current directory if needed
3. Shows helpful error messages
4. Works from any directory (as long as logs can be created)

## Test It

```cmd
# From anywhere
cd E:\Git\yuki-frame
setup.bat
run.bat
```

Should work! ✅
