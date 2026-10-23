@echo off
set "retdir=%CD%"
set "compression=6"

set "first=%~1"
echo %first%| findstr /r "^[0-9]$" >nul
if not errorlevel 1 (
    set "compression=%first%"
    shift /1
)

if "%~1" == "" (
    echo:
    echo Usage: packdat [compression_level] foldername [outputname]
    exit /b
)

if not exist "%~1" (
    echo:
    echo Error: Folder not found
    exit /b
)

set "datpath=%~dp1"

if "%~2" == "" (
    set "datfile=%~nx1.dat"
) else if /i "%~x2" == ".dat" (
    set "datfile=%~nx2"
) else (
    set "datfile=%~nx2.dat"
)

if exist "%~dp1%datfile%\" (
    echo:
    echo Error: "%datfile%" is an existing folder, choose a different output name
    exit /b
)

del "%~dp1%datfile%.bak" 2>nul
ren "%~dp1%datfile%" "%datfile%.bak" 2>nul
cd /d "%~dpnx1"
if exist "%retdir%\dat2.exe" (
    "%retdir%\dat2.exe" a -%compression% -r "..\%datfile%" *
) else (
    dat2.exe a -%compression% -r "..\%datfile%" *
)
cd /d "%retdir%"
