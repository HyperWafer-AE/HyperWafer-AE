#pragma once

#include "congestion_aware/BasicTopology.h"
#include <vector>

using namespace NetworkAnalytical;

namespace NetworkAnalyticalCongestionAware {

class Butterfly final : public BasicTopology {
  public:
    Butterfly(int npus_count, Bandwidth bandwidth, Latency latency, int radix, int stages) noexcept;

    [[nodiscard]] Route route(DeviceId src, DeviceId dest) const noexcept override;

  private:
    [[nodiscard]] int router_id(int stage_index, int row) const noexcept;
    [[nodiscard]] std::vector<int> enumerate_neighbors(int row, int stage_index) const noexcept;
    [[nodiscard]] std::vector<int> encode_digits(int value) const noexcept;
    [[nodiscard]] int decode_digits(const std::vector<int>& digits) const noexcept;

    int radix;
    int stages;
};

}  // namespace NetworkAnalyticalCongestionAware

