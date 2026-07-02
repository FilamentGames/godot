#!/usr/bin/env bash

set -eou pipefail

if command -v python3 >/dev/null 2>&1; then
    PYTHON=python3
elif command -v python >/dev/null 2>&1; then
    PYTHON=python
else
    echo "Python is not installed or not on PATH."
    exit 1
fi

echo "Python: $($PYTHON --version)"

if ! command -v scons >/dev/null 2>&1; then
    echo "SCons not found. Installing via pip..."
    "$PYTHON" -m pip install scons
fi

echo "SCons: $(scons --version 2>&1 | head -n1)"
echo

# Requires a manually-set Env Var for the EMSDK Path
if [ -z "${EMSDK:-}" ]; then
    echo "EMSDK environment variable is not set."
    echo "Please set the EMSDK environment variable to the root of the Emscripten SDK Folder."
    exit 1
fi

if [[ "${1:-}" == "-c" ]]; then
    if [ -d "./bin" ]; then
        echo "Removing existing bin directory..."
        rm -rf ./bin
    fi
fi

EMSDK="${EMSDK%/}"

echo "Activating Emscripten At Path: $EMSDK"

"$EMSDK/emsdk" install latest
"$EMSDK/emsdk" activate latest
# shellcheck source=/dev/null
source "$EMSDK/emsdk_env.sh"

if ! command -v emcc >/dev/null 2>&1; then
    echo "Emscripten activation failed: emcc is not on PATH."
    echo "Ensure EMSDK points to a valid Emscripten SDK root and that emsdk_env.sh ran successfully."
    exit 1
fi

echo "Emscripten Activated ($(emcc -v 2>&1 | head -n1))"

echo "Building Godot Editor..."
scons \
    platform=web \
    target=editor \
    production=yes \
    optimize=size_extra \
    deprecated=false \
    disable_xr=true \
    disable_overrides=true \
    engine_update_check=false \
    cache_path=".cache" \
    cache_limit=10 \
    modules_enabled_by_default=false \
    build_profile=profile.gdbuild

npx brotli-cli compress -q 5 --br=false bin/.web_zip/godot.editor.wasm
