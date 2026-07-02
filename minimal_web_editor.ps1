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

Write-Host "Activating Emscripten At Path: $env:EMSDK`n"

. "$env:EMSDK\emsdk.ps1" install latest
. "$env:EMSDK\emsdk.ps1" activate latest
. "$env:EMSDK\emsdk_env.ps1"

Write-Host "`nEmscripten Activated`n"

$devBuild = $args -contains "-dev"
if ($devBuild) {
    $production = "no"
    $optimize = "none"
    Write-Host "Building Godot Editor (dev: production=no, optimize=none)...`n"
} else {
    $production = "yes"
    $optimize = "size_extra"
    Write-Host "Building Godot Editor...`n"
}

scons `
    platform=web `
    target=editor `
    production=$production `
    optimize=$optimize `
    deprecated=false `
    disable_xr=true `
    disable_overrides=true `
    engine_update_check=false `
    cache_path=".cache" `
    cache_limit=10 `
    modules_enabled_by_default=false `
    build_profile=profile.gdbuild

npx brotli-cli compress -q 5 --br=false bin\.web_zip\godot.editor.wasm
