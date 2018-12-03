@echo off
setlocal enableextensions

set SCRIPT_DIR=%~dp0
set ROOT=%SCRIPT_DIR%\..
set BUILD_DIR=%ROOT%\build\windows

md "%BUILD_DIR%" || goto :error
cd "%BUILD_DIR%" || goto :error
cmake -A x64 "%ROOT%" || goto :error

exit /b 0

:error
echo Failed with error #%errorlevel%.
exit /b %errorlevel%