# astra-network-analytical

## Overview
Analytical network simulator models communications over multi-dimensional topologies through analytical equations. Currently, two variations of analytical network simulation are supported.
- `congestion_unaware` analytical network simulator
- `congestion_aware` analytical network simulator

This simulator is developed as a part of the [ASTRA-sim](https://github.com/astra-sim/astra-sim) project, thereby the analytical network simulator can naturally be used as the network modeling backend of the ASTRA-sim simulator.

## Build Status
| main branch | macOS | Ubuntu |
|:---:|:---:|:---:|
| congestion_unaware | [![build](https://github.com/astra-sim/astra-network-analytical/actions/workflows/test_congestion_unaware_macos.yml/badge.svg?branch=main)](https://github.com/astra-sim/astra-network-analytical/actions/workflows/test_congestion_unaware_macos.yml) | [![build](https://github.com/astra-sim/astra-network-analytical/actions/workflows/test_congestion_unaware_ubuntu.yml/badge.svg?branch=main)](https://github.com/astra-sim/astra-network-analytical/actions/workflows/test_congestion_unaware_ubuntu.yml) |
| congestion_aware | [![build](https://github.com/astra-sim/astra-network-analytical/actions/workflows/test_congestion_aware_macos.yml/badge.svg?branch=main)](https://github.com/astra-sim/astra-network-analytical/actions/workflows/test_congestion_aware_macos.yml) | [![build](https://github.com/astra-sim/astra-network-analytical/actions/workflows/test_congestion_aware_ubuntu.yml/badge.svg?branch=main)](https://github.com/astra-sim/astra-network-analytical/actions/workflows/test_congestion_aware_ubuntu.yml) |

## Formatting
| main branch | format |
|:---:|:---:|
| format | [![format](https://github.com/astra-sim/astra-network-analytical/actions/workflows/check-clang-format.yml/badge.svg?branch=main)](https://github.com/astra-sim/astra-network-analytical/actions/workflows/check-clang-format.yml) |

## Supported Topologies

The congestion-unaware analytical backend now accepts the following `topology`
tokens inside network YAML files:

| Token | Description | Optional parameters |
| --- | --- | --- |
| `Ring` | 1-D bidirectional ring | _none_ |
| `Switch` | Fully connected crossbar | _none_ |
| `FullyConnected` | Complete graph | _none_ |
| `Mesh2D` | Single-stage 2-D mesh | `Mesh2D(ROWSxCOLS)` or `Mesh2D(rows=R, cols=C)`; defaults to a near-square factorization |
| `Torus2D` | Single-stage 2-D torus with wrap-around links | Same syntax as `Mesh2D` |
| `Butterfly` | Radix-`r`, `m`-stage butterfly | `Butterfly(radix=R, stages=M)` (either parameter can be omitted; defaults to radix-2 with inferred stages) |

Multi-dimensional inputs still stack 1-D building blocks (Ring/Switch/FullyConnected). Use the dedicated `Mesh2D`, `Torus2D`, or `Butterfly`
tokens directly in single-dimension runs to model those fabrics analytically. Both the congestion-unaware and congestion-aware analytical
backends understand these tokens, so you can pick whichever simulator matches your study (just remember to point `--astrasim-bin` at the
appropriate executable).

See the new sample configs under `extern/network_backend/analytical/input/` and
`inputs/network/` for concrete examples.

## Documentation
- [Analytical Network Simulator Documentation](https://astra-sim.github.io/astra-network-analytical-docs/index.html)
- [ASTRA-sim Documentation](https://astra-sim.github.io/astra-sim-docs/index.html)

## References
- [ASTRA-sim website](https://astra-sim.github.io)
- [ASTRA-sim2.0 paper](https://arxiv.org/abs/2303.14006)
