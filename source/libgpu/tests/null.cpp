#include <doctest/doctest.h>

#include "potato/gpu/gpu_factory.h"
#include "potato/gpu/gpu_device.h"
#include "potato/gpu/gpu_swap_chain.h"
#include "potato/gpu/gpu_pipeline_state.h"

DOCTEST_TEST_SUITE("[potato][gpu] DeviceNull") {
    using namespace up;
    using namespace up;

    DOCTEST_TEST_CASE("factory enumerates") {
        auto factory = CreateFactoryNull();

        DOCTEST_CHECK(factory->isEnabled());

        bool enumerated = false;
        factory->enumerateDevices([&](auto const& deviceInfo) {
            DOCTEST_CHECK_EQ(deviceInfo.index, 0);

            // ensure we only get one
            DOCTEST_CHECK_FALSE(enumerated);
            enumerated = true;
        });
    }

    DOCTEST_TEST_CASE("factory abides") {
        auto factory = CreateFactoryNull();

        auto device = factory->createDevice(0);
        DOCTEST_CHECK_NE(device, nullptr);
    }

    DOCTEST_TEST_CASE("device abides") {
        auto factory = CreateFactoryNull();
        auto device = factory->createDevice(0);

        auto swapChain = device->createSwapChain(nullptr);
        DOCTEST_CHECK_NE(swapChain, nullptr);

        GpuPipelineStateDesc desc;
        auto pipelineState = device->createPipelineState(desc);
        DOCTEST_CHECK_NE(pipelineState, nullptr);
    }
}
