@echo off
echo Starting Yuki-Frame v2.0...
echo.

REM Create logs directory if it doesn't exist
if not exist "logs" (
    echo Creating logs directory...
    mkdir logs
)

REM Check if config exists
if not exist "yuki-frame.conf" (
    echo Error: yuki-frame.conf not found!
    echo Please copy yuki-frame.conf.example to yuki-frame.conf
    pause
    exit /b 1
)

REM Run framework
echo Running framework...
echo Press Ctrl+C to stop
echo.
build\Release\yuki-frame.exe -c yuki-frame.conf

echo.
echo Framework stopped
pause
