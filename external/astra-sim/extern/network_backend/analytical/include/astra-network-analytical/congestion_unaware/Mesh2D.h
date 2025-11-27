#pragma once

#include "common/Type.h"
#include "congestion_unaware/BasicTopology.h"
#include <utility>

using namespace NetworkAnalytical;

namespace NetworkAnalyticalCongestionUnaware {

class Mesh2D final : public BasicTopology {
  public:
    Mesh2D(int npus_count, Bandwidth bandwidth, Latency latency, int rows, int cols) noexcept;

  private:
    [[nodiscard]] int compute_hops_count(DeviceId src, DeviceId dest) const noexcept override;
    [[nodiscard]] std::pair<int, int> decode(DeviceId id) const noexcept;

    int rows;
    int cols;
};

}  // namespace NetworkAnalyticalCongestionUnaware
