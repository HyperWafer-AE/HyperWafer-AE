#include "congestion_unaware/Torus2D.h"
#include <algorithm>
#include <cassert>
#include <cstdlib>

using namespace NetworkAnalytical;
using namespace NetworkAnalyticalCongestionUnaware;

Torus2D::Torus2D(const int npus_count, const Bandwidth bandwidth, const Latency latency, const int rows, const int cols) noexcept
    : rows(rows),
      cols(cols),
      BasicTopology(npus_count, bandwidth, latency) {
    assert(rows > 0);
    assert(cols > 0);
    assert(npus_count == rows * cols);

    basic_topology_type = TopologyBuildingBlock::Torus2D;

    npus_count_per_dim.clear();
    npus_count_per_dim.push_back(rows);
    npus_count_per_dim.push_back(cols);
    bandwidth_per_dim.clear();
    bandwidth_per_dim.push_back(bandwidth);
    bandwidth_per_dim.push_back(bandwidth);
    dims_count = 2;
}

int Torus2D::compute_hops_count(const DeviceId src, const DeviceId dest) const noexcept {
    assert(0 <= src && src < npus_count);
    assert(0 <= dest && dest < npus_count);
    assert(src != dest);

    const auto [src_row, src_col] = decode(src);
    const auto [dest_row, dest_col] = decode(dest);

    const auto vertical_diff = std::abs(src_row - dest_row);
    const auto vertical = std::min(vertical_diff, rows - vertical_diff);

    const auto horizontal_diff = std::abs(src_col - dest_col);
    const auto horizontal = std::min(horizontal_diff, cols - horizontal_diff);

    return vertical + horizontal;
}

std::pair<int, int> Torus2D::decode(const DeviceId id) const noexcept {
    assert(0 <= id && id < npus_count);
    const auto row = id / cols;
    const auto col = id % cols;
    return std::make_pair(row, col);
}
