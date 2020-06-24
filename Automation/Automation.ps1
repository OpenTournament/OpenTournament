
# TO-DO Place the output in a dedicated log file.
#@echo

# $args[1] = "C:\Program Files\Epic Games\UE_4.25.1\Engine"
# $args[2] = "D:\Professional\Projects\DevOpsTests"
# $args[3] = Assemble\Build\Cook\Archive
# $args[4] = Editor\Client\Server\[Blank]
# $args[5] = Development\Shipping\Debug
# $args[6] = Win64\Linux

# These folders represent the setup of the machine which reads this automation file.
$TIME = Get-Date -Format "dd-mm-yyyy"
$TOOLS_PATH = $args[1]
$PROJECT_PATH = $args[2]
$PROJECT_TITLE = $args[2].split("\",2)[-1]

# These arguments are used in all commands.
$ARGUMENTS = "-Project=`"$PROJECT_PATH\$PROJECT_TITLE.uproject`" -Target=$PROJECT_TITLE$args[4] -Configuration=$args[5] -Platform=$args[6]'"

# These commands represent the various processes required to produce an executable.
$COMMAND_ASSEMBLE_A="`"$TOOLS_PATH\Binaries\DotNET\UnrealBuildTool.exe`" BootstrapPackagedGame Shipping Win64"
$COMMAND_ASSEMBLE_B="`"$TOOLS_PATH\Binaries\DotNET\UnrealBuildTool.exe`" `"$PROJECT_PATH\$PROJECT_TITLE.uproject`" -ProjectFiles -Game -Progress"
$COMMAND_ASSEMBLE_C="`"$TOOLS_PATH\Binaries\DotNET\UnrealBuildTool.exe`" `"$PROJECT_PATH\$PROJECT_TITLE.uproject`" $($PROJECT_TITLE)Editor Development Win64 -WaitMutex -FromMsBuild"
$COMMAND_ASSEMBLE_D"`"$TOOLS_PATH\Binaries\Win64\UE4Editor-Cmd.exe`" `"$PROJECT_PATH\$PROJECT_TITLE.uproject`" -Run=CompileAllBlueprints -IgnoreFolder=/Engine,/RuntimeTests"
$COMMAND_BUILD="`"$TOOLS_PATH\Build\BatchFiles\RunUAT.bat`" BuildTarget $ARGUMENTS"
$COMMAND_COOK="`"$TOOLS_PATH\Build\BatchFiles\RunUAT.bat`" BuildCookRun $ARGUMENTS -Cook -SkipEditorContent -Compressed -Unversioned"
$COMMAND_STAGE="`"$TOOLS_PATH\Build\BatchFiles\RunUAT.bat`" BuildCookRun $ARGUMENTS -Stage -StagingDirectory=`"%ROOT_PROJECT%Output\Staged\\%~n4\%~n5\%~n6`" -SkipCook"
$COMMAND_ARCHIVE="`"$PROJECT_PATH\Automation\Windows\Tools\7z1900-extra\x64\7za.exe`" a `"$PROJECT_PATH\Output\Archived\%~n4\%~n5\%~n6\%ROOT_TITLE%_%~n4_%~n5_%~n6_%PACKAGE_DATE%_%PACKAGE_TIME%.zip`" `"$PROJECT_PATH\Output\Staged\%~n4\%~n5\%~n6\*`""

$COMMAND_LIGHTING="`"$TOOLS_PATH\Build\BatchFiles\RunUAT.bat`" RebuildLightmaps $ARGUMENTS"

if ($args[3] -eq "Assemble")
{
    & $COMMAND_ASSEMBLE_A
    & $COMMAND_ASSEMBLE_B
    & $COMMAND_ASSEMBLE_C
    & $COMMAND_ASSEMBLE_D
}

if ($args[3] -eq "Build") 
{
    & $COMMAND_BUILD
}

if ($args[3] -eq "Cook")
{
    & $COMMAND_COOK
}

if ($args[3] -eq "Stage")
{
    & $COMMAND_STAGE
    if ($args[6] -eq "Win64")
    {
        $PATH_SOURCE  = "$TOOLS_PATH\Binaries\ThirdParty\AppLocalDependencies\%~n6\*"
        $PATH_DESTIONATION = "$PROJECT_PATH\Output\Staged\%~n4\%~n5\%~n6\Windows%~n4\Engine\Binaries\%~n6\"
        Copy-Item -Force -Recurse -Verbose $sourceDirectory -Destination $destinationDirectory
    }
}

if ($args[3] -eq "Archive")
{
    & $COMMAND_ARCHIVE
}

if ($args[3] -eq "Lighting")
{
    & $COMMAND_LIGHTING
}

echo --------------------
echo COMMAND_ASSEMBLE[0]: $COMMAND_ASSEMBLE[0]
echo COMMAND_ASSEMBLE[0]: $COMMAND_ASSEMBLE[1]
echo COMMAND_ASSEMBLE[0]: $COMMAND_ASSEMBLE[2]
echo COMMAND_BUILD: $COMMAND_BUILD
echo COMMAND_COOK: $COMMAND_COOK
echo COMMAND_STAGE[0]: $COMMAND_STAGE
echo COMMAND_ARCHIVE: $COMMAND_ARCHIVE
echo --------------------
