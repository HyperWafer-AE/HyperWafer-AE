#include "congestion_aware/Torus2D.h"
#include <algorithm>
#include <cassert>
#include <cstdlib>

using namespace NetworkAnalyticalCongestionAware;

Torus2D::Torus2D(const int npus_count, const Bandwidth bandwidth, const Latency latency, const int rows, const int cols) noexcept
    : rows(rows),
      cols(cols),
      BasicTopology(npus_count, npus_count, bandwidth, latency) {
    assert(rows > 0);
    assert(cols > 0);
    assert(rows * cols == npus_count);

    basic_topology_type = TopologyBuildingBlock::Torus2D;

    connect_neighbors(bandwidth, latency);

    npus_count_per_dim.clear();
    npus_count_per_dim.push_back(rows);
    npus_count_per_dim.push_back(cols);
    bandwidth_per_dim.clear();
    bandwidth_per_dim.push_back(bandwidth);
    bandwidth_per_dim.push_back(bandwidth);
    dims_count = 2;
}

void Torus2D::connect_neighbors(const Bandwidth bandwidth, const Latency latency) noexcept {
    for (auto row = 0; row < rows; row++) {
        for (auto col = 0; col < cols; col++) {
            const auto id = encode(row, col);
            const auto right = encode(row, (col + 1) % cols);
            const auto down = encode((row + 1) % rows, col);
            connect(id, right, bandwidth, latency, true);
            connect(id, down, bandwidth, latency, true);
        }
    }
}

Route Torus2D::route(const DeviceId src, const DeviceId dest) const noexcept {
    assert(0 <= src && src < npus_count);
    assert(0 <= dest && dest < npus_count);

    const auto [src_row, src_col] = decode(src);
    const auto [dest_row, dest_col] = decode(dest);

    auto current_row = src_row;
    auto current_col = src_col;
    auto route = Route();
    route.push_back(devices[src]);

    // vertical traversal
    int row_diff = dest_row - current_row;
    int steps_down = (row_diff >= 0) ? row_diff : row_diff + rows;
    int steps_up = rows - steps_down;
    int row_direction = (steps_down <= steps_up) ? 1 : -1;
    int row_steps = std::min(steps_down, steps_up);
    for (auto step = 0; step < row_steps; step++) {
        current_row = wrap(current_row, row_direction, rows);
        const auto next = encode(current_row, current_col);
        route.push_back(devices[next]);
    }

    // horizontal traversal
    int col_diff = dest_col - current_col;
    int steps_right = (col_diff >= 0) ? col_diff : col_diff + cols;
    int steps_left = cols - steps_right;
    int col_direction = (steps_right <= steps_left) ? 1 : -1;
    int col_steps = std::min(steps_right, steps_left);
    for (auto step = 0; step < col_steps; step++) {
        current_col = wrap(current_col, col_direction, cols);
        const auto next = encode(current_row, current_col);
        route.push_back(devices[next]);
    }

    return route;
}

int Torus2D::wrap(const int coordinate, const int delta, const int bound) const noexcept {
    auto next = coordinate + delta;
    if (next < 0) {
        next += bound;
    } else if (next >= bound) {
        next -= bound;
    }
    return next;
}

int Torus2D::encode(const int row, const int col) const noexcept {
    return row * cols + col;
}

std::pair<int, int> Torus2D::decode(const DeviceId id) const noexcept {
    assert(0 <= id && id < npus_count);
    const auto row = id / cols;
    const auto col = id % cols;
    return {row, col};
}

