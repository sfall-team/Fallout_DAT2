# Builds build/dat2.exe with the newest Visual Studio that has MFC, using the settings of src/dat2.vcproj's
# Release|Win32 configuration: static MFC and CRT, no character set, x86.
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $true

$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$vs = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.ATLMFC -property installationPath
if (-not $vs) { throw 'No Visual Studio with the ATL/MFC component found' }
& (Join-Path $vs 'Common7\Tools\Launch-VsDevShell.ps1') -Arch x86 -HostArch amd64 -SkipAutomaticLocation | Out-Null

$root = Resolve-Path (Join-Path $PSScriptRoot '..\..')
$src = Join-Path $root 'src'
$out = Join-Path $root 'build'
New-Item -ItemType Directory -Force $out | Out-Null
Set-Location $src

rc /nologo /fo "$out\dat2.res" dat2.rc
# The precompiled header is built first, then every other file compiles against it in parallel
$flags = '/nologo', '/c', '/O2', '/MT', '/W3', '/EHsc', '/DWIN32', '/DNDEBUG', '/D_CONSOLE', "/Fp$out\dat2.pch", "/Fo$out\"
cl @flags /Ycstdafx.h stdafx.cpp
$sources = Get-ChildItem *.cpp | Where-Object Name -ne 'stdafx.cpp' | ForEach-Object Name
cl @flags /MP /Yustdafx.h @sources
$objects = Get-ChildItem "$out\*.obj" | ForEach-Object FullName
link /nologo /SUBSYSTEM:CONSOLE /MACHINE:X86 /OPT:REF /OPT:ICF /NODEFAULTLIB:msvcrt.lib "/OUT:$out\dat2.exe" `
	@objects "$out\dat2.res" lib\zlib.lib
