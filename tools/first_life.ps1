<#
    Copyright (C) 2026 Matej Gomboc https://github.com/ai-quokka-wannabe/rc-worm

    This program is free software: you can redistribute it and/or modify it under the terms of
    the GNU General Public License as published by the Free Software Foundation, either version
    3 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
    without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with this program.
    If not, see https://www.gnu.org/licenses/.
#>

<#
.SYNOPSIS
    A life on the Grid: Master Control recording to a Disk and a log, the Grid's window to watch
    it in, the Grid hosting rc-worm with its panel - and you at the keys. When you close Master
    Control's window (or press Ctrl+C in it), the host leaves, and Clu re-simulates the log and
    says whether the Disk is the world it describes.

.DESCRIPTION
    Three processes in three console windows, so every log is readable as it happens:
      1. master-control <port> --disk <out>/life.disk --log <out>/life.log
      2. TronGridLite 127.0.0.1:<port> --window            (WASD to fly, mouse to look, Tab to capture the cursor)
      3. TronGridLite 127.0.0.1:<port> --program rc_worm   (the panel opens from inside it; W/S A/D Space X)
    The Grid's programs/ directory must hold the deployed worm - `cmake --install <build> --prefix <where>`
    and copy <where>/programs/* there (docs/PANEL.md § Deployment). No Qt on the PATH is needed.

.PARAMETER Grid
    Path to TronGridLite.exe.
.PARAMETER MasterControl
    Path to master-control.exe.
.PARAMETER Out
    Directory for the Disk and the log (created). Default: .\life-<timestamp>.
.PARAMETER Port
    The port Master Control listens on. Default 47000.

.EXAMPLE
    .\tools\first_life.ps1 -Grid ..\tron-grid-lite\build\windows-msvc\src\Release\TronGridLite.exe -MasterControl ..\master-control\target\release\master-control.exe
#>
param(
    [Parameter(Mandatory = $true)] [string] $Grid,
    [Parameter(Mandatory = $true)] [string] $MasterControl,
    [string] $Out = "",
    [int] $Port = 47000
)

$ErrorActionPreference = 'Stop'

$Grid = (Resolve-Path $Grid).Path
$MasterControl = (Resolve-Path $MasterControl).Path
if ($Out -eq "") { $Out = Join-Path (Get-Location) ("life-" + (Get-Date -Format 'yyyyMMdd-HHmmss')) }
New-Item -ItemType Directory -Force -Path $Out | Out-Null
$Out = (Resolve-Path $Out).Path
$disk = Join-Path $Out 'life.disk'
$log = Join-Path $Out 'life.log'

$programs = Join-Path (Split-Path $Grid) 'programs'
if (-not (Test-Path (Join-Path $programs 'rc_worm.dll'))) {
    throw "No rc_worm.dll in $programs - deploy the worm there first (docs/PANEL.md § Deployment)."
}
if (-not (Test-Path (Join-Path $programs 'platforms\qwindows.dll'))) {
    throw "No platforms\qwindows.dll beside the worm in $programs - the panel would refuse to start (windeployqt at install puts it there)."
}

Write-Host "Recording to $disk, logging to $log."
$mc = Start-Process -FilePath $MasterControl -ArgumentList @("$Port", '--disk', "`"$disk`"", '--log', "`"$log`"") -PassThru
Start-Sleep -Seconds 1
if ($mc.HasExited) { throw "Master Control exited at once (exit $($mc.ExitCode)) - is port $Port free?" }

$window = Start-Process -FilePath $Grid -ArgumentList @("127.0.0.1:$Port", '--window') -PassThru
Start-Sleep -Seconds 2
$host1 = Start-Process -FilePath $Grid -ArgumentList @("127.0.0.1:$Port", '--program', 'rc_worm') -PassThru

Write-Host ""
Write-Host "The Grid's window: WASD to fly, mouse to look, Tab captures the cursor."
Write-Host "The panel (from the host): W/S forward, A/D turn, Space call, X brake; sliders hold a course."
Write-Host "Close Master Control's window (or Ctrl+C in it) to end the life."
Write-Host ""

$mc.WaitForExit()
Write-Host "Master Control stopped; the host leaves on its own."
$host1.WaitForExit(15000) | Out-Null
if (-not $host1.HasExited) { Stop-Process -Id $host1.Id -Force }
if (-not $window.HasExited) { Stop-Process -Id $window.Id -Force }

Write-Host ""
Write-Host "Clu:"
& $MasterControl clu "$log" "$disk"
Write-Host ""
Write-Host "The life is in $Out."
