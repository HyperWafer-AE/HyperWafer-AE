#include "congestion_unaware/Butterfly.h"
#include <cassert>
#include <cstdint>

using namespace NetworkAnalytical;
using namespace NetworkAnalyticalCongestionUnaware;

namespace {

int64_t pow_int(const int base, const int exp) {
    int64_t result = 1;
    for (int i = 0; i < exp; ++i) {
        result *= base;
    }
    return result;
}

}  // namespace

Butterfly::Butterfly(const int npus_count, const Bandwidth bandwidth, const Latency latency, const int radix, const int stages) noexcept
    : radix(radix),
      stages(stages),
      BasicTopology(npus_count, bandwidth, latency) {
    assert(radix >= 2);
    assert(stages >= 1);
    assert(npus_count > 1);

    const auto total_nodes = pow_int(radix, stages);
    assert(total_nodes == npus_count);

    basic_topology_type = TopologyBuildingBlock::Butterfly;

    nodes_per_stage.clear();
    nodes_per_stage.reserve(stages);
    auto width = 1;
    for (int i = 0; i < stages; ++i) {
        width *= radix;
        nodes_per_stage.push_back(width);
    }
}

int Butterfly::compute_hops_count(const DeviceId src, const DeviceId dest) const noexcept {
    assert(0 <= src && src < npus_count);
    assert(0 <= dest && dest < npus_count);
    assert(src != dest);

    return stages;
}
