/******************************************************************************
This source code is licensed under the MIT license found in the
LICENSE file in the root directory of this source tree.
*******************************************************************************/

#include "common/NetworkParser.h"
#include "common/TopologyUtils.h"
#include "common/Type.h"
#include "congestion_unaware/Helper.h"
#include <gtest/gtest.h>

using namespace NetworkAnalytical;
using namespace NetworkAnalyticalCongestionUnaware;

namespace {

EventTime expected_delay(int hops, Latency latency, Bandwidth bandwidth_GBps, ChunkSize chunk_size) {
    const auto bandwidth_Bpns = bandwidth_GBps * (1ull << 30) / 1'000'000'000;
    const auto serialization = static_cast<double>(chunk_size) / bandwidth_Bpns;
    const auto total = latency * hops + serialization;
    return static_cast<EventTime>(total);
}

}  // namespace

class TestNetworkAnalyticalCongestionUnaware : public ::testing::Test {
  protected:
    void SetUp() override {
        // set chunk size
        chunk_size = 1'048'576;  // 1 MB
    }

    ChunkSize chunk_size;
};

TEST_F(TestNetworkAnalyticalCongestionUnaware, Ring) {
    // create network
    const auto network_parser = NetworkParser("../../input/Ring.yml");
    const auto topology = construct_topology(network_parser);

    // run communication
    const auto comm_delay = topology->send(1, 4, chunk_size);
    EXPECT_EQ(comm_delay, 21'031);
}

TEST_F(TestNetworkAnalyticalCongestionUnaware, FullyConnected) {
    // create network
    const auto network_parser = NetworkParser("../../input/FullyConnected.yml");
    const auto topology = construct_topology(network_parser);

    // run communication
    const auto comm_delay = topology->send(1, 4, chunk_size);
    EXPECT_EQ(comm_delay, 20'031);
}

TEST_F(TestNetworkAnalyticalCongestionUnaware, Switch) {
    // create network
    const auto network_parser = NetworkParser("../../input/Switch.yml");
    const auto topology = construct_topology(network_parser);

    // run communication
    const auto comm_delay = topology->send(1, 4, chunk_size);
    EXPECT_EQ(comm_delay, 20'531);
}

TEST_F(TestNetworkAnalyticalCongestionUnaware, Ring_FullyConnected_Switch) {
    // create network
    const auto network_parser = NetworkParser("../../input/Ring_FullyConnected_Switch.yml");
    const auto topology = construct_topology(network_parser);

    // run on dim 1
    const auto comm_delay_dim1 = topology->send(0, 1, chunk_size);
    EXPECT_EQ(comm_delay_dim1, 4'932);

    // run on dim 2
    const auto comm_delay_dim2 = topology->send(37, 41, chunk_size);
    EXPECT_EQ(comm_delay_dim2, 10'265);

    // run on dim 3
    const auto comm_delay_dim3 = topology->send(26, 42, chunk_size);
    EXPECT_EQ(comm_delay_dim3, 23'531);
}

TEST_F(TestNetworkAnalyticalCongestionUnaware, Mesh2D) {
    const auto network_parser = NetworkParser("../../input/Mesh2D.yml");
    const auto topology = construct_topology(network_parser);

    const auto comm_delay = topology->send(0, 15, chunk_size);
    const auto expected = expected_delay(6, 500.0, 60.0, chunk_size);
    EXPECT_EQ(comm_delay, expected);
}

TEST_F(TestNetworkAnalyticalCongestionUnaware, Torus2D) {
    const auto network_parser = NetworkParser("../../input/Torus2D.yml");
    const auto topology = construct_topology(network_parser);

    const auto comm_delay = topology->send(0, 3, chunk_size);
    const auto expected = expected_delay(1, 400.0, 60.0, chunk_size);
    EXPECT_EQ(comm_delay, expected);
}

TEST_F(TestNetworkAnalyticalCongestionUnaware, Butterfly) {
    const auto network_parser = NetworkParser("../../input/Butterfly.yml");
    const auto topology = construct_topology(network_parser);

    const auto comm_delay = topology->send(0, 17, chunk_size);
    const auto expected = expected_delay(3, 350.0, 70.0, chunk_size);
    EXPECT_EQ(comm_delay, expected);
}

TEST(TopologyParamParserTest, MeshDefaults) {
    const auto shape = parse_mesh2d_shape("", 36);
    EXPECT_EQ(shape.rows * shape.cols, 36);
    EXPECT_LE(shape.rows, shape.cols);
}

TEST(TopologyParamParserTest, ButterflyRadixOnly) {
    const auto spec = parse_butterfly_spec("radix=4", 64);
    EXPECT_EQ(spec.radix, 4);
    EXPECT_EQ(spec.stages, 3);
}
