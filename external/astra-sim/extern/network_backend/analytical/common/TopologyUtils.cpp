#include "common/TopologyUtils.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace {

using namespace NetworkAnalytical;

std::string trim(const std::string& input) {
    const auto begin = input.find_first_not_of(" \t\n\r");
    if (begin == std::string::npos) {
        return "";
    }
    const auto end = input.find_last_not_of(" \t\n\r");
    return input.substr(begin, end - begin + 1);
}


std::string to_lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

std::vector<std::string> split(const std::string& input, char delimiter) {
    auto tokens = std::vector<std::string>();
    auto current = std::string();
    std::istringstream stream(input);
    while (std::getline(stream, current, delimiter)) {
        tokens.push_back(trim(current));
    }
    return tokens;
}

int parse_positive(const std::string& token, const std::string& context) {
    try {
        const auto value = std::stoi(token);
        if (value <= 0) {
            std::cerr << "[Error] (network/analytical) " << context << " must be positive (got " << value << ")" << std::endl;
            std::exit(-1);
        }
        return value;
    } catch (const std::exception& e) {
        std::cerr << "[Error] (network/analytical) failed to parse " << context << ": " << e.what() << std::endl;
        std::exit(-1);
    }
}

GridShape finalize_grid_shape(int npus_count, std::optional<int> rows_hint, std::optional<int> cols_hint, const std::string& context) {
    int rows = rows_hint.value_or(-1);
    int cols = cols_hint.value_or(-1);

    if (rows > 0 && cols > 0) {
        if (rows * cols != npus_count) {
            std::cerr << "[Error] (network/analytical) " << context << " rows*cols (" << rows << "x" << cols << ") must equal npus_count (" << npus_count << ")" << std::endl;
            std::exit(-1);
        }
        return GridShape{rows, cols};
    }

    if (rows > 0) {
        if (npus_count % rows != 0) {
            std::cerr << "[Error] (network/analytical) " << context << " rows (" << rows << ") do not divide npus_count (" << npus_count << ")" << std::endl;
            std::exit(-1);
        }
        cols = npus_count / rows;
        return GridShape{rows, cols};
    }

    if (cols > 0) {
        if (npus_count % cols != 0) {
            std::cerr << "[Error] (network/analytical) " << context << " cols (" << cols << ") do not divide npus_count (" << npus_count << ")" << std::endl;
            std::exit(-1);
        }
        rows = npus_count / cols;
        return GridShape{rows, cols};
    }

    int best_rows = static_cast<int>(std::sqrt(static_cast<double>(npus_count)));
    while (best_rows > 1 && (npus_count % best_rows != 0)) {
        best_rows--;
    }
    if (best_rows == 1) {
        best_rows = 1;
    }
    const auto inferred_cols = npus_count / best_rows;
    return GridShape{best_rows, inferred_cols};
}

GridShape parse_grid_like_shape(const std::string& param, int npus_count, const std::string& context) {
    std::optional<int> rows;
    std::optional<int> cols;

    const auto cleaned = trim(param);
    if (!cleaned.empty()) {
        const auto lower = to_lower(cleaned);
        const auto x_pos = lower.find('x');
        if (x_pos != std::string::npos && lower.find('=') == std::string::npos) {
            const auto lhs = trim(lower.substr(0, x_pos));
            const auto rhs = trim(lower.substr(x_pos + 1));
            if (!lhs.empty() && !rhs.empty()) {
                rows = parse_positive(lhs, context + " rows");
                cols = parse_positive(rhs, context + " cols");
            }
        } else {
            for (const auto& token : split(lower, ',')) {
                if (token.empty()) {
                    continue;
                }
                const auto eq = token.find('=');
                if (eq == std::string::npos) {
                    continue;
                }
                const auto key = trim(token.substr(0, eq));
                const auto value = trim(token.substr(eq + 1));
                if (key == "rows" || key == "row" || key == "r") {
                    rows = parse_positive(value, context + " rows");
                } else if (key == "cols" || key == "columns" || key == "col" || key == "c") {
                    cols = parse_positive(value, context + " cols");
                }
            }
        }
    }

    return finalize_grid_shape(npus_count, rows, cols, context);
}

int64_t pow_int(int base, int exp) {
    int64_t result = 1;
    for (int i = 0; i < exp; i++) {
        result *= base;
    }
    return result;
}

