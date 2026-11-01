#!/usr/bin/env bash
# Builds and runs the engine-agnostic simulation core tests (no Unreal Engine required).
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
OUT="${ROOT}/Tools/CoreTests/.build"
mkdir -p "${OUT}"
CXX="${CXX:-g++}"
"${CXX}" -std=c++17 -O1 -g -Wall -Wextra -Wshadow -Werror \
  -I "${ROOT}/Source/HadiViceCity/Public" \
  "${ROOT}"/Source/HadiViceCity/Private/Core/*.cpp \
  "${ROOT}/Tools/CoreTests/HadiCoreTests.cpp" \
  -o "${OUT}/hadi_core_tests"
"${OUT}/hadi_core_tests"
