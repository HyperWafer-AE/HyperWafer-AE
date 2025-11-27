/******************************************************************************
This source code is licensed under the MIT license found in the
LICENSE file in the root directory of this source tree.
*******************************************************************************/

#include "common/NetworkParser.h"
#include <cassert>
#include <iostream>
#include <string>

namespace {

std::string trim(const std::string& input) {
    const auto begin = input.find_first_not_of(" \t\n\r");
    if (begin == std::string::npos) {
        return "";
    }
    const auto end = input.find_last_not_of(" \t\n\r");
    return input.substr(begin, end - begin + 1);
}

struct ParsedTopologyToken {
    std::string name;
    std::string param;
};

ParsedTopologyToken parse_topology_token(const std::string& token) {
    const auto cleaned = trim(token);
    if (cleaned.empty()) {
        std::cerr << "[Error] (network/analytical) topology entry is empty" << std::endl;
        std::exit(-1);
    }

    const auto open = cleaned.find_first_of("([{");
    if (open == std::string::npos) {
        return ParsedTopologyToken{cleaned, ""};
    }

    const auto close = cleaned.find_last_of(")]}");
    if (close == std::string::npos || close <= open) {
        std::cerr << "[Error] (network/analytical) malformed topology token: " << cleaned << std::endl;
        std::exit(-1);
    }

    auto name = trim(cleaned.substr(0, open));
    auto param = trim(cleaned.substr(open + 1, close - open - 1));
    if (name.empty()) {
        std::cerr << "[Error] (network/analytical) topology name missing in token: " << cleaned << std::endl;
        std::exit(-1);
    }

    return ParsedTopologyToken{name, param};
}

}  // namespace

using namespace NetworkAnalytical;

NetworkParser::NetworkParser(const std::string& path) noexcept : dims_count(-1) {
    // initialize values
    npus_count_per_dim = {};
    bandwidth_per_dim = {};
    latency_per_dim = {};
    topology_per_dim = {};
    topology_params_per_dim = {};

    try {
        // load network config file
        const auto network_config = YAML::LoadFile(path);

        // parse network configs
        parse_network_config_yml(network_config);
    } catch (const YAML::BadFile& e) {
        // loading network config file failed
        std::cerr << "[Error] (network/analytical) " << e.what() << std::endl;
        std::exit(-1);
    }
}

int NetworkParser::get_dims_count() const noexcept {
    assert(dims_count > 0);

    return dims_count;
}

std::vector<int> NetworkParser::get_npus_counts_per_dim() const noexcept {
    assert(dims_count > 0);
    assert(npus_count_per_dim.size() == dims_count);

    return npus_count_per_dim;
}

std::vector<Bandwidth> NetworkParser::get_bandwidths_per_dim() const noexcept {
    assert(dims_count > 0);
    assert(bandwidth_per_dim.size() == dims_count);

    return bandwidth_per_dim;
}

std::vector<Latency> NetworkParser::get_latencies_per_dim() const noexcept {
    assert(dims_count > 0);
    assert(latency_per_dim.size() == dims_count);

    return latency_per_dim;
}

std::vector<TopologyBuildingBlock> NetworkParser::get_topologies_per_dim() const noexcept {
    assert(dims_count > 0);
    assert(topology_per_dim.size() == dims_count);

    return topology_per_dim;
}

std::vector<std::string> NetworkParser::get_topology_params_per_dim() const noexcept {
    assert(dims_count > 0);
    assert(topology_params_per_dim.size() == dims_count);

    return topology_params_per_dim;
}

void NetworkParser::parse_network_config_yml(const YAML::Node& network_config) noexcept {
    // parse topology_per_dim
    const auto topology_names = parse_vector<std::string>(network_config["topology"]);
    for (const auto& topology_token : topology_names) {
        const auto parsed = parse_topology_token(topology_token);
        const auto topology_dim = NetworkParser::parse_topology_name(parsed.name);
        topology_per_dim.push_back(topology_dim);
        topology_params_per_dim.push_back(parsed.param);
    }

    // set dims_count
    dims_count = static_cast<int>(topology_per_dim.size());

    // parse vector values
    npus_count_per_dim = parse_vector<int>(network_config["npus_count"]);
    bandwidth_per_dim = parse_vector<Bandwidth>(network_config["bandwidth"]);
    latency_per_dim = parse_vector<Latency>(network_config["latency"]);

    // check the validity of the parsed network config
    check_validity();
}

TopologyBuildingBlock NetworkParser::parse_topology_name(const std::string& topology_name) noexcept {
    assert(!topology_name.empty());

    if (topology_name == "Ring") {
        return TopologyBuildingBlock::Ring;
    }

    if (topology_name == "FullyConnected") {
        return TopologyBuildingBlock::FullyConnected;
    }

    if (topology_name == "Switch") {
        return TopologyBuildingBlock::Switch;
    }

    if (topology_name == "Mesh2D" || topology_name == "Mesh") {
        return TopologyBuildingBlock::Mesh2D;
    }

    if (topology_name == "Torus2D" || topology_name == "Torus") {
        return TopologyBuildingBlock::Torus2D;
    }

    if (topology_name == "Butterfly") {
        return TopologyBuildingBlock::Butterfly;
    }

    std::cerr << "[Error] (network/analytical) Topology name " << topology_name
              << " not supported (expected Ring/FullyConnected/Switch/Mesh2D/Torus2D/Butterfly)" << std::endl;
    std::exit(-1);
}

void NetworkParser::check_validity() const noexcept {
    // dims_count should match
    if (dims_count != npus_count_per_dim.size()) {
        std::cerr << "[Error] (network/analytical) " << "length of npus_count (" << npus_count_per_dim.size()
                  << ") doesn't match with dimensions (" << dims_count << ")" << std::endl;
        std::exit(-1);
    }

    if (dims_count != topology_params_per_dim.size()) {
        std::cerr << "[Error] (network/analytical) " << "internal topology metadata mismatch" << std::endl;
        std::exit(-1);
    }

    if (dims_count != bandwidth_per_dim.size()) {
        std::cerr << "[Error] (network/analytical) " << "length of bandwidth (" << bandwidth_per_dim.size()
                  << ") doesn't match with dims_count (" << dims_count << ")" << std::endl;
        std::exit(-1);
    }

    if (dims_count != latency_per_dim.size()) {
        std::cerr << "[Error] (network/analytical) " << "length of latency (" << latency_per_dim.size()
                  << ") doesn't match with dims_count (" << dims_count << ")" << std::endl;
        std::exit(-1);
    }

    // npus_count should be all positive
    for (const auto& npus_count : npus_count_per_dim) {
        if (npus_count <= 1) {
            std::cerr << "[Error] (network/analytical) " << "npus_count (" << npus_count << ") should be larger than 1"
                      << std::endl;
            std::exit(-1);
        }
    }

    // bandwidths should be all positive
    for (const auto& bandwidth : bandwidth_per_dim) {
        if (bandwidth <= 0) {
            std::cerr << "[Error] (network/analytical) " << "bandwidth (" << bandwidth << ") should be larger than 0"
                      << std::endl;
            std::exit(-1);
        }
    }

    // latency should be non-negative
    for (const auto& latency : latency_per_dim) {
        if (latency < 0) {
            std::cerr << "[Error] (network/analytical) " << "latency (" << latency << ") should be non-negative"
                      << std::endl;
            std::exit(-1);
        }
    }
}
