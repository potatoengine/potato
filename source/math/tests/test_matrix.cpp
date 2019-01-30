#include "doctest.h"
#include "expect_float.h"
#include "grimm/math/matrix.h"
#include "grimm/math/vector.h"
#include "grimm/math/constants.h"
#include "stream_vector.h"

DOCTEST_TEST_SUITE("[grimm][math] Matrix4f") {
    using namespace gm;

    DOCTEST_TEST_CASE("identity") {
        Mat4x4 ident;
        Vec4 value{1, 2, 3, 4};

        DOCTEST_CHECK_EQ(value * ident, value);
    }

    DOCTEST_TEST_CASE("rotation") {
        Vec4 value{2, 3, 4, 0};

        Mat4x4 y90 = rotationY(constants::halfPi<float>);
        DOCTEST_CHECK_EQ(value * y90, ExpectVector{Vec4{4, 3, -2, 0}});

        Mat4x4 z90 = rotationZ(constants::halfPi<float>);
        DOCTEST_CHECK_EQ(value * z90, ExpectVector{Vec4{3, -2, 4, 0}});
    }
}
