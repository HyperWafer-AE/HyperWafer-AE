#include "congestion_aware/Butterfly.h"
#include <cassert>
#include <cmath>

using namespace NetworkAnalyticalCongestionAware;

namespace {

int64_t pow_int(const int base, const int exp) {
    int64_t value = 1;
    for (int i = 0; i < exp; ++i) {
        value *= base;
    }
    return value;
}

}  // namespace

Butterfly::Butterfly(const int npus_count, const Bandwidth bandwidth, const Latency latency, const int radix, const int stages) noexcept
    : radix(radix),
      stages(stages),
      BasicTopology(npus_count, npus_count + std::max(0, stages - 1) * npus_count, bandwidth, latency) {
    assert(radix >= 2);
    assert(stages >= 1);
    assert(pow_int(radix, stages) == npus_count);

    basic_topology_type = TopologyBuildingBlock::Butterfly;

    if (stages == 1) {
        for (auto src = 0; src < npus_count; ++src) {
            for (auto dest = 0; dest < npus_count; ++dest) {
                if (src != dest) {
                    connect(src, dest, bandwidth, latency, true);
                }
            }
        }
        return;
    }

    for (auto stage = 0; stage < stages; ++stage) {
        const auto from_is_npu = (stage == 0);
        const auto to_is_router = (stage + 1 < stages);
        for (auto row = 0; row < npus_count; ++row) {
            const auto from_id = from_is_npu ? row : router_id(stage, row);
            const auto neighbors = enumerate_neighbors(row, stage);
            for (const auto neighbor_row : neighbors) {
                const auto to_id = to_is_router ? router_id(stage + 1, neighbor_row) : neighbor_row;
                connect(from_id, to_id, bandwidth, latency, true);
            }
        }
    }
}

Route Butterfly::route(const DeviceId src, const DeviceId dest) const noexcept {
    assert(0 <= src && src < npus_count);
    assert(0 <= dest && dest < npus_count);

    auto route = Route();
    route.push_back(devices[src]);

    if (src == dest) {
        return route;
    }

    const auto dest_digits = encode_digits(dest);
    auto current_digits = encode_digits(src);

    for (auto stage = 0; stage < stages - 1; ++stage) {
        current_digits[stage] = dest_digits[stage];
        const auto next_row = decode_digits(current_digits);
        const auto next_device = router_id(stage + 1, next_row);
        route.push_back(devices[next_device]);
    }

    route.push_back(devices[dest]);
    return route;
}

int Butterfly::router_id(const int stage_index, const int row) const noexcept {
    // stage_index ranges [1, stages - 1]
    return npus_count + (stage_index - 1) * npus_count + row;
}

std::vector<int> Butterfly::enumerate_neighbors(const int row, const int stage_index) const noexcept {
    auto digits = encode_digits(row);
    auto neighbors = std::vector<int>();
    neighbors.reserve(radix);

    for (auto digit = 0; digit < radix; ++digit) {
        auto candidate = digits;
        candidate[stage_index] = digit;
        neighbors.push_back(decode_digits(candidate));
    }

    return neighbors;
}

std::vector<int> Butterfly::encode_digits(int value) const noexcept {
    auto digits = std::vector<int>(stages, 0);
    for (int i = stages - 1; i >= 0; --i) {
        digits[i] = value % radix;
        value /= radix;
    }
    return digits;
}

int Butterfly::decode_digits(const std::vector<int>& digits) const noexcept {
    auto value = 0;
    for (auto digit : digits) {
        value = value * radix + digit;
    }
    return value;
}

