param(
    [string]$DatasetsRoot = (Join-Path $PSScriptRoot "..\models"),
    [string]$RiverPath = (Join-Path $PSScriptRoot "..\models\river"),
    [string]$ExePath = (Join-Path $PSScriptRoot "..\Release\FeedbackRTSfM.exe"),
    [string]$SettingsPath = (Join-Path $PSScriptRoot "..\Release\settings.ini"),
    [string]$RiverName = "river",
    [string]$OutputName = "mesh.ply",
    [string[]]$Dataset,
    [switch]$Overwrite,
    [int]$LoadTimeoutSec = 300,
    [int]$MeshTimeoutSec = 7200,
    [int]$StableSeconds = 10
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

Add-Type -AssemblyName UIAutomationClient
Add-Type -AssemblyName UIAutomationTypes

function Resolve-FullPath {
    param([Parameter(Mandatory = $true)][string]$Path)
    $executionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($Path)
}

function Test-ColmapModelDir {
    param([Parameter(Mandatory = $true)][string]$Path)

    $textModel = @("cameras.txt", "images.txt", "points3D.txt")
    $binModel = @("cameras.bin", "images.bin", "points3D.bin")
    $hasText = ($textModel | ForEach-Object { Test-Path -LiteralPath (Join-Path $Path $_) }) -notcontains $false
    $hasBin = ($binModel | ForEach-Object { Test-Path -LiteralPath (Join-Path $Path $_) }) -notcontains $false
    return ($hasText -or $hasBin)
}

function Get-DatasetDirs {
    param(
        [Parameter(Mandatory = $true)][string]$Root,
        [string[]]$NamesOrPaths,
        [Parameter(Mandatory = $true)][string]$RiverPath
    )

    if ($NamesOrPaths -and $NamesOrPaths.Count -gt 0) {
        foreach ($item in $NamesOrPaths) {
            $candidate = if ([System.IO.Path]::IsPathRooted($item)) { $item } else { Join-Path $Root $item }
            $full = Resolve-FullPath $candidate
            if (-not (Test-Path -LiteralPath $full -PathType Container)) {
                throw "Dataset directory not found: $item"
            }
            if (-not (Test-ColmapModelDir $full)) {
                throw "Dataset is not a COLMAP sparse model directory: $full"
            }
            Get-Item -LiteralPath $full
        }
        return
    }

    Get-ChildItem -LiteralPath $Root -Directory |
        Where-Object { $_.FullName -ne $RiverPath } |
        Where-Object { Test-ColmapModelDir $_.FullName } |
        Sort-Object FullName
}

function Write-FixedRiverSettings {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)][string]$RiverName,
        [Parameter(Mandatory = $true)][string]$OutputName
    )

    $content = @(
        "% Basic settings",
        "[Base]",
        "% The sparse model path.",
        "model_path = ..\models\$RiverName",
        "% The output ply mesh model file path.",
        "output_path = ..\models\$RiverName\$OutputName"
    )
    Set-Content -LiteralPath $Path -Value $content -Encoding ASCII
}

function Find-WindowForProcess {
    param(
        [Parameter(Mandatory = $true)][int]$ProcessId,
        [Parameter(Mandatory = $true)][int]$TimeoutSec
    )

    $deadline = (Get-Date).AddSeconds($TimeoutSec)
    $root = [System.Windows.Automation.AutomationElement]::RootElement
    $condition = New-Object System.Windows.Automation.PropertyCondition(
        [System.Windows.Automation.AutomationElement]::ProcessIdProperty,
        $ProcessId
    )

    while ((Get-Date) -lt $deadline) {
        $window = $root.FindFirst([System.Windows.Automation.TreeScope]::Children, $condition)
        if ($window -ne $null) {
            return $window
        }
        Start-Sleep -Milliseconds 500
    }

    throw "Timed out waiting for application window."
}

function Invoke-UiElementByName {
    param(
        [Parameter(Mandatory = $true)]$Window,
        [Parameter(Mandatory = $true)][string]$Name
    )

    $nameCondition = New-Object System.Windows.Automation.PropertyCondition(
        [System.Windows.Automation.AutomationElement]::NameProperty,
        $Name
    )
    $element = $Window.FindFirst([System.Windows.Automation.TreeScope]::Descendants, $nameCondition)
    if ($element -eq $null) {
        throw "Could not find UI element named '$Name'."
    }

    $pattern = $null
    if (-not $element.TryGetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern, [ref]$pattern)) {
        throw "UI element '$Name' does not support InvokePattern."
    }
    $pattern.Invoke()
}

function Wait-ProcessCpuQuiet {
    param(
        [Parameter(Mandatory = $true)][System.Diagnostics.Process]$Process,
        [Parameter(Mandatory = $true)][int]$TimeoutSec,
        [int]$QuietSamples = 3
    )

    $deadline = (Get-Date).AddSeconds($TimeoutSec)
    $quiet = 0
    $lastCpu = $Process.TotalProcessorTime
    Start-Sleep -Seconds 1

    while ((Get-Date) -lt $deadline) {
        $Process.Refresh()
        if ($Process.HasExited) {
            throw "Application exited unexpectedly while waiting."
        }

        $currentCpu = $Process.TotalProcessorTime
        $deltaMs = ($currentCpu - $lastCpu).TotalMilliseconds
        $lastCpu = $currentCpu

        if ($deltaMs -lt 100) {
            $quiet += 1
            if ($quiet -ge $QuietSamples) {
                return
            }
        }
        else {
            $quiet = 0
        }
        Start-Sleep -Seconds 1
    }

    throw "Timed out waiting for process to become idle."
}

