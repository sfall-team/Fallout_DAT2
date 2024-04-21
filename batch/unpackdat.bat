@echo off
set "retdir=%CD%"

if "%~1" == "" (
    echo:
    echo Usage: unpackdat filename [output_folder]
    exit /b
)

if not exist "%~1" (
    echo:
    echo Error: DAT file not found
    exit /b
)

if "%~2" == "" (
    set "ofldr=%~n1"
) else (
    set "ofldr=%~nx2"
)

cd /d "%~dp1"
dat2.exe x -d "%ofldr%" "%~nx1"
cd /d "%retdir%"
