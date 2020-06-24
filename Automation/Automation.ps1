
# TO-DO Place the output in a dedicated log file.

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
$TOOLS_PATH = $($args[0])
$PROJECT_PATH = $($args[1])
$PROJECT_TITLE = $($args[1]).split("\",2)[-1]

# These arguments are used in all commands.
$ARGUMENTS = "-Project=`"$PROJECT_PATH\$PROJECT_TITLE.uproject`" -Target=$PROJECT_TITLE$($args[3]) -Configuration=$($args[4]) -Platform=$($args[5])'"

if ($($args[2]) -eq "Assemble")
{
    Write-Host | & "$TOOLS_PATH\Binaries\DotNET\UnrealBuildTool.exe" BootstrapPackagedGame, Shipping, Win64
    Write-Host | & "$TOOLS_PATH\Binaries\DotNET\UnrealBuildTool.exe" "$PROJECT_PATH\$PROJECT_TITLE.uproject", -ProjectFiles, -Game, -Progress
    Write-Host | & "$TOOLS_PATH\Binaries\DotNET\UnrealBuildTool.exe" "$PROJECT_PATH\$PROJECT_TITLE.uproject", $($PROJECT_TITLE)Editor, Development, Win64, -WaitMutex, -FromMsBuild
    Write-Host | & "$TOOLS_PATH\Binaries\Win64\UE4Editor-Cmd.exe" "$PROJECT_PATH\$PROJECT_TITLE.uproject", -Run=CompileAllBlueprints, -IgnoreFolder=/Engine,/RuntimeTests
}

if ($($args[2]) -eq "Build") 
{
    Write-Host | & "$TOOLS_PATH\Build\BatchFiles\RunUAT.bat" BuildTarget, $ARGUMENTS
}

if ($($args[2]) -eq "Cook")
{
    Write-Host | & "$TOOLS_PATH\Build\BatchFiles\RunUAT.bat" BuildCookRun, $ARGUMENTS, -Cook, -SkipEditorContent, -Compressed, -Unversioned
}

if ($($args[2]) -eq "Stage")
{
    Write-Host | & "$TOOLS_PATH\Build\BatchFiles\RunUAT.bat" BuildCookRun, $ARGUMENTS, -Stage, -StagingDirectory="%ROOT_PROJECT%Output\Staged\\$($args[3])\$($args[4])\$($args[5])", -SkipCook
    if ($($args[5]) -eq "Win64")
    {
        $PATH_SOURCE  = "$TOOLS_PATH\Binaries\ThirdParty\AppLocalDependencies\$($args[5])\*"
        $PATH_DESTIONATION = "$PROJECT_PATH\Output\Staged\$($args[3])\$($args[4])\$($args[5])\Windows$($args[3])\Engine\Binaries\$($args[5])\"
        Copy-Item -Force -Recurse -Verbose $PATH_SOURCE -Destination $PATH_DESTIONATION
    }
}

if ($($args[2]) -eq "Archive")
{
    Write-Host | Compress-Archive -Path "$PROJECT_PATH\Output\Staged\$($args[3])\$($args[4])\$($args[5])\*" -DestinationPath "$PROJECT_PATH\Output\Archived\$($args[3])\$($args[4])\$($args[5])\$($ROOT_TITLE)_$($args[3])_$($args[4])_$($args[5])_$($PACKAGE_DATE)_$($PACKAGE_TIME).zip"
}

if ($($args[2]) -eq "Lighting")
{
    Write-Host | & "$TOOLS_PATH\Build\BatchFiles\RunUAT.bat" RebuildLightmaps, $ARGUMENTS
}
