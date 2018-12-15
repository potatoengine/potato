#include "doctest.h"

#include "null_factory.h"
#include "null_device.h"

DOCTEST_TEST_SUITE("[grimm][gpu] NullDevice")
{
    DOCTEST_TEST_CASE("factory enumerates")
    {
        using namespace gm;

        NullFactory factory;

        DOCTEST_CHECK(factory.isEnabled());

        bool enumerated = false;
        factory.enumerateDevices([&](auto const& deviceInfo){
            DOCTEST_CHECK_EQ(deviceInfo.index, 0);

            // ensure we only get one
            DOCTEST_CHECK_FALSE(enumerated);
            enumerated = true;
        });
    }

    DOCTEST_TEST_CASE("factory abides")
    {
        using namespace gm;

        NullFactory factory;

        auto device = factory.createDevice(0);
        DOCTEST_CHECK_NE(device, nullptr);
    }
}
