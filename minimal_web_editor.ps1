# !/usr/bin/env pwsh

if (-not (Get-Command python -ErrorAction SilentlyContinue)) {
    Write-Error "Python is not installed or not on PATH."
    exit 1
}

Write-Host "Python: $(python --version)"

if (-not (Get-Command scons -ErrorAction SilentlyContinue)) {
    Write-Warning "SCons not found. Installing via pip..."
    python -m pip install scons
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Failed to install SCons."
        exit 1
    }
}

Write-Host "SCons: $((scons --version 2>&1 | Select-Object -First 1))`n"

# Requires a manually-set Env Var for the EMSDK Path
if (-not $env:EMSDK) {
    Write-Error "EMSDK environment variable is not set."
    Write-Warning "Please set the EMSDK environment variable to the root of the Emscripten SDK Folder."
    exit 1
}

$emscriptenVersion = if ($env:EMSCRIPTEN_VERSION) { $env:EMSCRIPTEN_VERSION } else { "4.0.11" }

Write-Host "Activating Emscripten At Path: $env:EMSDK"
Write-Host "Emscripten Version: $emscriptenVersion`n"

. "$env:EMSDK\emsdk.ps1" install $emscriptenVersion
. "$env:EMSDK\emsdk.ps1" activate $emscriptenVersion
. "$env:EMSDK\emsdk_env.ps1"

if (-not (Get-Command emcc -ErrorAction SilentlyContinue)) {
    Write-Error "Emscripten activation failed: emcc is not on PATH."
    Write-Warning "Ensure EMSDK points to a valid Emscripten SDK root and that emsdk_env.ps1 ran successfully."
    exit 1
}

Write-Host "Emscripten Activated $((emcc -v 2>&1 | Select-Object -First 1))`n"

$devBuild = $args -contains "--dev"
$cleanBuild = $args -contains "--clean"
if ($devBuild) {
    $production = "no"
    $optimize = "none"
    Write-Host "Building Godot Editor (dev: debug_symbols=yes, use_assertions=yes, lto=none, optimize=speed_trace)...`n"
} else {
    $production = "yes"
    $optimize = "size_extra"
    Write-Host "Building Godot Editor...`n"
}

$sconsArgs = @(
    "platform=web",
    "target=editor",
    "production=$production",
    "optimize=$optimize",
    "deprecated=false",
    "disable_xr=true",
    "disable_overrides=true",
    "engine_update_check=false",
    "cache_path=.cache",
    "cache_limit=10",
    "modules_enabled_by_default=false",
    "build_profile=profile.gdbuild"
)

if ($devBuild) {
    $sconsArgs += @("debug_symbols=yes", "use_assertions=yes", "lto=none")
} else {
    $sconsArgs += @("use_closure_compiler=yes")
}

if ($cleanBuild) {
    Write-Host "Cleaning previous build...`n"
    scons --clean @sconsArgs
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }
}

scons @sconsArgs

npx brotli-cli compress -q 5 --br=false bin\.web_zip\godot.editor.wasm
