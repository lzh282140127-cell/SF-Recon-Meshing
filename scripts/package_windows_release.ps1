param(
    [string]$RuntimeDir = (Join-Path $PSScriptRoot "..\Release"),
    [string]$OutputDir = (Join-Path $PSScriptRoot "..\dist"),
    [string]$ArchiveName = "SF-Recon-Meshing-Windows-x64.zip"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Resolve-FullPath {
    param([Parameter(Mandatory = $true)][string]$Path)
    $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($Path)
}

$repoRoot = Resolve-FullPath (Join-Path $PSScriptRoot "..")
$runtime = Resolve-FullPath $RuntimeDir
$output = Resolve-FullPath $OutputDir

if (-not (Test-Path -LiteralPath $runtime -PathType Container)) {
    throw "Runtime directory not found: $runtime"
}

$runtimeFiles = @(
    "FeedbackRTSfM.exe",
    "settings.ini",
    "boost_filesystem-vc142-mt-x64-1_86.dll",
    "D3Dcompiler_47.dll",
    "gflags.dll",
    "glog.dll",
    "gmp-10.dll",
    "libEGL.dll",
    "libGLESV2.dll",
    "opengl32sw.dll",
    "Qt5Core.dll",
    "Qt5Gui.dll",
    "Qt5Svg.dll",
    "Qt5Widgets.dll",
    "tbb12.dll"
)
$pluginDirs = @("iconengines", "imageformats", "platforms", "styles", "translations")

foreach ($name in $runtimeFiles) {
    $path = Join-Path $runtime $name
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw "Required runtime file is missing: $path"
    }
}
foreach ($name in $pluginDirs) {
    $path = Join-Path $runtime $name
    if (-not (Test-Path -LiteralPath $path -PathType Container)) {
        throw "Required plugin directory is missing: $path"
    }
}

[void][System.IO.Directory]::CreateDirectory($output)
$tempRoot = [System.IO.Path]::GetFullPath(
    (Join-Path ([System.IO.Path]::GetTempPath()) ("sfrecon-meshing-" + [guid]::NewGuid().ToString("N")))
)
$systemTemp = [System.IO.Path]::GetFullPath([System.IO.Path]::GetTempPath())
if (-not $tempRoot.StartsWith($systemTemp, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "Unsafe temporary path: $tempRoot"
}

$packageRoot = Join-Path $tempRoot "SF-Recon-Meshing-Windows-x64"
$packageRuntime = Join-Path $packageRoot "Release"
$packageModel = Join-Path $packageRoot "models\river"
$archivePath = Join-Path $output $ArchiveName

try {
    [void][System.IO.Directory]::CreateDirectory($packageRoot)
    [void][System.IO.Directory]::CreateDirectory($packageRuntime)
    [void][System.IO.Directory]::CreateDirectory($packageModel)

    foreach ($name in $runtimeFiles) {
        Copy-Item -LiteralPath (Join-Path $runtime $name) -Destination $packageRuntime
    }
    foreach ($name in $pluginDirs) {
        Copy-Item -LiteralPath (Join-Path $runtime $name) -Destination $packageRuntime -Recurse
    }

    Copy-Item -LiteralPath (Join-Path $repoRoot "LICENSE") -Destination $packageRoot
    Copy-Item -LiteralPath (Join-Path $repoRoot "THIRD_PARTY_NOTICES.md") -Destination $packageRoot
    Copy-Item -LiteralPath (Join-Path $repoRoot "packaging\README-Windows.md") -Destination $packageRoot
    Copy-Item -LiteralPath (Join-Path $repoRoot "models\river\README.md") -Destination $packageModel

    if (Test-Path -LiteralPath $archivePath) {
        Remove-Item -LiteralPath $archivePath
    }
    Compress-Archive -LiteralPath $packageRoot -DestinationPath $archivePath -CompressionLevel Optimal

    $hash = Get-FileHash -Algorithm SHA256 -LiteralPath $archivePath
    $checksumPath = "$archivePath.sha256"
    "$($hash.Hash)  $([System.IO.Path]::GetFileName($archivePath))" |
        Set-Content -LiteralPath $checksumPath -Encoding ascii

    Write-Host "Created: $archivePath"
    Write-Host "SHA256: $($hash.Hash)"
}
finally {
    if (Test-Path -LiteralPath $tempRoot) {
        Remove-Item -LiteralPath $tempRoot -Recurse -Force
    }
}

