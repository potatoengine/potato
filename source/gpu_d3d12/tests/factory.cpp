#include "doctest.h"

#include "d3d12_factory.h"
#include "d3d12_device.h"

DOCTEST_TEST_SUITE("[grimm][gpu_d3d12] D3d12Factory")
{
    DOCTEST_TEST_CASE("factory constructs")
    {
        auto factory = gm::CreateD3d12Factory();

        DOCTEST_CHECK_NE(factory, nullptr);
        DOCTEST_CHECK(factory->isEnabled());
    }

    DOCTEST_TEST_CASE("device exists")
    {
        auto factory = gm::CreateD3d12Factory();

        int count = 0;
        factory->enumerateDevices([&](auto const& info){ ++count; });

        DOCTEST_CHECK_GE(count, 1);

        auto device = factory->createDevice(0);

        DOCTEST_CHECK_NE(device, nullptr);
    }
}
