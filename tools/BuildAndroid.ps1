[CmdletBinding()]
param(
    [string]$ProjectPath = (Join-Path $PSScriptRoot '..\VelocityRush.uproject'),
    [string]$OutputPath = (Join-Path $PSScriptRoot '..\Build\Android'),
    [string]$UnrealRoot = $env:UE_ROOT
)

$ErrorActionPreference = 'Stop'
$ProjectPath = [System.IO.Path]::GetFullPath($ProjectPath)
$OutputPath = [System.IO.Path]::GetFullPath($OutputPath)
if ([string]::IsNullOrWhiteSpace($UnrealRoot)) {
    throw 'Set UE_ROOT to your licensed Unreal Engine 5.5 installation directory.'
}
$UnrealRoot = [System.IO.Path]::GetFullPath($UnrealRoot)
$RunUAT = Join-Path $UnrealRoot 'Engine/Build/BatchFiles/RunUAT.bat'
$BuildBat = Join-Path $UnrealRoot 'Engine/Build/BatchFiles/Build.bat'
if (-not (Test-Path $ProjectPath)) { throw "Project file not found: $ProjectPath" }
if (-not (Test-Path $RunUAT)) { throw "RunUAT not found: $RunUAT" }
if (-not (Test-Path $BuildBat)) { throw "Build.bat not found: $BuildBat" }

$EditorArgs = @('VelocityRushEditor', 'Win64', 'Development', $ProjectPath, '-WaitMutex', '-NoHotReloadFromIDE')
Write-Host 'Building the Unreal Editor C++ target...'
& $BuildBat @EditorArgs
if ($LASTEXITCODE -ne 0) { throw "Editor build failed with exit code $LASTEXITCODE" }

New-Item -ItemType Directory -Force -Path $OutputPath | Out-Null
$UATArgs = @(
    'BuildCookRun', "-project=$ProjectPath", '-noP4', '-platform=Android', '-cookflavor=ETC2',
    '-clientconfig=Development', '-build', '-cook', '-stage', '-pak', '-package', '-archive',
    "-archivedirectory=$OutputPath", '-utf8output'
)
Write-Host 'Cooking and packaging Android; this requires the SDK/NDK configured by this UE installation.'
& $RunUAT @UATArgs
if ($LASTEXITCODE -ne 0) { throw "Android packaging failed with exit code $LASTEXITCODE" }
$Apk = Get-ChildItem -Path $OutputPath -Filter '*.apk' -File -Recurse | Select-Object -First 1
if (-not $Apk) { throw "UAT finished without producing an APK in $OutputPath" }
Write-Host "APK verified: $($Apk.FullName) ($($Apk.Length) bytes)"
