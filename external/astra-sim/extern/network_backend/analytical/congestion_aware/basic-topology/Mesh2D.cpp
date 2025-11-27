#include "congestion_aware/Mesh2D.h"
#include <cassert>
#include <cstdlib>

using namespace NetworkAnalyticalCongestionAware;

Mesh2D::Mesh2D(const int npus_count, const Bandwidth bandwidth, const Latency latency, const int rows, const int cols) noexcept
    : rows(rows),
      cols(cols),
      BasicTopology(npus_count, npus_count, bandwidth, latency) {
    assert(rows > 0);
    assert(cols > 0);
    assert(rows * cols == npus_count);

    basic_topology_type = TopologyBuildingBlock::Mesh2D;

    connect_neighbors(bandwidth, latency);

    npus_count_per_dim.clear();
    npus_count_per_dim.push_back(rows);
    npus_count_per_dim.push_back(cols);
    bandwidth_per_dim.clear();
    bandwidth_per_dim.push_back(bandwidth);
    bandwidth_per_dim.push_back(bandwidth);
    dims_count = 2;
}

void Mesh2D::connect_neighbors(const Bandwidth bandwidth, const Latency latency) noexcept {
    for (auto row = 0; row < rows; row++) {
        for (auto col = 0; col < cols; col++) {
            const auto id = encode(row, col);
            if (col + 1 < cols) {
                const auto right = encode(row, col + 1);
                connect(id, right, bandwidth, latency, true);
            }
            if (row + 1 < rows) {
                const auto down = encode(row + 1, col);
                connect(id, down, bandwidth, latency, true);
            }
        }
    }
}

Route Mesh2D::route(const DeviceId src, const DeviceId dest) const noexcept {
    assert(0 <= src && src < npus_count);
    assert(0 <= dest && dest < npus_count);

    const auto [src_row, src_col] = decode(src);
    const auto [dest_row, dest_col] = decode(dest);

    auto current_row = src_row;
    auto current_col = src_col;
    auto route = Route();
    route.push_back(devices[src]);

    const auto row_step = (dest_row > current_row) ? 1 : -1;
    while (current_row != dest_row) {
        current_row += row_step;
        const auto next = encode(current_row, current_col);
        route.push_back(devices[next]);
    }

    const auto col_step = (dest_col > current_col) ? 1 : -1;
    while (current_col != dest_col) {
        current_col += col_step;
        const auto next = encode(current_row, current_col);
        route.push_back(devices[next]);
    }

    return route;
}

int Mesh2D::encode(const int row, const int col) const noexcept {
    return row * cols + col;
}

std::pair<int, int> Mesh2D::decode(const DeviceId id) const noexcept {
    assert(0 <= id && id < npus_count);
    const auto row = id / cols;
    const auto col = id % cols;
    return {row, col};
}

