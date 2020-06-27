@echo

:: %~dp1 = "C:\Program Files\Epic Games\UE_4.25.1\Engine\"
:: %~dp2 = "D:\Professional\Projects\DevOpsTests\"
:: %~dp3 = Assemble\Build\Cook\Archive
:: %~dp2 = Editor\Client\Server\[Blank]
:: %~dp3 = Development\Shipping\Debug
:: %~dp4 = Win64\Linux

for /f "tokens=2-4 delims=/ " %%a in ('date /t') do (set PACKAGE_DATE=%%c-%%a-%%b)
for /f "tokens=1-3 delims=/:/ " %%a in ('time /t') do (set PACKAGE_TIME=%%a-%%b-%%c)
set PACKAGE_TIME=%PACKAGE_TIME: =%

:: These folders represent the setup of the machine which reads this automation file.
set ROOT_TOOLS=%~dp1
set ROOT_PROJECT=%~dp2
set ROOT_TITLE=

:: Extract the expected project name from the second parameter.
for %%a in (%ROOT_PROJECT%*.uproject) do (
    for %%b in ("%%a") do set ROOT_TITLE=%%~nb
)

:: These parameters are used in all commands.
SET ATTRIBUTES=-Project="%ROOT_PROJECT%%ROOT_TITLE%.uproject" -Target=%ROOT_TITLE%%~n4 -Configuration=%~n5 -Platform=%~n6

:: These commands represent the various processes required to produce an executable.
set COMMAND_ASSEMBLE[0]="%ROOT_TOOLS%Binaries\DotNET\UnrealBuildTool.exe" BootstrapPackagedGame Shipping Win64
set COMMAND_ASSEMBLE[1]="%ROOT_TOOLS%Binaries\DotNET\UnrealBuildTool.exe" "%ROOT_PROJECT%%ROOT_TITLE%.uproject" -ProjectFiles -Game -Progress
set COMMAND_ASSEMBLE[2]="%ROOT_TOOLS%Binaries\DotNET\UnrealBuildTool.exe" "%ROOT_PROJECT%%ROOT_TITLE%.uproject" %ROOT_TITLE%Editor Development Win64 -WaitMutex -FromMsBuild
set COMMAND_ASSEMBLE[3]="%ROOT_TOOLS%Binaries\Win64\UE4Editor-Cmd.exe" "%ROOT_PROJECT%%ROOT_TITLE%.uproject" -Run=CompileAllBlueprints -IgnoreFolder=/Engine,/RuntimeTests
set COMMAND_BUILD="%ROOT_TOOLS%Build\BatchFiles\RunUAT.bat" BuildTarget %ATTRIBUTES%
set COMMAND_COOK="%ROOT_TOOLS%Build\BatchFiles\RunUAT.bat" BuildCookRun %ATTRIBUTES% -Cook -SkipEditorContent -Compressed -Unversioned
set COMMAND_STAGE="%ROOT_TOOLS%Build\BatchFiles\RunUAT.bat" BuildCookRun %ATTRIBUTES% -Stage -StagingDirectory="%ROOT_PROJECT%Output\Staged\\%~n4\%~n5\%~n6" -SkipCook
set COMMAND_ARCHIVE="%ROOT_PROJECT%Automation\Windows\Tools\7z1900-extra\x64\7za.exe" a "%ROOT_PROJECT%Output\Archived\%~n4\%~n5\%~n6\%ROOT_TITLE%_%~n4_%~n5_%~n6_%PACKAGE_DATE%_%PACKAGE_TIME%.zip" "%ROOT_PROJECT%Output\Staged\%~n4\%~n5\%~n6\*"

set COMMAND_LIGHTING="%ROOT_TOOLS%Build\BatchFiles\RunUAT.bat" RebuildLightmaps %ATTRIBUTES%

if "%~n3" == "Assemble" %COMMAND_ASSEMBLE[0]% & %COMMAND_ASSEMBLE[1]% & %COMMAND_ASSEMBLE[2]% & %COMMAND_ASSEMBLE[3]%
if "%~n3" == "Build" %COMMAND_BUILD%
if "%~n3" == "Cook" %COMMAND_COOK%
if "%~n3" == "Stage" %COMMAND_STAGE% & if "%~n6" == "Win64" for /r "%ROOT_TOOLS%Binaries\ThirdParty\AppLocalDependencies\%~n6\" %%f in (*.dll) do ( xcopy "%%f" "%ROOT_PROJECT%Output\Staged\%~n4\%~n5\%~n6\Windows%~n4\Engine\Binaries\%~n6\" )
if "%~n3" == "Archive" %COMMAND_ARCHIVE%
if "%~n3" == "Lighting" %COMMAND_LIGHTING%

echo --------------------
echo COMMAND_ASSEMBLE[0]: %COMMAND_ASSEMBLE[0]%
echo COMMAND_ASSEMBLE[0]: %COMMAND_ASSEMBLE[1]%
echo COMMAND_ASSEMBLE[0]: %COMMAND_ASSEMBLE[2]%
echo COMMAND_BUILD: %COMMAND_BUILD%
echo COMMAND_COOK: %COMMAND_COOK%
echo COMMAND_STAGE[0]: %COMMAND_STAGE%
echo COMMAND_ARCHIVE: %COMMAND_ARCHIVE%
echo --------------------

