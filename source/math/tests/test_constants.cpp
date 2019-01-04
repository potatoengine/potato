#include "doctest.h"
#include "grimm/math/constants.h"
#include "expect_float.h"

DOCTEST_TEST_SUITE("[grimm][math] constants") {
    using namespace gm;
    using namespace gm::constants;

    DOCTEST_TEST_CASE("pi") {
        // ensure pi is accurate enough for sin and cos
        DOCTEST_CHECK_EQ(std::sin(pi<float>), ExpectFloat{0.f});
        DOCTEST_CHECK_EQ(std::cos(pi<float>), ExpectFloat{-1.f});
    }

    DOCTEST_TEST_CASE("halfPi") {
        // ensure halfPi is accurate enough for sin and cos
        DOCTEST_CHECK_EQ(std::sin(halfPi<float>), ExpectFloat{1.f});
        DOCTEST_CHECK_EQ(std::cos(halfPi<float>), ExpectFloat{0.f});
    }

    DOCTEST_TEST_CASE("radian degree conversion") {
        DOCTEST_CHECK_EQ(halfPi<float> * radiansToDegrees<float>, ExpectFloat{90.f});
        DOCTEST_CHECK_EQ(90.f * degreesToRadians<float>, ExpectFloat{halfPi<float>});
    }
}
