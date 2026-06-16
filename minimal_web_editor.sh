#!/usr/bin/env bash

set -eou pipefail

# Requires a manually-set Env Var for the EMSDK Path
if [ -z "$EMSDK" ]; then
    echo "EMSDK environment variable is not set."
    echo "Please set the EMSDK environment variable to the root of the Emscripten SDK Folder."
    exit 1
fi

echo "Activating Emscripten At Path: $EMSDK"

# shellcheck source=/dev/null
"$EMSDK/emsdk" install latest
# shellcheck source=/dev/null
"$EMSDK/emsdk" activate latest
# shellcheck source=/dev/null
"$EMSDK/emsdk_env.sh"

echo "Emscripten Activated"

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
