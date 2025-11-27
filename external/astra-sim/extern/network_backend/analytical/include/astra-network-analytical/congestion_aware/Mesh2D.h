#pragma once

#include "congestion_aware/BasicTopology.h"
#include <utility>

using namespace NetworkAnalytical;

namespace NetworkAnalyticalCongestionAware {

class Mesh2D final : public BasicTopology {
  public:
    Mesh2D(int npus_count, Bandwidth bandwidth, Latency latency, int rows, int cols) noexcept;

    [[nodiscard]] Route route(DeviceId src, DeviceId dest) const noexcept override;

  private:
    void connect_neighbors(Bandwidth bandwidth, Latency latency) noexcept;
    [[nodiscard]] int encode(int row, int col) const noexcept;
    [[nodiscard]] std::pair<int, int> decode(DeviceId id) const noexcept;

    int rows;
    int cols;
};

}  // namespace NetworkAnalyticalCongestionAware

