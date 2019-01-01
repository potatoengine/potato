#include "doctest.h"
#include <iostream>

#define GM_MATH_ENABLE_SWIZZLE
#include "grimm/math/packed.h"

namespace gm {
    template <typename T, int N>
    std::ostream& operator<<(std::ostream& os, gm::PackedVector<T, N> const& vec) {
        os << '{' << vec.u[0];
        for (int i = 1; i < vec.length; ++i) {
            os << ',' << vec.u[i];
        }
        return os << '}';
    }
} // namespace gm

DOCTEST_TEST_SUITE("[grimm][math] PackedVector4f") {
    DOCTEST_TEST_CASE("initialization") {
        using namespace gm;

        PackedVector4f zero;

        DOCTEST_CHECK_EQ(zero.m.x, 0.f);
        DOCTEST_CHECK_EQ(zero.m.y, 0.f);
        DOCTEST_CHECK_EQ(zero.m.z, 0.f);
        DOCTEST_CHECK_EQ(zero.m.w, 0.f);

        PackedVector pv{1.f, 2.f, 3.f, 4.f};

        DOCTEST_CHECK_EQ(pv.m.x, 1.f);
        DOCTEST_CHECK_EQ(pv.m.y, 2.f);
        DOCTEST_CHECK_EQ(pv.m.z, 3.f);
        DOCTEST_CHECK_EQ(pv.m.w, 4.f);

        PackedVector4f pv2{PackedVector3f{1.f, 2.f, 3.f}, 1.f};

        DOCTEST_CHECK_EQ(pv.m.x, 1.f);
        DOCTEST_CHECK_EQ(pv.m.y, 2.f);
        DOCTEST_CHECK_EQ(pv.m.z, 3.f);
        DOCTEST_CHECK_EQ(pv.m.w, 4.f);
    }

    DOCTEST_TEST_CASE("swizzle") {
        using namespace gm;

        PackedVector pv{1.f, 2.f, 3.f, 4.f};

        DOCTEST_CHECK_EQ(pv.wzyx(), PackedVector{4.f, 3.f, 2.f, 1.f});
        DOCTEST_CHECK_EQ(pv.xyz(), PackedVector{1.f, 2.f, 3.f});
        DOCTEST_CHECK_EQ(pv.xy(), PackedVector{1.f, 2.f});
        DOCTEST_CHECK_EQ(pv.yx(), PackedVector{2.f, 1.f});
        DOCTEST_CHECK_EQ(pv.yx().yx(), PackedVector{1.f, 2.f});
    }

    DOCTEST_TEST_CASE("equalities") {
        using namespace gm;

        DOCTEST_CHECK_EQ(PackedVector4f{}, PackedVector4f{});
        DOCTEST_CHECK_EQ(PackedVector{4.f, 3.f, 2.f, 1.f}, PackedVector{4.f, 3.f, 2.f, 1.f});
        DOCTEST_CHECK_NE(PackedVector{1.f, 2.f, 3.f, 4.f}, PackedVector{4.f, 3.f, 2.f, 1.f});
    }

    DOCTEST_TEST_CASE("arithmetic") {
        using namespace gm;

        PackedVector a{1.f, 2.f, 3.f, 4.f};
        PackedVector b{-1.f, 4.f, -6.f, 8.f};

        DOCTEST_CHECK_EQ(a + b, PackedVector{0.f, 6.f, -3.f, 12.f});
    }
}
