#!/usr/bin/env sh
# Just a utility script for when I'm on Linux tehe
scons \
    platform=linuxbsd \
    target=editor \
    dev_build=yes \
    deprecated=false \
    disable_xr=true \
    disable_overrides=true \
    engine_update_check=false \
    cache_path=".cache" \
    cache_limit=10 \
    modules_enabled_by_default=false \
    build_profile=profile.gdbuild
