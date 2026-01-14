@echo off
echo ========================================
echo Standalone Tool Test
echo ========================================
echo.

cd tools

echo Testing Sender (will send 3 messages then stop)...
echo ----------------------------------------
timeout /t 1 /nobreak >nul
(
    echo Starting sender...
    python sender.py
) &

echo.
echo.
echo Testing Receiver (paste an event or press Ctrl+C)...
echo ----------------------------------------
echo Example input: MESSAGE|sender|Hello World
echo.
python receiver.py

echo.
echo Test complete!
pause
