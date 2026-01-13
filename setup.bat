@echo off
echo ========================================
echo Yuki-Frame v2.0 Setup
echo ========================================
echo.

echo [1/3] Creating directories...
if not exist "logs" mkdir logs
if not exist "tools" mkdir tools
echo Done!

echo.
echo [2/3] Creating configuration...
if exist "yuki-frame.conf" (
    echo Configuration already exists: yuki-frame.conf
) else (
    if exist "yuki-frame.conf.example" (
        copy yuki-frame.conf.example yuki-frame.conf >nul
        echo Created: yuki-frame.conf
    ) else (
        echo Warning: yuki-frame.conf.example not found
    )
)

echo.
echo [3/3] Setup complete!
echo.
echo ========================================
echo Next steps:
echo ========================================
echo 1. Edit yuki-frame.conf (if needed)
echo 2. Run: run.bat
echo.
pause
