@echo off
set "retdir=%CD%"

if "%~1" == "" (
    echo:
    echo Usage: packdat foldername [outputname]
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
) else (
    set "datfile=%~nx2"
)

del "%~dp1%datfile%.bak" 2>nul
ren "%~dp1%datfile%" "%datfile%.bak" 2>nul
cd /d "%~dpnx1"
dat2.exe a -r "..\%datfile%" *
cd /d "%retdir%"
