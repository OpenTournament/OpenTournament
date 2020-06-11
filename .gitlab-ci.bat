@echo

:: %~dp1 = "C:\Program Files\Epic Games\UE_4.25.1\Engine\"
:: %~dp2 = "D:/Professional/Projects/DevOpsTests/"
:: %~dp3 = Assemble/Build/Cook/Package
:: %~dp2 = Editor/Client/Server/[Blank]
:: %~dp3 = Development/Shipping/Debug
:: %~dp4 = Win64/Linux

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
set COMMAND_ASSEMBLE[0]="%ROOT_TOOLS%Binaries\DotNET\UnrealBuildTool.exe" "%ROOT_PROJECT%%ROOT_TITLE%.uproject" -ProjectFiles -Game -Progress
set COMMAND_ASSEMBLE[1]="%ROOT_TOOLS%Binaries\DotNET\UnrealBuildTool.exe" "%ROOT_PROJECT%%ROOT_TITLE%.uproject" %ROOT_TITLE%Editor Development Win64 -WaitMutex -FromMsBuild
set COMMAND_ASSEMBLE[2]="%ROOT_TOOLS%Binaries\Win64\UE4Editor-Cmd.exe" "%ROOT_PROJECT%%ROOT_TITLE%.uproject" -Run=CompileAllBlueprints -IgnoreFolder=/Engine,/RuntimeTests
set COMMAND_BUILD="%ROOT_TOOLS%Build\BatchFiles\RunUAT.bat" BuildTarget %ATTRIBUTES% -NoTools
set COMMAND_COOK="%ROOT_TOOLS%Build\BatchFiles\RunUAT.bat" BuildCookRun %ATTRIBUTES% -Cook -SkipEditorContent -Compressed -Unversioned
set COMMAND_STAGE="%ROOT_TOOLS%Build\BatchFiles\RunUAT.bat" BuildCookRun %ATTRIBUTES% -Stage -SkipCook
::set COMMAND_ARCHIVE="%ROOT_TOOLS%Build\BatchFiles\RunUAT.bat" BuildCookRun %ATTRIBUTES% -Stage -SkipCook

if "%~n3" == "Assemble" %COMMAND_ASSEMBLE[0]% & %COMMAND_ASSEMBLE[1]% & %COMMAND_ASSEMBLE[2]%
if "%~n3" == "Build" %COMMAND_BUILD%
if "%~n3" == "Cook" %COMMAND_COOK%
if "%~n3" == "Stage" %COMMAND_STAGE%
if "%~n3" == "Archive" %COMMAND_ARCHIVE%

:: echo --------------------
:: echo %COMMAND_ASSEMBLE[0]%
:: echo %COMMAND_ASSEMBLE[1]%
:: echo %COMMAND_ASSEMBLE[2]%
:: echo %COMMAND_BUILD%
:: echo %COMMAND_COOK%
:: echo %COMMAND_STAGE%
:: echo %COMMAND_ARCHIVE%
:: echo --------------------