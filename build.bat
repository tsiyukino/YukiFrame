@echo off
REM Build script for Yuki-Frame v2.0 (Windows)

echo ====================================
echo Building Yuki-Frame v2.0
echo ====================================
echo.

if not exist build mkdir build
cd build

echo [1/2] Configuring...
cmake .. -G "Visual Studio 17 2022" -A x64
if errorlevel 1 goto error

echo [2/2] Building...
cmake --build . --config Release
if errorlevel 1 goto error

cd ..

echo.
echo ====================================
echo Build Complete!
echo ====================================
echo.
echo Executable: build\Release\yuki-frame.exe
echo.
echo Next steps:
echo   1. Copy yuki-frame.conf.example to yuki-frame.conf
echo   2. Edit configuration
echo   3. Run: build\Release\yuki-frame.exe -c yuki-frame.conf
echo.
pause
exit /b 0

:error
echo.
echo Build failed!
pause
exit /b 1
