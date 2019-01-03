#include "doctest.h"
#include "grimm/math/vector.h"
#include "expect_float.h"
#include <iostream>

namespace gm {
    std::ostream& operator<<(std::ostream& os, gm::Vector4f vec) {
        os << '{' << vec[0];
        for (int i = 1; i < vec.component_length; ++i) {
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

    DOCTEST_TEST_CASE("linear algebra") {
        using namespace gm;

        Vector a{1.f, 2.f, 3.f, 4.f};
        Vector b{-1.f, 4.f, -6.f, 8.f};

        DOCTEST_CHECK_EQ(dot(a, b), ExpectFloat{-1.f + 8.f + -18.f + 32.f});
        DOCTEST_CHECK_EQ(squareLength(a), ExpectFloat{1.f + 4.f + 9.f + 16.f});
        DOCTEST_CHECK_EQ(length(a), ExpectFloat{std::sqrt(1.f + 4.f + 9.f + 16.f)});
        DOCTEST_CHECK_EQ(reciprocal(a), ExpectVector{Vector4f{1.f, 1 / 2.f, 1 / 3.f, 1 / 4.f}});
        DOCTEST_CHECK_EQ(normalize(a), ExpectVector{Vector4f{0.1826f, 0.3651f, 0.5477f, 0.7303f}}); // https: //www.wolframalpha.com/input/?i=normalize(%7B1.f,+2.f,+3.f,+4.f%7D)
        DOCTEST_CHECK_EQ(lerp(a, b, 0.5f), ExpectVector{Vector4f{0.f, 3.f, -1.5f, 6.f}});
    }
}
