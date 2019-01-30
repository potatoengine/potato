#include "doctest.h"
#include "grimm/math/packed.h"
#include "grimm/math/packed_op.h"
#include "stream_vector.h"

DOCTEST_TEST_SUITE("[grimm][math] Packed4") {
    DOCTEST_TEST_CASE_TEMPLATE("initialization", T, double, float, int) {
        using namespace gm;

        Packed<T, 4> zero;

        DOCTEST_CHECK_EQ(zero.x, 0);
        DOCTEST_CHECK_EQ(zero.y, 0);
        DOCTEST_CHECK_EQ(zero.z, 0);
        DOCTEST_CHECK_EQ(zero.w, 0);

        Packed<T, 4> pv{1, 2, 3, 4};

        DOCTEST_CHECK_EQ(pv.x, 1);
        DOCTEST_CHECK_EQ(pv.y, 2);
        DOCTEST_CHECK_EQ(pv.z, 3);
        DOCTEST_CHECK_EQ(pv.w, 4);
    }

    DOCTEST_TEST_CASE("equalities") {
        using namespace gm;

        DOCTEST_CHECK_EQ(Packed4{}, Packed4{});
        DOCTEST_CHECK_EQ(Packed{4.f, 3.f, 2.f, 1.f}, Packed{4.f, 3.f, 2.f, 1.f});
        DOCTEST_CHECK_NE(Packed{1.f, 2.f, 3.f, 4.f}, Packed{4.f, 3.f, 2.f, 1.f});
    }

    DOCTEST_TEST_CASE("arithmetic") {
        using namespace gm;

        Packed a{1.f, 2.f, 3.f, 4.f};
        Packed b{-1.f, 4.f, -6.f, 8.f};

        DOCTEST_CHECK_EQ(a + b, Packed{0.f, 6.f, -3.f, 12.f});
        DOCTEST_CHECK_EQ(a - b, Packed{2.f, -2.f, 9.f, -4.f});
        DOCTEST_CHECK_EQ(a * b, Packed{-1.f, 8.f, -18.f, 32.f});
        DOCTEST_CHECK_EQ(b / a, Packed{-1.f, 2.f, -2.f, 2.f});

        DOCTEST_CHECK_EQ(1.f + b, Packed{0.f, 5.f, -5.f, 9.f});
        DOCTEST_CHECK_EQ(b + 1.f, Packed{0.f, 5.f, -5.f, 9.f});
    }
}