std::optional<ButterflySpec> factorize_butterfly(int npus_count) {
    if (npus_count <= 1) {
        return std::nullopt;
    }

    for (int radix = 2; radix <= npus_count; ++radix) {
        int remaining = npus_count;
        int stages = 0;
        while (remaining % radix == 0) {
            remaining /= radix;
            stages++;
        }

        if (remaining == 1 && stages >= 1) {
            return ButterflySpec{radix, stages};
        }
    }

    return std::nullopt;
}

ButterflySpec finalize_butterfly(int npus_count, std::optional<int> radix_hint, std::optional<int> stages_hint) {
    if (radix_hint && *radix_hint <= 1) {
        std::cerr << "[Error] (network/analytical) butterfly radix must be >= 2 (got " << *radix_hint << ")" << std::endl;
        std::exit(-1);
    }
    if (stages_hint && *stages_hint <= 0) {
        std::cerr << "[Error] (network/analytical) butterfly stages must be positive (got " << *stages_hint << ")" << std::endl;
        std::exit(-1);
    }

    if (radix_hint && stages_hint) {
        const auto total = pow_int(*radix_hint, *stages_hint);
        if (total != npus_count) {
            std::cerr << "[Error] (network/analytical) butterfly radix^stages (" << *radix_hint << "^" << *stages_hint
                      << ") must equal npus_count (" << npus_count << ")" << std::endl;
            std::exit(-1);
        }
        return ButterflySpec{*radix_hint, *stages_hint};
    }

    if (radix_hint) {
        int remaining = npus_count;
        int stages = 0;
        while (remaining % *radix_hint == 0) {
            remaining /= *radix_hint;
            stages++;
            if (remaining == 1) {
                break;
            }
        }
        if (remaining != 1) {
            std::cerr << "[Error] (network/analytical) npus_count (" << npus_count << ") is not a power of radix " << *radix_hint
                      << std::endl;
            std::exit(-1);
        }
        return ButterflySpec{*radix_hint, stages};
    }

    if (stages_hint) {
        const auto approx_radix = static_cast<int>(std::round(std::pow(static_cast<double>(npus_count), 1.0 / *stages_hint)));
        const auto total = pow_int(approx_radix, *stages_hint);
        if (total != npus_count) {
            std::cerr << "[Error] (network/analytical) npus_count (" << npus_count << ") is not " << approx_radix << "^"
                      << *stages_hint << " (rounded guess based on provided stages)" << std::endl;
            std::exit(-1);
        }
        return ButterflySpec{approx_radix, *stages_hint};
    }

    const auto factored = factorize_butterfly(npus_count);
    if (factored.has_value()) {
        return factored.value();
    }

    return ButterflySpec{npus_count, 1};
}

std::optional<std::pair<std::string, std::string>> parse_key_value(const std::string& token) {
    const auto eq = token.find('=');
    if (eq == std::string::npos) {
        return std::nullopt;
    }
    auto key = trim(token.substr(0, eq));
    auto value = trim(token.substr(eq + 1));
    if (key.empty() || value.empty()) {
        return std::nullopt;
    }
    return std::make_pair(to_lower(key), value);
}

}  // namespace

namespace NetworkAnalytical {

GridShape parse_mesh2d_shape(const std::string& param, const int npus_count) noexcept {
    assert(npus_count > 1);
    return parse_grid_like_shape(param, npus_count, "Mesh2D");
}

GridShape parse_torus2d_shape(const std::string& param, const int npus_count) noexcept {
    assert(npus_count > 1);
    return parse_grid_like_shape(param, npus_count, "Torus2D");
}

ButterflySpec parse_butterfly_spec(const std::string& param, const int npus_count) noexcept {
    assert(npus_count > 1);

    std::optional<int> radix_hint;
    std::optional<int> stages_hint;

    const auto cleaned = trim(param);
    if (!cleaned.empty()) {
        for (const auto& token : split(cleaned, ',')) {
            if (token.empty()) {
                continue;
            }
            const auto kv = parse_key_value(token);
            if (!kv.has_value()) {
                continue;
            }
            const auto& [key, value] = kv.value();
            if (key == "radix" || key == "fanout" || key == "r") {
                radix_hint = parse_positive(value, "Butterfly radix");
            } else if (key == "stages" || key == "stage" || key == "levels" || key == "s") {
                stages_hint = parse_positive(value, "Butterfly stages");
            }
        }
    }

    return finalize_butterfly(npus_count, radix_hint, stages_hint);
}

}  // namespace NetworkAnalytical
