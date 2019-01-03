#include "doctest.h"
#include "grimm/math/vector.h"
#include <iostream>

namespace gm {
    std::ostream& operator<<(std::ostream& os, gm::Vector4f vec) {
        os << '{' << vec[0];
        for (int i = 1; i < vec.length; ++i) {
            os << ',' << vec[i];
        }
        return os << '}';
    }
} // namespace gm

DOCTEST_TEST_SUITE("[grimm][math] Vector4f") {
    DOCTEST_TEST_CASE("initialization") {
        using namespace gm;

        Vector4f zero;

        DOCTEST_CHECK_EQ(zero.x(), 0.f);
        DOCTEST_CHECK_EQ(zero.y(), 0.f);
        DOCTEST_CHECK_EQ(zero.z(), 0.f);
        DOCTEST_CHECK_EQ(zero.w(), 0.f);

        Vector pv{1.f, 2.f, 3.f, 4.f};

        DOCTEST_CHECK_EQ(pv.x(), 1.f);
        DOCTEST_CHECK_EQ(pv.y(), 2.f);
        DOCTEST_CHECK_EQ(pv.z(), 3.f);
        DOCTEST_CHECK_EQ(pv.w(), 4.f);
    }

    DOCTEST_TEST_CASE("equalities") {
        using namespace gm;

        DOCTEST_CHECK_EQ(Vector4f{}, Vector4f{});
        DOCTEST_CHECK_EQ(Vector{4.f, 3.f, 2.f, 1.f}, Vector{4.f, 3.f, 2.f, 1.f});
        DOCTEST_CHECK_NE(Vector{1.f, 2.f, 3.f, 4.f}, Vector{4.f, 3.f, 2.f, 1.f});
    }

    DOCTEST_TEST_CASE("arithmetic") {
        using namespace gm;

        Vector a{1.f, 2.f, 3.f, 4.f};
        Vector b{-1.f, 4.f, -6.f, 8.f};

        DOCTEST_CHECK_EQ(a + b, Vector{0.f, 6.f, -3.f, 12.f});
        DOCTEST_CHECK_EQ(a - b, Vector{2.f, -2.f, 9.f, -4.f});
        DOCTEST_CHECK_EQ(a * b, Vector{-1.f, 8.f, -18.f, 32.f});
        DOCTEST_CHECK_EQ(b / a, Vector{-1.f, 2.f, -2.f, 2.f});

        DOCTEST_CHECK_EQ(1.f + b, Vector{0.f, 5.f, -5.f, 9.f});
        DOCTEST_CHECK_EQ(b + 1.f, Vector{0.f, 5.f, -5.f, 9.f});
    }
}
