#!/bin/bash
# SPDX-License-Identifier: MIT
# Copyright (c) 2025 HyperWafer Authors
set -e

# Auto-locate the project root.
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PYTHON="${PYTHON:-python}"

# ====== Tunable parameters (override via env vars before running) ======
MATRIX_SELECTOR="${MATRIX_SELECTOR:-pesa}"   # e.g., Gaertner/pesa or any ssgetpy selector.
NUM_PARTS="${NUM_PARTS:-64}"                 # tiles / NPUs
ELEM_BYTES="${ELEM_BYTES:-4}"                # bytes per element in B (float32 by default)

# SuiteSparse download/extract directory.
SS_DEST_DIR="${SS_DEST_DIR:-${HOME}/.cache/ssgetpy}"

# Mt-KaHyPar settings.
if [ -z "${MTK_BIN:-}" ]; then
  if command -v MtKaHyPar >/dev/null 2>&1; then
    MTK_BIN="$(command -v MtKaHyPar)"
  else
    echo "[run_pesa][ERROR] MTK_BIN is unset and MtKaHyPar was not found in PATH." >&2
    echo "            Export MTK_BIN=/path/to/MtKaHyPar before rerunning." >&2
    exit 1
  fi
fi
MTK_EPS="${MTK_EPS:-0.03}"
if command -v nproc >/dev/null 2>&1; then
  DEFAULT_THREADS="$(nproc --all)"
else
  DEFAULT_THREADS=64
fi
MTK_THREADS="${MTK_THREADS:-${DEFAULT_THREADS}}"
MTK_PRESET="${MTK_PRESET:-quality}"
if [ ! -x "${MTK_BIN}" ]; then
  echo "[run_pesa][ERROR] MtKaHyPar binary not found at ${MTK_BIN}." >&2
  exit 1
fi

# Chakra converter.
CHAKRA_BIN="${CHAKRA_BIN:-chakra_converter}"
NUM_PASSES="${NUM_PASSES:-1}"

# Astra-sim binary (built under external/astra-sim by default).
ASTRASIM_BIN="${ASTRASIM_BIN:-${PROJECT_ROOT}/external/astra-sim/build/astra_analytical/build/bin/AstraSim_Analytical_Congestion_Aware}"
if [ ! -x "${ASTRASIM_BIN}" ]; then
  echo "[run_pesa][WARN] AstraSim binary not found at ${ASTRASIM_BIN}." >&2
  echo "               Build external/astra-sim or override ASTRASIM_BIN." >&2
fi

# Astra-sim config files (override if you want different configs).
SYSTEM_CONFIG="${SYSTEM_CONFIG:-${PROJECT_ROOT}/external/astra-sim/examples/system/native_collectives/Ring_4chunks.json}"
NETWORK_CONFIG="${NETWORK_CONFIG:-${PROJECT_ROOT}/external/astra-sim/examples/network/analytical/Mesh_64cores.yml}"
REMOTE_MEM_CONFIG="${REMOTE_MEM_CONFIG:-${PROJECT_ROOT}/external/astra-sim/examples/remote_memory/analytical/no_memory_expansion.json}"

# Output directory for all intermediate artifacts.
WORKDIR="${WORKDIR:-${PROJECT_ROOT}/spgemm_out}"
# =====================================================================

echo "[run_pesa] PROJECT_ROOT      = ${PROJECT_ROOT}"
echo "[run_pesa] MATRIX_SELECTOR   = ${MATRIX_SELECTOR}"
echo "[run_pesa] NUM_PARTS         = ${NUM_PARTS}"
echo "[run_pesa] ASTRA-sim binary  = ${ASTRASIM_BIN}"
echo "[run_pesa] SYSTEM_CONFIG     = ${SYSTEM_CONFIG}"
echo "[run_pesa] NETWORK_CONFIG    = ${NETWORK_CONFIG}"
echo "[run_pesa] REMOTE_MEM_CONFIG = ${REMOTE_MEM_CONFIG}"
echo "[run_pesa] WORKDIR           = ${WORKDIR}"

echo "[run_pesa] SS_DEST_DIR       = ${SS_DEST_DIR}"
echo "[run_pesa] MtKaHyPar binary  = ${MTK_BIN}"
echo "[run_pesa] MtKaHyPar threads = ${MTK_THREADS}"

# Ensure the output directory exists.
mkdir -p "${WORKDIR}"

# Run the pipeline end-to-end.
${PYTHON} "${PROJECT_ROOT}/src/spgemm_full_pipeline.py" \
  --ss-dest-dir "${SS_DEST_DIR}" \
  --num-parts "${NUM_PARTS}" \
  --elem-bytes "${ELEM_BYTES}" \
  --mtk-bin "${MTK_BIN}" \
  --mtk-eps "${MTK_EPS}" \
  --mtk-threads "${MTK_THREADS}" \
  --mtk-preset "${MTK_PRESET}" \
  --chakra-bin "${CHAKRA_BIN}" \
  --num-passes "${NUM_PASSES}" \
  --astrasim-bin "${ASTRASIM_BIN}" \
  --system-config "${SYSTEM_CONFIG}" \
  --network-config "${NETWORK_CONFIG}" \
  --remote-mem-config "${REMOTE_MEM_CONFIG}" \
  --workdir "${WORKDIR}" \
  --b-owner-mode rowblock \
  --matrix-selector "${MATRIX_SELECTOR}"
