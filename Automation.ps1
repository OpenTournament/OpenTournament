<#

.SYNOPSIS

Handles all the individual steps required to turn an unpackaged project into a packaged project.

.DESCRIPTION

Insert description here.

.EXAMPLE

Insert example here. 

#>

[CmdletBinding()]
param
(
    # 'C:\UE_4.25.1'
    [string]$PathToEngine = $(throw "The <PathToEngine> parameter is required."),
    
    # 'C:\UE_PROJECTS\ProjectName'
    [string]$PathToProject = $(throw "The <PathToProject> parameter is required."),
    
    # 'C:\UE_PROJECTS\ProjectName\Artifacts'
    [string]$PathToArtifacts = $(throw "The <PathToArtifacts> parameter is required."),
    
    # Assemble|Resave|Build|Cook|Stage|Safeguard|Archive|Deploy
    [string]$Command = $(throw "The <Command> parameter is required."),
    
    # Editor|Client|Server|Game
    [string]$TargetType,
    
    # Development|Shipping|Debug
    [string]$TargetConfiguration,
    
    # Win64|Linux|Mac
    [string]$TargetPlatform    
)

function Main
{
    # Log the raw parameters.

    Write-Output "[Parameter-Raw] PathToEngine: $PathToEngine"
    Write-Output "[Parameter-Raw] PathToProject: $PathToProject"
    Write-Output "[Parameter-Raw] PathToArtifacts: $PathToArtifacts"
    Write-Output "[Parameter-Raw] Command: $Command"
    Write-Output "[Parameter-Raw] TargetType: $TargetType"
    Write-Output "[Parameter-Raw] TargetConfiguration: $TargetConfiguration"
    Write-Output "[Parameter-Raw] TargetPlatform: $TargetPlatform"

    # Extract the derived parameters.

    $PathToUnrealBuildTool = "$PathToEngine\Engine\Binaries\DotNET\UnrealBuildTool.exe"
    $PathToUnrealAutomationTool = "$PathToEngine\Engine\Build\BatchFiles\RunUAT.bat"
    $PathToUnrealEditor = "$PathToEngine\Engine\Binaries\Win64\UE4Editor-Cmd.exe"

    [string[]]$Projects = (Get-ChildItem "$PathToProject\*.uproject").BaseName

    if ($Projects.Length -ne 1)
    {
        throw "Unable to extract the project title. There needs to be exactly one project descriptor located at <$PathToProject>."
    }

    [string]$Project = $Projects[0]

    # Log the derived parameters.

    Write-Output "[Parameter-Derived] PathToUnrealBuildTool: $PathToUnrealBuildTool"
    Write-Output "[Parameter-Derived] PathToUnrealAutomationTool: $PathToUnrealAutomationTool"
    Write-Output "[Parameter-Derived] PathToUnrealEditor: $PathToUnrealEditor"
    Write-Output "[Parameter-Derived] Project: $Project"

    # Prepare the script.

    $ErrorActionPreference = 'Stop'

    if ($Command -notin @('Assemble', 'Resave') -and ([string]::IsNullOrEmpty($TargetType) -or [string]::IsNullOrEmpty($TargetConfiguration) -or [string]::IsNullOrEmpty($TargetPlatform)))
    {
        throw "Only the <Assemble> and <Resave> commands can run without the <TargetType>, <TargetConfiguration> and <TargetPlatform> parameters."
    }

    # Run the specified command.

    switch ($Command)
    {
        "Assemble"
        {
            Invoke-Process -FilePath $PathToUnrealBuildTool -ArgumentList @("$PathToProject\$Project.uproject", '-ProjectFiles', '-Game', '-Progress')
            # To-Do: The <Win64> parameter should be determined automatically based on the hosting machine's OS.
            Invoke-Process -FilePath $PathToUnrealBuildTool -ArgumentList @("$PathToProject\$Project.uproject", "$($Project)Editor", 'Development', 'Win64', '-WaitMutex', '-FromMsBuild')
            Invoke-Process -FilePath $PathToUnrealEditor -ArgumentList @("$PathToProject\$Project.uproject", '-Run=CompileAllBlueprints', '-IgnoreFolder=/Engine,/RuntimeTests')
        }
        "Resave"
        {
            Invoke-Process -FilePath $PathToUnrealEditor -ArgumentList @("$PathToProject\$Project.uproject", '-Run=ResavePackages', '-ProjectOnly', '-AllowCommandletRendering', '-IgnoreChangelist', '-BuildReflectionCaptures', '-BuildTextureStreaming', '-BuildNavigationData', '-BuildLighting', '-Quality=Preview')
        }
        "Build"
        {
            Invoke-Process -FilePath $PathToUnrealAutomationTool -ArgumentList @('BuildTarget', "-Project=""$PathToProject\$Project.uproject""", "-Target=$Project$TargetType", "-Configuration=$TargetConfiguration", "-Platform=$TargetPlatform")
        }
        "Cook"
        {
            Invoke-Process -FilePath $PathToUnrealAutomationTool -ArgumentList @('BuildCookRun', "-Project=""$PathToProject\$Project.uproject""", "-Target=$Project$TargetType", "-Configuration=$TargetConfiguration", "-Platform=$TargetPlatform", '-Cook', '-SkipEditorContent', '-Compressed')
        }
        "Stage"
        {
            Remove-Item -Recurse -Force "$PathToArtifacts\$Project-$TargetType-$TargetConfiguration-$TargetPlatform"
            Invoke-Process -FilePath $PathToUnrealAutomationTool -ArgumentList @(BuildCookRun, -Project="$PathToProject\$Project.uproject", "-Target=$Project$TargetType", "-Configuration=$TargetConfiguration", "-Platform=$TargetPlatform", '-Stage', "-StagingDirectory=""$PathToArtifacts""", '-SkipCook')
            $PathOld = ""
            $PathNew = "$PathToArtifacts\$Project-$TargetType-$TargetConfiguration-$TargetPlatform"
            switch ($TargetPlatform)
            {
                "Win64"
                {
                    $PathOld = "$PathToArtifacts\Windows$TargetType"
                    break
                }
                "Linux"
                {
                    $PathOld = "$PathToArtifacts\Linux$TargetType"
                    break
                }
                "Mac"
                {
                    # Nothing to do at the moment.
                    break
                }
                default
                {
                    # No default actions are required, at the moment.
                }
            }
            Rename-Item -Path "$PathOld" -NewName "$PathNew"
        }
        "Safeguard"
        {
            $RedistributablesSource = "$PathToEngine\Engine\Binaries\ThirdParty\AppLocalDependencies\$TargetPlatform\*"
            $RedistributablesDestination = "$PathToArtifacts\$Project-$TargetType-$TargetConfiguration-$TargetPlatform\"
            Copy-Item -Force -Recurse -Verbose "$RedistributablesSource" -Destination "$RedistributablesDestination"
        }
        "Archive"
        {
            Remove-Item -Recurse -Force "$PathToArtifacts\$Project-$TargetType-$TargetConfiguration-$TargetPlatform.zip"
            Compress-Archive -Path "$PathToArtifacts\$Project-$TargetType-$TargetConfiguration-$TargetPlatform" -DestinationPath "$PathToArtifacts\$Project-$TargetType-$TargetConfiguration-$TargetPlatform.zip"
        }
        "Deploy"
        {
            Stop-Process -Name "OpenTournamentServer"
            Compress-Archive -Path "$PathToArtifacts\$Project-$TargetType-$TargetConfiguration-$TargetPlatform" -DestinationPath "$PathToArtifacts\$Project-$TargetType-$TargetConfiguration-$TargetPlatform.zip"
            Start-Process -FilePath "$PathToArtifacts\$Project-$TargetType-$TargetConfiguration-$TargetPlatform\OpenTournamentServer.exe" -ArgumentList ['-log']
        }
        default
        {
            throw "The <Command> parameter contains an unexpected string. Valid values are: <Assemble|Resave|Build|Cook|Stage|Safeguard|Archive|Deploy>."
        }
    }
}

