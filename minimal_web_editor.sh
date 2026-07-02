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

dev_build=false
clean_build=false
for arg in "$@"; do
    case "$arg" in
        --dev) dev_build=true ;;
        --clean) clean_build=true ;;
    esac
done

if [[ "$dev_build" == true ]]; then
    production=no
    optimize=none
    echo "Building Godot Editor (dev: debug_symbols=yes, use_assertions=yes, lto=none, optimize=speed_trace)..."
else
    production=yes
    optimize=size_extra
    echo "Building Godot Editor..."
fi

scons_args=(
    platform=web
    target=editor
    production="$production"
    optimize="$optimize"
    deprecated=false
    disable_xr=true
    disable_overrides=true
    engine_update_check=false
    cache_path=.cache
    cache_limit=10
    modules_enabled_by_default=false
    build_profile=profile.gdbuild
)

if [[ "$dev_build" == true ]]; then
    scons_args+=(
        debug_symbols=yes
        use_assertions=yes
        lto=none
    )
fi

if [[ "$clean_build" == true ]]; then
    echo "Cleaning previous build..."
    scons --clean "${scons_args[@]}"
fi

scons "${scons_args[@]}"

npx brotli-cli compress -q 5 --br=false bin/.web_zip/godot.editor.wasm
