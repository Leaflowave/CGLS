#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD="$ROOT/build"
CXX="${CXX:-g++}"

mkdir -p "$BUILD"

"$CXX" -std=c++17 -O3 \
  "$ROOT/third_party/rls_gis/Code/main.cpp" \
  "$ROOT/third_party/rls_gis/Code/Graph.cpp" \
  "$ROOT/third_party/rls_gis/Code/Utility.cpp" \
  -o "$BUILD/RLS"

"$CXX" -std=c++17 -O3 -DNDEBUG \
  -DPARAM_TIME_LIMIT=3600.0 \
  -DPARAM_RLS_MWIS_SOLVER='"./build/RLS"' \
  -DPARAM_RLS_MWIS_MAX_COLUMNS=1000 \
  -DPARAM_RLS_MASTER_PERIOD=25 \
  -DPARAM_RLS_MASTER_MAX_CALLS=3 \
  -DPARAM_RLS_MWIS_CALL_TIME_LIMIT=3.0 \
  -DPARAM_RLS_MASTER_ENABLED=1 \
  "$ROOT/src/main.cpp" \
  "$ROOT/src/common_func_def.cpp" \
  "$ROOT/src/local_search.cpp" \
  -o "$BUILD/IUC"

echo "Build complete:"
echo "  $BUILD/IUC"
echo "  $BUILD/RLS"
echo "Total IUC cutoff: 3600 seconds"
echo "Master mode: hard-GIS/MWIS"
