#include "doctest.h"

#include "grimm/gpu/factory.h"
#include "grimm/gpu/device.h"
#include "grimm/gpu/swap_chain.h"
#include "grimm/gpu/pipeline_state.h"

DOCTEST_TEST_SUITE("[grimm][gpu] DeviceNull") {
    using namespace gm;
    using namespace gm::gpu;

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

        PipelineStateDesc desc;
        auto pipelineState = device->createPipelineState(desc);
        DOCTEST_CHECK_NE(pipelineState, nullptr);
    }
}
