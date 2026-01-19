@echo off
REM Test runner for Yuki-Frame (Windows)

setlocal

echo.
echo === Yuki-Frame Test Suite ===
echo.

set BUILD_DIR=build

if not exist "%BUILD_DIR%" (
    echo Build directory not found. Please build the project first.
    echo Run: build.bat
    exit /b 1
)

cd "%BUILD_DIR%"

echo Running unit tests...
echo.

REM Run CTest
cmake --build . --target test_all 2>nul
if errorlevel 1 (
    echo.
    echo Running tests with CTest...
    ctest --output-on-failure
    if errorlevel 1 (
        echo.
        echo Some tests failed!
        cd ..
        exit /b 1
    )
)

echo.
echo === All tests passed! ===
echo.

cd ..
endlocal
