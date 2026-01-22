@echo off
REM yuki-control.bat - Simple control script for Yuki-Frame (Windows)
REM Writes commands to command file and displays response

setlocal enabledelayedexpansion

set COMMAND_FILE=yuki-frame.cmd
set RESPONSE_FILE=yuki-frame.response
set MAX_WAIT=5

if "%1"=="" goto show_usage
if "%1"=="help" goto show_usage
if "%1"=="-h" goto show_usage
if "%1"=="--help" goto show_usage

set COMMAND=%1
set TOOL_NAME=%2

REM Remove old response file
if exist "%RESPONSE_FILE%" del "%RESPONSE_FILE%"

REM Write command to file
if "%TOOL_NAME%"=="" (
    echo %COMMAND% > "%COMMAND_FILE%"
) else (
    echo %COMMAND% %TOOL_NAME% > "%COMMAND_FILE%"
)

echo Sent command: %COMMAND% %TOOL_NAME%
echo|set /p="Waiting for response"

REM Wait for response
set /a count=0
:wait_loop
if %count% GEQ %MAX_WAIT% goto timeout

if exist "%RESPONSE_FILE%" (
    echo.
    echo.
    type "%RESPONSE_FILE%"
    del "%RESPONSE_FILE%"
    del "%COMMAND_FILE%"
    goto end
)

echo|set /p="."
timeout /t 1 /nobreak >nul
set /a count+=1
goto wait_loop

:timeout
echo.
echo.
echo Error: Timeout waiting for response.
echo Is yuki-frame running? Check with: tasklist ^| findstr yuki-frame
if exist "%COMMAND_FILE%" del "%COMMAND_FILE%"
exit /b 1

:show_usage
echo Yuki-Frame Control Script v2.0
echo.
echo Usage: %0 COMMAND [TOOL_NAME]
echo.
echo Commands:
echo   start ^<tool^>     Start a tool
echo   stop ^<tool^>      Stop a tool
echo   restart ^<tool^>   Restart a tool
echo   list             List all tools and their status
echo   status ^<tool^>    Show detailed status of a tool
echo   shutdown         Shutdown the framework
echo   help             Show this help message
echo.
echo Examples:
echo   %0 list
echo   %0 start my_tool
echo   %0 status my_tool
echo   %0 stop my_tool
echo.
exit /b 0

:end
endlocal
