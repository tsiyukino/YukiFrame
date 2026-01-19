@echo off
REM Build script for Yuki-Frame (Windows)

setlocal

echo === Yuki-Frame Build Script ===
echo.

REM Parse arguments
set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Release

set BUILD_DIR=build

echo Build type: %BUILD_TYPE%
echo.

REM Create build directory
if not exist "%BUILD_DIR%" (
    echo Creating build directory...
    mkdir "%BUILD_DIR%"
)

cd "%BUILD_DIR%"

REM Configure
echo Configuring...
cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE% .. || (
    echo Configuration failed!
    exit /b 1
)

REM Build
echo Building...
cmake --build . --config %BUILD_TYPE% || (
    echo Build failed!
    exit /b 1
)

echo.
echo === Build successful! ===
echo.
if exist "%BUILD_DIR%\%BUILD_TYPE%\yuki-frame.exe" (
    echo Executable: %BUILD_DIR%\%BUILD_TYPE%\yuki-frame.exe
    echo.
    echo To run:
    echo   cd %BUILD_DIR%\%BUILD_TYPE% ^&^& yuki-frame.exe ..\..\yuki-frame.conf.example
) else (
    echo Executable: %BUILD_DIR%\yuki-frame.exe
    echo.
    echo To run:
    echo   cd %BUILD_DIR% ^&^& yuki-frame.exe ..\yuki-frame.conf.example
)
echo.
echo To install:
echo   cd %BUILD_DIR% ^&^& cmake --install . --config %BUILD_TYPE%
echo.

endlocal
