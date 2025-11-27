#pragma once

#include "congestion_aware/BasicTopology.h"
#include <utility>

using namespace NetworkAnalytical;

namespace NetworkAnalyticalCongestionAware {

class Torus2D final : public BasicTopology {
  public:
    Torus2D(int npus_count, Bandwidth bandwidth, Latency latency, int rows, int cols) noexcept;

    [[nodiscard]] Route route(DeviceId src, DeviceId dest) const noexcept override;

  private:
    void connect_neighbors(Bandwidth bandwidth, Latency latency) noexcept;
    [[nodiscard]] int encode(int row, int col) const noexcept;
    [[nodiscard]] std::pair<int, int> decode(DeviceId id) const noexcept;
    [[nodiscard]] int wrap(int coordinate, int delta, int bound) const noexcept;

    int rows;
    int cols;
};

}  // namespace NetworkAnalyticalCongestionAware

