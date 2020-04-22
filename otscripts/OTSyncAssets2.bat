@echo off
setlocal EnableDelayedExpansion EnableExtensions
title OTSyncAssets2.bat

:: Scripts directory
set "otscripts=%~dp0"
cd !otscripts!

:: Extract root directory (OT root)
set "extract-root=!otscripts!..\"

:: List of archives
set "packlist=!otscripts!otdeps.packlist"

:: Folder to cache downloaded packs in
if not exist packcache (
	md packcache
)
:: Head there directly because we'll have to run wget from there anyway
:: Because wget --timestamping doesn't support --output-document
cd packcache

:: Parse arguments
set nopause=0
set overwrite=0
for %%a in (%*) do (
	:: Automation - skip pause at end of script
	if /I "%%a"=="-nopause" (
		set nopause=1
	)
	:: Overwrite - extract archives overwriting all local files instead of keeping newer ones
	if /I "%%a"=="-overwrite" (
		set overwrite=1
	)
)

(set lf=^

)

set /a numpacks=0
for /F "tokens=*" %%l in (!packlist!) do (
	set /a numpacks+=1
)

set /a count=0

set errors=

for /F "tokens=*" %%l in (!packlist!) do (
	call :PROC "%%l"
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

:: For loop body - separated so I can use GOTO inside...
:PROC
for /F "tokens=1,2*" %%a in (%1) do (
	set url=%%a
	set archive=%%~nxa
	set extract-dir=%%b
	:: ensure backward slashes
	set "extract-dir=!extract-dir:/=\!"
)

set /a count+=1
echo [!count!/!numpacks!] !url!
echo.

:: Temp file path
set "tmpfile=!archive!.dling"

:: Check if temp file exists - which means download failed or was interrupted during previous sync
if exist !tmpfile! (
	:: Delete failed/interrupted file
	if exist !archive! (
		echo ^^!^^! Cleaning up incomplete download from previous sync
		del !archive!
	)
) else (
	:: Create temp file to indicate downloading
	type NUL > !tmpfile!
)

:: Launch wget conditional download
!otscripts!wget.exe --timestamping --no-verbose --show-progress "!url!"
echo.

if errorlevel 1 (
	set err=!errorlevel!
	set errors=!errors![!count!/!numpacks!] !url!
	if !err! equ 8 set "errors=!errors! (WGET SERVER ERROR)"
	if !err! equ 7 set "errors=!errors! (WGET PROTOCOL ERROR)"
	if !err! equ 6 set "errors=!errors! (WGET AUTH FAIL)"
	if !err! equ 5 set "errors=!errors! (WGET SSL FAIL)"
	if !err! equ 4 set "errors=!errors! (WGET NETWORK FAIL)"
	if !err! equ 3 set "errors=!errors! (WGET IO ERROR)"
	if !err! equ 2 set "errors=!errors! (WGET PARSE/CMD ERROR)"
	if !err! equ 1 set "errors=!errors! (WGET GENERIC/UNKNOWN)"
	set errors=!errors!!lf!
	exit /b
)

:: Delete the temp file
del !tmpfile!

:: If we have no reference archive and wget didn't download one, something went wrong
if not exist !archive! (
	set "errors=!errors![!count!/!numpacks!] !archive! (NO ARCHIVE)!lf!"
	exit /b
)

:: If archive is our 0kb reference file, then we are up to date - skip it
for %%a in (!archive!) do set size=%%~za
if !size! LSS 10 (
	exit /b
)

:: Extract archive
echo [!count!/!numpacks!] Extract !archive! -^> !extract-dir!

if !overwrite!==1 (

	:: If overwrite is on, we can extract-overwrite directly to target
	!otscripts!7z.exe x -aoa -o"!extract-root!!extract-dir!" !archive!
	echo.

	if errorlevel 1 (
		set "errors=!errors![!count!/!numpacks!] !archive! (7Z !errorlevel!)!lf!"
		exit /b
	)

) else (

	:: If overwrite is off, extract into a temp folder first
	if exist "extract" (
		rmdir /S/Q "extract"
	)

	!otscripts!7z.exe x -o"extract" !archive!
	echo.

	if errorlevel 1 (
		set "errors=!errors![!count!/!numpacks!] !archive! (7Z !errorlevel!)!lf!"
		exit /b
	)

	:: Use XCOPY to copy only newer files
	xcopy "extract\*" "!extract-root!!extract-dir!" /D /S /V /I /Y /Q
	echo.

	if errorlevel 1 (
		set "errors=!errors![!count!/!numpacks!] !archive! (XCOPY !errorlevel!)!lf!"
		exit /b
	)

	rmdir /S/Q "extract"
)

:: Turn archive into a 0kb file, while keeping timestamp for wget comparisons
type NUL > .tmp
!otscripts!nircmd.exe clonefiletime !archive! .tmp
del !archive!
ren .tmp !archive!

:: Subroutine end
exit /b