function Wait-FileStable {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)][int]$TimeoutSec,
        [Parameter(Mandatory = $true)][int]$StableSeconds,
        [Parameter(Mandatory = $true)][System.Diagnostics.Process]$Process
    )

    $deadline = (Get-Date).AddSeconds($TimeoutSec)
    $lastLength = -1
    $stableFor = 0

    while ((Get-Date) -lt $deadline) {
        $Process.Refresh()
        if ($Process.HasExited) {
            if (Test-Path -LiteralPath $Path -PathType Leaf) {
                return
            }
            throw "Application exited before mesh was written."
        }

        if (Test-Path -LiteralPath $Path -PathType Leaf) {
            $length = (Get-Item -LiteralPath $Path).Length
            if ($length -gt 0 -and $length -eq $lastLength) {
                $stableFor += 1
                if ($stableFor -ge $StableSeconds) {
                    return
                }
            }
            else {
                $lastLength = $length
                $stableFor = 0
            }
        }

        Start-Sleep -Seconds 1
    }

    throw "Timed out waiting for mesh output: $Path"
}

function Remove-RiverJunction {
    param([Parameter(Mandatory = $true)][string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        return
    }

    $item = Get-Item -LiteralPath $Path -Force
    if ($item.LinkType -ne "Junction") {
        throw "Refusing to remove '$Path' because it is not a junction."
    }

    [System.IO.Directory]::Delete($item.FullName)
}

$root = Resolve-FullPath $DatasetsRoot
$exe = Resolve-FullPath $ExePath
$settings = Resolve-FullPath $SettingsPath
$workingDir = Split-Path -Parent $exe
$riverPath = Resolve-FullPath $RiverPath

if (Get-Process FeedbackRTSfM -ErrorAction SilentlyContinue) {
    throw "Please close FeedbackRTSfM.exe before running this script. It needs to temporarily replace '$riverPath' with a directory junction."
}
if (-not (Test-Path -LiteralPath $exe -PathType Leaf)) {
    throw "Executable not found: $exe"
}
if (-not (Test-Path -LiteralPath $settings -PathType Leaf)) {
    throw "settings.ini not found: $settings"
}
if (-not (Test-Path -LiteralPath $riverPath -PathType Container)) {
    throw "River directory not found: $riverPath"
}

$datasets = @(Get-DatasetDirs -Root $root -NamesOrPaths $Dataset -RiverPath $riverPath)
if ($datasets.Count -eq 0) {
    throw "No COLMAP model directories found under: $root"
}

$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$riverParent = Split-Path -Parent $riverPath
$riverBackupPath = Join-Path $riverParent "${RiverName}__batch_backup_$timestamp"
$originalSettings = Get-Content -LiteralPath $settings -Raw
$failures = New-Object System.Collections.Generic.List[string]

try {
    Rename-Item -LiteralPath $riverPath -NewName (Split-Path -Leaf $riverBackupPath)
    Write-FixedRiverSettings -Path $settings -RiverName $RiverName -OutputName $OutputName

    foreach ($dir in $datasets) {
        $outputPath = Join-Path $dir.FullName $OutputName
        if ((Test-Path -LiteralPath $outputPath -PathType Leaf) -and -not $Overwrite) {
            Write-Host "[SKIP] $($dir.Name) already has $OutputName"
            continue
        }

        if ((Test-Path -LiteralPath $outputPath -PathType Leaf) -and $Overwrite) {
            Remove-Item -LiteralPath $outputPath -Force
        }

        Write-Host "[RUN ] $($dir.FullName)"
        $proc = $null

        try {
            New-Item -ItemType Junction -Path $riverPath -Target $dir.FullName | Out-Null

            $proc = Start-Process -FilePath $exe -WorkingDirectory $workingDir -PassThru
            $window = Find-WindowForProcess -ProcessId $proc.Id -TimeoutSec 60

            Invoke-UiElementByName -Window $window -Name "Load Model"
            Wait-ProcessCpuQuiet -Process $proc -TimeoutSec $LoadTimeoutSec

            Invoke-UiElementByName -Window $window -Name "Calculate Mesh"
            Wait-FileStable -Path $outputPath -TimeoutSec $MeshTimeoutSec -StableSeconds $StableSeconds -Process $proc

            Write-Host "[DONE] $outputPath"
        }
        catch {
            $message = "[FAIL] $($dir.FullName): $($_.Exception.Message)"
            $failures.Add($message)
            Write-Warning $message
        }
        finally {
            if ($proc -ne $null) {
                $proc.Refresh()
                if (-not $proc.HasExited) {
                    $proc.CloseMainWindow() | Out-Null
                    if (-not $proc.WaitForExit(5000)) {
                        Stop-Process -Id $proc.Id -Force
                    }
                }
            }
            if (Test-Path -LiteralPath $riverPath) {
                Remove-RiverJunction -Path $riverPath
            }
        }
    }
}
finally {
    if (Test-Path -LiteralPath $riverPath) {
        Remove-RiverJunction -Path $riverPath
    }
    if (Test-Path -LiteralPath $riverBackupPath -PathType Container) {
        Rename-Item -LiteralPath $riverBackupPath -NewName $RiverName
    }
    Set-Content -LiteralPath $settings -Value $originalSettings -Encoding ASCII
}

if ($failures.Count -gt 0) {
    Write-Host ""
    Write-Host "Failed datasets:"
    $failures | ForEach-Object { Write-Host $_ }
    exit 1
}

Write-Host ""
Write-Host "All requested datasets finished."
