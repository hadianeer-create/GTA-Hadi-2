#!/usr/bin/env bash
# Exports the simulation core's city + Hadi poses and renders preview screenshots in headless Chromium.
# Usage: Tools/Preview/build_preview.sh [view[:night] ...]   (views: aerial street spawn beach neon marina southpoint rig)
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "${HERE}/../.." && pwd)"
mkdir -p "${HERE}/.build"
"${CXX:-g++}" -std=c++17 -O2 -I "${ROOT}/Source/HadiViceCity/Public" "${ROOT}"/Source/HadiViceCity/Private/Core/*.cpp "${HERE}/export_preview.cpp" -o "${HERE}/.build/export_preview"
"${HERE}/.build/export_preview" "${HERE}/preview_data.js"
if [ ! -d "${HERE}/node_modules/three" ]; then (cd "${HERE}" && npm install --no-audit --no-fund); fi
node "${HERE}/render.mjs" "${HERE}/shots" "$@"
