
# TO-DO Place the output in a dedicated log file.
#@echo

# $($args[0]) = "C:\Program Files\Epic Games\UE_4.25.1\Engine"
# $($args[1]) = "D:\Professional\Projects\DevOpsTests"
# $($args[2]) = Assemble\Build\Cook\Archive
# $($args[3]) = Editor\Client\Server\[Blank]
# $($args[4]) = Development\Shipping\Debug
# $($args[5]) = Win64\Linux

Write-Output "ARG-0: $($args[0])"
Write-Output "ARG-1: $($args[1])"
Write-Output "ARG-2: $($args[2])"
Write-Output "ARG-3: $($args[3])"
Write-Output "ARG-4: $($args[4])"
Write-Output "ARG-5: $($args[5])"

# These folders represent the setup of the machine which reads this automation file.
$TIME = Get-Date -Format "dd-mm-yyyy"
$TOOLS_PATH = $args[0]
$PROJECT_PATH = $args[1]
$PROJECT_TITLE = $args[1].split("\",2)[-1]

# These arguments are used in all commands.
$ARGUMENTS = "-Project=`"$PROJECT_PATH\$PROJECT_TITLE.uproject`" -Target=$PROJECT_TITLE$($args[3]) -Configuration=$($args[4]) -Platform=$($args[5])'"

# These commands represent the various processes required to produce an executable.
$COMMAND_ASSEMBLE_A="`"$TOOLS_PATH\Binaries\DotNET\UnrealBuildTool.exe`" BootstrapPackagedGame Shipping Win64"
$COMMAND_ASSEMBLE_B="`"$TOOLS_PATH\Binaries\DotNET\UnrealBuildTool.exe`" `"$PROJECT_PATH\$PROJECT_TITLE.uproject`" -ProjectFiles -Game -Progress"
$COMMAND_ASSEMBLE_C="`"$TOOLS_PATH\Binaries\DotNET\UnrealBuildTool.exe`" `"$PROJECT_PATH\$PROJECT_TITLE.uproject`" $($PROJECT_TITLE)Editor Development Win64 -WaitMutex -FromMsBuild"
$COMMAND_ASSEMBLE_D="`"$TOOLS_PATH\Binaries\Win64\UE4Editor-Cmd.exe`" `"$PROJECT_PATH\$PROJECT_TITLE.uproject`" -Run=CompileAllBlueprints -IgnoreFolder=/Engine,/RuntimeTests"
$COMMAND_BUILD="`"$TOOLS_PATH\Build\BatchFiles\RunUAT.bat`" BuildTarget $ARGUMENTS"
$COMMAND_COOK="`"$TOOLS_PATH\Build\BatchFiles\RunUAT.bat`" BuildCookRun $ARGUMENTS -Cook -SkipEditorContent -Compressed -Unversioned"
$COMMAND_STAGE="`"$TOOLS_PATH\Build\BatchFiles\RunUAT.bat`" BuildCookRun $ARGUMENTS -Stage -StagingDirectory=`"%ROOT_PROJECT%Output\Staged\\%~n4\%~n5\%~n6`" -SkipCook"
$COMMAND_ARCHIVE="`"$PROJECT_PATH\Automation\Windows\Tools\7z1900-extra\x64\7za.exe`" a `"$PROJECT_PATH\Output\Archived\%~n4\%~n5\%~n6\%ROOT_TITLE%_%~n4_%~n5_%~n6_%PACKAGE_DATE%_%PACKAGE_TIME%.zip`" `"$PROJECT_PATH\Output\Staged\%~n4\%~n5\%~n6\*`""

$COMMAND_LIGHTING="`"$TOOLS_PATH\Build\BatchFiles\RunUAT.bat`" RebuildLightmaps $ARGUMENTS"

if ($($args[2]) -eq "Assemble")
{
    & Write-Host | & "$TOOLS_PATH\Binaries\DotNET\UnrealBuildTool.exe" BootstrapPackagedGame, Shipping, Win64
    #& '`"$TOOLS_PATH\Binaries\DotNET\UnrealBuildTool.exe`" `"$PROJECT_PATH\$PROJECT_TITLE.uproject`" -ProjectFiles -Game -Progress'
    #& '`"$TOOLS_PATH\Binaries\DotNET\UnrealBuildTool.exe`" `"$PROJECT_PATH\$PROJECT_TITLE.uproject`" $($PROJECT_TITLE)Editor Development Win64 -WaitMutex -FromMsBuild'
    #& '`"$TOOLS_PATH\Binaries\Win64\UE4Editor-Cmd.exe`" `"$PROJECT_PATH\$PROJECT_TITLE.uproject`" -Run=CompileAllBlueprints -IgnoreFolder=/Engine,/RuntimeTests'
}

if ($($args[2]) -eq "Build") 
{
    & $COMMAND_BUILD
}

if ($($args[2]) -eq "Cook")
{
    & $COMMAND_COOK
}

if ($($args[2]) -eq "Stage")
{
    & $COMMAND_STAGE
    if ($args[6] -eq "Win64")
    {
        $PATH_SOURCE  = "$TOOLS_PATH\Binaries\ThirdParty\AppLocalDependencies\%~n6\*"
        $PATH_DESTIONATION = "$PROJECT_PATH\Output\Staged\%~n4\%~n5\%~n6\Windows%~n4\Engine\Binaries\%~n6\"
        Copy-Item -Force -Recurse -Verbose $PATH_SOURCE -Destination $PATH_DESTIONATION
    }
}

if ($($args[2]) -eq "Archive")
{
    & $COMMAND_ARCHIVE
}

if ($($args[2]) -eq "Lighting")
{
    & $COMMAND_LIGHTING
}

#echo --------------------
#echo COMMAND_ASSEMBLE_A: $COMMAND_ASSEMBLE_A
#echo COMMAND_ASSEMBLE_B: $COMMAND_ASSEMBLE_B
#echo COMMAND_ASSEMBLE_C: $COMMAND_ASSEMBLE_C
#echo COMMAND_BUILD: $COMMAND_BUILD
#echo COMMAND_COOK: $COMMAND_COOK
#echo COMMAND_STAGE: $COMMAND_STAGE
#echo COMMAND_ARCHIVE: $COMMAND_ARCHIVE
#echo --------------------
