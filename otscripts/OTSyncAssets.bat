@echo off
setlocal EnableDelayedExpansion EnableExtensions
title OTSyncAssets.bat

:: Move to OT root
cd "%~dp0.."

:: List of files to download
set filelist=otscripts\otdeps.filelist
:: Base URL
set baseurl=https://opentournamentgame.com/game/
:: Skip the /game/ component when syncing to local directories
set cutdirs=1

:: Parse arguments
set nopause=0
for %%a in (%*) do (
	:: Automation - skip pause at end of script
	if /I "%%a"=="-nopause" (
		set nopause=1
	)
)

(set lf=^

)

set /a numfiles=0
for /F "tokens=*" %%f in (!filelist!) do (
	set /a numfiles+=1
)

set /a count=0

set errors=

for /F "tokens=*" %%f in (!filelist!) do (
	set "file=%%f"

	set /a count+=1
	echo [!count!/!numfiles!] File: !file!

	:: Get a version with backslashes
	set "winpath=!file:/=\!"
	:: Get a version with forward slashes
	set "webpath=!file:\=/!"

	:: Temp file path
	set "tmpfile=!winpath!.tmp"

	:: Folder
	set "folder=!winpath!\..\"

	:: Prep folder tree because we need to create temp file before wget
	if not exist !folder! (
		md !folder!
	)

	:: Check if temp file exists - which means file failed or was interrupted during previous sync
	if exist !tmpfile! (
		:: Delete failed/interrupted file
		if exist !winpath! (
			echo ^^!^^! Cleaning up incomplete download from previous sync
			del !winpath!
		)
	) else (
		:: Create temp file to indicate downloading
		type NUL > !tmpfile!
	)

	:: Launch wget routine
	otscripts\wget.exe --timestamping --force-directories --no-host-directories --cut-dirs=!cutdirs! --no-verbose --show-progress "!baseurl!!webpath!"

	if errorlevel 1 (
		:: Error - notify
		set err=!errorlevel!
		set errors=!errors![!count!/!numfiles!] !baseurl!!webpath!
		if !err! equ 8 set "errors=!errors! (SERVER ERROR)"
		if !err! equ 7 set "errors=!errors! (PROTOCOL ERROR)"
		if !err! equ 6 set "errors=!errors! (AUTH FAIL)"
		if !err! equ 5 set "errors=!errors! (SSL FAIL)"
		if !err! equ 4 set "errors=!errors! (NETWORK FAIL)"
		if !err! equ 3 set "errors=!errors! (IO ERROR)"
		if !err! equ 2 set "errors=!errors! (PARSE/CMD ERROR)"
		if !err! equ 1 set "errors=!errors! (GENERIC/UNKNOWN)"
		set errors=!errors!!lf!
	) else (
		:: Success - delete temp file
		del !tmpfile!
	)

	echo.
)

if "!errors!"=="" (
	echo ========================================
	echo =====          SUCCESS             =====
	echo ========================================
	if !nopause!==0 pause
	exit 0
) else (
	echo ========================================
	echo =====       ERRORS OCCURED         =====
	echo ========================================
	echo !errors!
	if !nopause!==0 pause
	exit 1
)