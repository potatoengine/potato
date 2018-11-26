@echo off
setlocal enableextensions

set SCRIPT_DIR=%~dp0
set ROOT=%SCRIPT_DIR%\..
set BUILD_DIR=%ROOT%\build\windows

cd "%ROOT%"\.. || goto :error
call "%ROOT%\vcpkg\bootstrap-vcpkg.bat" || goto :error
"%ROOT%\vcpkg\vcpkg.exe" install sdl2:x64-windows || goto :error
md "%BUILD_DIR%" || goto :error
cd "%BUILD_DIR%" || goto :error
cmake -A x64 -DCMAKE_TOOLCHAIN_FILE="%ROOT%/vcpkg/scripts/buildsystems/vcpkg.cmake" "%ROOT%" || goto :error

exit /b 0

:error
echo Failed with error #%errorlevel%.
exit /b %errorlevel%