function Invoke-Process
{
    <#

    .SYNOPSIS
    
    Starts a process and then captures its output and error streams.

    .DESCRIPTION
    
    Insert description here.

    .EXAMPLE
    
    Insert example here. 

    #>

    [CmdletBinding()]
    param
    (
        # 'C:\Users\Name\Desktop'
        [string]$FilePath,

        # @('A', 'B', 'C')
        [string[]]$ArgumentList
    )

    # Initialize the desired process.
    $Process = New-Object System.Diagnostics.Process
    $Process.StartInfo.FileName = $FilePath
    $Process.StartInfo.Arguments = $ArgumentList
    $Process.StartInfo.UseShellExecute = $false
    $Process.StartInfo.RedirectStandardOutput = $true
    $Process.StartInfo.RedirectStandardError = $true
    
    # Register the output and error events.
    $OutEvent = Register-ObjectEvent -Action { Write-Host "PING_OUT $($Event.SourceEventArgs.Data)" } -InputObject $Process -EventName 'OutputDataReceived'
    $ErrEvent = Register-ObjectEvent -Action { Write-Host "PING_ERR $($Event.SourceEventArgs.Data)" -ForegroundColor DarkRed } -InputObject $Process -EventName 'ErrorDataReceived'

    # Start the process.
    [void]$Process.Start()
    
    # Begin to read the output and error streams.
    $Process.BeginOutputReadLine()
    $Process.BeginErrorReadLine()
    
    # Force the function to wait for the process to exit.
    while (!$Process.HasExited) {}
    
    # Unregister the output and error events.
    Unregister-Event -SourceIdentifier $OutEvent.Name
    Unregister-Event -SourceIdentifier $ErrEvent.Name

    if ($Process.ExitCode -ne 0)
    {
        Write-Error "Process exited with code $($Process.ExitCode)."
    }
}

Main
