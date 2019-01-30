#include "doctest.h"
#include "stream_vector.h"

#include "grimm/foundation/traits.h"
#include "grimm/math/packed.h"
#include "grimm/math/swizzle.h"
#include "grimm/math/vector.h"

template <typename T>
using can_xyzw = decltype(gm::swizzle::xyzw(*(T*)0));
template <typename T>
using can_xyz = decltype(gm::swizzle::xyz(*(T*)0));

DOCTEST_TEST_SUITE("[grimm][math] swizzle") {
    using namespace gm;
    using namespace gm::swizzle;

    DOCTEST_TEST_CASE("swizzle2") {
        Packed pv{1.f, 2.f, 3.f, 4.f};

        DOCTEST_CHECK_EQ(xy(pv), Packed{1.f, 2.f});
        DOCTEST_CHECK_EQ(yx(pv), Packed{2.f, 1.f});
        DOCTEST_CHECK_EQ(yz(pv), Packed{2.f, 3.f});
        DOCTEST_CHECK_EQ(zw(pv), Packed{3.f, 4.f});
        DOCTEST_CHECK_EQ(wz(pv), Packed{4.f, 3.f});

        Vec4 v{1.f, 2.f, 3.f, 4.f};

        DOCTEST_CHECK_EQ(xy(v), Vec4{1.f, 2.f, 3.f, 4.f});
        DOCTEST_CHECK_EQ(yx(v), Vec4{2.f, 1.f, 3.f, 4.f});
        DOCTEST_CHECK_EQ(yz(v), Vec4{2.f, 3.f, 3.f, 4.f});
        DOCTEST_CHECK_EQ(zw(v), Vec4{3.f, 4.f, 3.f, 4.f});
        DOCTEST_CHECK_EQ(wz(v), Vec4{4.f, 3.f, 3.f, 4.f});
    }

    DOCTEST_TEST_CASE("swizzle3") {
        Packed pv{1.f, 2.f, 3.f, 4.f};

        DOCTEST_CHECK_EQ(xyz(pv), Packed{1.f, 2.f, 3.f});
        DOCTEST_CHECK_EQ(yzw(pv), Packed{2.f, 3.f, 4.f});
        DOCTEST_CHECK_EQ(zyx(pv), Packed{3.f, 2.f, 1.f});

        Vec4 v{1.f, 2.f, 3.f, 4.f};

        DOCTEST_CHECK_EQ(xyz(v), Vec4{1.f, 2.f, 3.f, 4.f});
        DOCTEST_CHECK_EQ(yzw(v), Vec4{2.f, 3.f, 4.f, 4.f});
        DOCTEST_CHECK_EQ(zyx(v), Vec4{3.f, 2.f, 1.f, 4.f});
    }

    DOCTEST_TEST_CASE("swizzle4") {
        Packed pv{1.f, 2.f, 3.f, 4.f};

        DOCTEST_CHECK_EQ(xyzw(pv), Packed{1.f, 2.f, 3.f, 4.f});
        DOCTEST_CHECK_EQ(wzyx(pv), Packed{4.f, 3.f, 2.f, 1.f});

        Vec4 v{1.f, 2.f, 3.f, 4.f};

        DOCTEST_CHECK_EQ(xyzw(v), Vec4{1.f, 2.f, 3.f, 4.f});
        DOCTEST_CHECK_EQ(wzyx(v), Vec4{4.f, 3.f, 2.f, 1.f});
    }

    DOCTEST_TEST_CASE("swizzle2to4") {
        Packed pv{1.f, 2.f};

        DOCTEST_CHECK_EQ(xxxx(pv), Packed{1.f, 1.f, 1.f, 1.f});

        // won't compile - guaranteed in static_asserts
        //DOCTEST_CHECK_EQ(xyzw(pv), Packed{1.f, 1.f, 1.f, 1.f});

        // illustrate that can_xyz[w] succeed when required
        static_assert(is_detected_v<can_xyzw, Packed4>);
        static_assert(is_detected_v<can_xyz, Packed3>);

        // illustrate that they fail when expected
        static_assert(!is_detected_v<can_xyzw, Packed3>);
        static_assert(!is_detected_v<can_xyzw, Packed2>);
        static_assert(!is_detected_v<can_xyz, Packed2>);
    }
}
