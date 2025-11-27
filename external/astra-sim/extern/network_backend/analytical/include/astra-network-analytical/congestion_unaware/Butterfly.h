#pragma once

#include "common/Type.h"
#include "congestion_unaware/BasicTopology.h"
#include <vector>

using namespace NetworkAnalytical;

namespace NetworkAnalyticalCongestionUnaware {

class Butterfly final : public BasicTopology {
  public:
    Butterfly(int npus_count, Bandwidth bandwidth, Latency latency, int radix, int stages) noexcept;

  private:
    [[nodiscard]] int compute_hops_count(DeviceId src, DeviceId dest) const noexcept override;

    int radix;
    int stages;
    std::vector<int> nodes_per_stage;
};

}  // namespace NetworkAnalyticalCongestionUnaware
