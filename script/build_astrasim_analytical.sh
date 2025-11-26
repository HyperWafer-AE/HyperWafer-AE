#!/bin/bash
set -e

# Auto-locate the project root
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ASTRA_DIR="${PROJECT_ROOT}/external/astra-sim"

echo "[build] Project root: ${PROJECT_ROOT}"
echo "[build] ASTRA-sim dir: ${ASTRA_DIR}"

if [ ! -d "${ASTRA_DIR}" ]; then
  echo "[build][ERROR] external/astra-sim not found. Please check your repo layout."
  exit 1
fi

cd "${ASTRA_DIR}"

# Optional: remove the old analytical build (helpful when rebuilding)
if [ -d "build/astra_analytical/build" ]; then
  echo "[build] Removing old build/astra_analytical directory..."
  rm -rf build/astra_analytical/build
fi

echo "[build] Running ASTRA-sim analytical build script..."
./build/astra_analytical/build.sh

echo "[build] Done."
echo "[build] Binary should be at: build/astra_analytical/build/bin/AstraSim_Analytical_Congestion_Aware"
