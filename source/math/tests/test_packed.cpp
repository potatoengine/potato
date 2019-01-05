#include "doctest.h"
#include "grimm/math/packed.h"
#include "stream_vector.h"

DOCTEST_TEST_SUITE("[grimm][math] PackedVector4f") {
    DOCTEST_TEST_CASE_TEMPLATE("initialization", T, double, float, int) {
        using namespace gm;

        PackedVector<T, 4> zero;

        DOCTEST_CHECK_EQ(zero.m.x, 0);
        DOCTEST_CHECK_EQ(zero.m.y, 0);
        DOCTEST_CHECK_EQ(zero.m.z, 0);
        DOCTEST_CHECK_EQ(zero.m.w, 0);

        PackedVector<T, 4> pv{1, 2, 3, 4};

        DOCTEST_CHECK_EQ(pv.m.x, 1);
        DOCTEST_CHECK_EQ(pv.m.y, 2);
        DOCTEST_CHECK_EQ(pv.m.z, 3);
        DOCTEST_CHECK_EQ(pv.m.w, 4);

        PackedVector<T, 4> pv2{PackedVector<T, 3>{7, -7, 0}, -1};

        DOCTEST_CHECK_EQ(pv2.m.x, 7);
        DOCTEST_CHECK_EQ(pv2.m.y, -7);
        DOCTEST_CHECK_EQ(pv2.m.z, 0);
        DOCTEST_CHECK_EQ(pv2.m.w, -1);
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
