#pragma once

#include "common/Type.h"
#include <string>

namespace NetworkAnalytical {

struct GridShape {
    int rows;
    int cols;
};

struct ButterflySpec {
    int radix;
    int stages;
};

[[nodiscard]] GridShape parse_mesh2d_shape(const std::string& param, int npus_count) noexcept;

[[nodiscard]] GridShape parse_torus2d_shape(const std::string& param, int npus_count) noexcept;

[[nodiscard]] ButterflySpec parse_butterfly_spec(const std::string& param, int npus_count) noexcept;

}  // namespace NetworkAnalytical
