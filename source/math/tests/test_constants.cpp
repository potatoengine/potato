#include "doctest.h"
#include "grimm/math/constants.h"
#include <cmath>
#include <iostream>

namespace {
    template <typename T>
    struct ExpectFloat {
        T value;

        ExpectFloat(T v) : value(v) {}

        friend bool operator==(T lhs, ExpectFloat rhs) {
            // FIXME: calculate appropriate epsilon scale vs a minimum tolerance
            // https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
            return std::fabs(lhs - rhs.value) < (T)0.0001;
        }

        operator T() const { return value; }
    };
} // namespace

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
