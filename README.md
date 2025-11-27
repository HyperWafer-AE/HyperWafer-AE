# HyperWafer: Communication-Aware Sparse Matrix Multiplication on Wafer-Scale Chips

## 1. Project Overview

This repository contains a reproducible pipeline to evaluate the communication behavior of sparse matrix–matrix multiplication (SpGEMM) under different task-to-tile mappings on wafer-style 2D meshes.

The code is designed to compare:

- **WaferSpMM** – a row-block baseline mapping.
- **HyperWafer** – a hypergraph-based mapping that co-locates tasks sharing input rows of (B), aiming to reduce communication volume and link hotspots.

The pipeline integrates:

- SuiteSparse Matrix Collection (via `ssgetpy`)
- Hypergraph partitioning (via [Mt-KaHyPar](https://github.com/kahypar/mt-kahypar))
- A 2D mesh oracle for exact per-link communication simulation
- MICRO-level workload generation for [Chakra](https://github.com/astra-sim/chakra)
- Analytical network simulation via [ASTRA-sim](https://github.com/astra-sim/astra-sim)

---

## 2. Repository layout

A typical layout of this repository:

```text
.
├── src/
│   └── spgemm_full_pipeline.py       # main Python CLI pipeline
├── external/
│   └── astra-sim/                    # bundled ASTRA-sim snapshot (analytical backend)
├── scripts/
│   └── build_astrasim_analytical.sh  # helper to build ASTRA-sim analytical backend
├── examples/
│   └── run_pesa.sh                   # example experiment on Gaertner/pesa
├── README.md
├── LICENSE
├── requirements.txt
└── .gitignore
```

Notes:

- `external/astra-sim/` is a snapshot of the ASTRA-sim codebase (with minor modifications) placed directly under `external/`.  
  Its own `LICENSE` and headers are preserved.
- The Python pipeline does **not** require rebuilding ASTRA-sim for every run; only when you change the ASTRA-sim sources.

---

## 3. Environment & dependencies

| Layer | Requirement | Notes |
|-------|-------------|-------|
| OS | Ubuntu 22.04 LTS (tested) | Any recent Debian/Ubuntu with `glibc >= 2.31` is fine. |
| System packages | `git`, `cmake >= 3.15`, `g++ >= 11`, `make`, `python3`, `pip` | `sudo apt install git build-essential cmake python3 python3-venv` |
| Python | 3.9 – 3.11 | Create a venv/conda env to isolate packages. |
| Python deps | `numpy`, `scipy`, `ssgetpy`, `tqdm`, `PyYAML` | `pip install -r requirements.txt` |
| Mt-KaHyPar | Latest `main` build | Needed for hypergraph partitioning (`--mtk-bin`). |
| Chakra converter | `chakra_converter` CLI | Install via `pip install chakra-tools` or the Chakra repo. |
| ASTRA-sim (analytical) | Snapshot under `external/astra-sim/` | Build via `scripts/build_astrasim_analytical.sh`. |


## 4. Building ASTRA-sim (analytical backend)

From the repository root:

```bash
bash scripts/build_astrasim_analytical.sh
```

This script:

1. Locates `external/astra-sim/`.

2. Removes any existing `build/astra_analytical` directory (for a clean build).

3. Runs:

   ```bash
   ./build/astra_analytical/build.sh
   ```

inside `external/astra-sim`.

After a successful build, the main binary is at:

```text
external/astra-sim/build/astra_analytical/build/bin/AstraSim_Analytical_Congestion_Aware
```

You can override this path via `--astrasim-bin` when running the Python pipeline.

---

## 5. Quick start: PESA example

This repository provides an example script that runs the full pipeline on the SuiteSparse matrix `Gaertner/pesa`:

- Script: `examples/run_pesa.sh`

### 5.1 Configure the script (if needed)

Open `examples/run_pesa.sh` and adjust the following paths if they differ on your system:

- `SS_DEST_DIR` – where SuiteSparse matrices are downloaded (e.g. `./mat`)
- `MTK_BIN` – path to the `MtKaHyPar` executable
- `ASTRASIM_BIN` – path to `AstraSim_Analytical_Congestion_Aware` (usually under `external/astra-sim/...`)
- Optional: system/network/remote-memory config paths for ASTRA-sim

### 5.2 Run the example

From the repository root:

```bash
chmod +x examples/run_pesa.sh
bash examples/run_pesa.sh
```

If everything is configured correctly, you should see logs from:

- SuiteSparse matrix download (`ssgetpy`)
- Hypergraph construction and partitioning (Mt-KaHyPar)
- 2D mesh oracle statistics (bytes, GB-hop, peak link)
- MICRO workload generation (WAFERSPMM vs HyperWafer)
- Chakra conversion (`chakra_converter`)
- ASTRA-sim analytical simulation

At the end, the script prints a summary similar to:

```text
========== SUMMARY (FULL PIPELINE) ==========
Matrix: Gaertner/pesa
A: shape=(...), nnz=...
B: shape=(...), nnz=...
num_parts: 64

WaferSpMM:
  Oracle total_bytes      = ...
  Oracle GB-hop           = ...
  Oracle peak_link_bytes  = ...
  MICRO total_bytes       = ...
  MICRO num_layers        = ...
  AstraSim Comm time      = ...

HyperWafer:
  Oracle total_bytes      = ...
  Oracle GB-hop           = ...
  Oracle peak_link_bytes  = ...
  MICRO total_bytes       = ...
  MICRO num_layers        = ...
  AstraSim Comm time      = ...

Volume reduction (WaferSpMM / HyperWafer):      ...
GB-hop reduction (WaferSpMM / HyperWafer):      ...
Peak-link reduction (WaferSpMM / HyperWafer):   ...
Comm-time speedup (AstraSim, WaferSpMM/HyperWafer): ...
==============================================
```

This gives you an end-to-end comparison of the baseline row-block mapping vs the HyperWafer hypergraph mapping, both in terms of oracle metrics and ASTRA-sim communication time.

---
## 6. CLI usage of `spgemm_full_pipeline.py`

Although `examples/run_pesa.sh` is the recommended entry point for reproducibility, you can also invoke the pipeline directly:

```bash
python src/spgemm_full_pipeline.py [arguments...]
```

Key arguments (non-exhaustive):

- `--matrix-selector`  
  SuiteSparse matrix selector. Supports:
  - `1234` → exact id
  - `Group/Name` → group/name (e.g. `Gaertner/pesa`)
  - `foo` → substring search on matrix name

- `--ss-dest-dir`  
  Directory where SuiteSparse matrices are downloaded and cached.

- `--num-parts`  
  Number of tiles / NPUs / partitions (e.g., 16, 64). Also the number of nodes in the 2D mesh.

- `--elem-bytes`  
  Size of a numeric element in bytes (e.g. 4 for `float32`).

- `--mtk-bin` / `--mtk-eps` / `--mtk-threads` / `--mtk-preset`  
  Control Mt-KaHyPar partitioning.

- `--b-owner-mode`  
  Strategy for picking the owner of row \(B_{k,:}\) (e.g. `rowblock`).

- `--workdir`  
  Directory for all intermediate outputs (hypergraphs, partitions, workloads, ET traces, logs).

- `--astrasim-bin` / `--system-config` / `--network-config` / `--remote-mem-config`  
  ASTRA-sim binary and configuration files.

- `--chakra-bin` / `--num-passes`  
  Chakra converter binary and number of passes to model.

Run:

```bash
python src/spgemm_full_pipeline.py -h
```

for the complete list and detailed descriptions.

---

## 7. Methodology notes

- The **2D mesh oracle** is responsible for:
  - Exact point-to-point simulation of traffic induced by Gustavson SpGEMM.
  - Computing total bytes, GB-hop, and per-link loads for each mapping.

- The **MICRO workloads** are an intermediate abstraction:
  - Oracle traffic is grouped into “phases” and modeled as `ALLGATHER` collectives.
  - This allows us to use standard Chakra + ASTRA-sim tooling while preserving relative trends.

- The pipeline is designed to explore:
  - How hypergraph-based mappings reduce:
    - Cross-tile reuse of hot rows of \(B\)
    - Aggregate bytes
    - GB-hop and peak link load
  - And how these reductions translate into end-to-end communication time in ASTRA-sim.

---

## 8. License

This project is released under the **MIT License**. See [LICENSE](LICENSE) for details.

### Third-party components

- `external/astra-sim/` &mdash; snapshot of [astra-sim/astra-sim](https://github.com/astra-sim/astra-sim).
  - License: see `external/astra-sim/LICENSE` (Apache 2.0).
  - We only apply local patches; all upstream copyright notices remain intact.
  - When building or redistributing, please comply with the upstream license and cite the ASTRA-sim paper.


---

## 9. Acknowledgements

This project builds on the following open-source components:

- [SuiteSparse Matrix Collection](https://sparse.tamu.edu/)
- [ssgetpy](https://github.com/bkj/ssgetpy)
- [Mt-KaHyPar](https://github.com/kahypar/mt-kahypar)
- [Chakra](https://github.com/astra-sim/chakra)
- [ASTRA-sim](https://github.com/astra-sim/astra-sim)

If you use this repository in academic work, please also consider citing the
original ASTRA-sim and Mt-KaHyPar papers.