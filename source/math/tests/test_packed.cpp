#include "doctest.h"
#include "grimm/math/packed.h"
#include <iostream>

namespace gm {
    template <typename T, int N>
    std::ostream& operator<<(std::ostream& os, gm::PackedVector<T, N> const& vec) {
        os << '{' << vec.u[0];
        for (int i = 1; i < vec.component_length; ++i) {
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

        PackedVector4f pv2{PackedVector3f{7.f, -7.f, 0.f}, -1.f};

        DOCTEST_CHECK_EQ(pv2.m.x, 7.f);
        DOCTEST_CHECK_EQ(pv2.m.y, -7.f);
        DOCTEST_CHECK_EQ(pv2.m.z, 0.f);
        DOCTEST_CHECK_EQ(pv2.m.w, -1.f);
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
        DOCTEST_CHECK_EQ(a - b, PackedVector{2.f, -2.f, 9.f, -4.f});
        DOCTEST_CHECK_EQ(a * b, PackedVector{-1.f, 8.f, -18.f, 32.f});
        DOCTEST_CHECK_EQ(b / a, PackedVector{-1.f, 2.f, -2.f, 2.f});

        DOCTEST_CHECK_EQ(1.f + b, PackedVector{0.f, 5.f, -5.f, 9.f});
        DOCTEST_CHECK_EQ(b + 1.f, PackedVector{0.f, 5.f, -5.f, 9.f});
    }
}
