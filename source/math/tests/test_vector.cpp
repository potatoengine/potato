#include "doctest.h"
#include "expect_float.h"
#include "grimm/math/vector.h"
#include "stream_vector.h"

DOCTEST_TEST_SUITE("[grimm][math] Vec4") {
    using namespace gm;

    DOCTEST_TEST_CASE("initialization") {
        Vec4 zero;

        DOCTEST_CHECK_EQ(zero.x(), 0.f);
        DOCTEST_CHECK_EQ(zero.y(), 0.f);
        DOCTEST_CHECK_EQ(zero.z(), 0.f);
        DOCTEST_CHECK_EQ(zero.w(), 0.f);

        Vec4 pv{1.f, 2.f, 3.f, 4.f};

        DOCTEST_CHECK_EQ(pv.x(), 1.f);
        DOCTEST_CHECK_EQ(pv.y(), 2.f);
        DOCTEST_CHECK_EQ(pv.z(), 3.f);
        DOCTEST_CHECK_EQ(pv.w(), 4.f);
    }

    DOCTEST_TEST_CASE("equalities") {
        DOCTEST_CHECK_EQ(Vec4{}, Vec4{});
        DOCTEST_CHECK_EQ(Vec4{4.f, 3.f, 2.f, 1.f}, Vec4{4.f, 3.f, 2.f, 1.f});
        DOCTEST_CHECK_NE(Vec4{1.f, 2.f, 3.f, 4.f}, Vec4{4.f, 3.f, 2.f, 1.f});
    }

    DOCTEST_TEST_CASE("arithmetic") {
        Vec4 a{1.f, 2.f, 3.f, 4.f};
        Vec4 b{-1.f, 4.f, -6.f, 8.f};

        DOCTEST_CHECK_EQ(a + b, Vec4{0.f, 6.f, -3.f, 12.f});
        DOCTEST_CHECK_EQ(a - b, Vec4{2.f, -2.f, 9.f, -4.f});
        DOCTEST_CHECK_EQ(a * b, Vec4{-1.f, 8.f, -18.f, 32.f});
        DOCTEST_CHECK_EQ(b / a, Vec4{-1.f, 2.f, -2.f, 2.f});

        DOCTEST_CHECK_EQ(1.f + b, Vec4{0.f, 5.f, -5.f, 9.f});
        DOCTEST_CHECK_EQ(b + 1.f, Vec4{0.f, 5.f, -5.f, 9.f});
    }

    DOCTEST_TEST_CASE("horizontal") {
        Vec4 a{1.f, 2.f, 3.f, 4.f};

        DOCTEST_CHECK_EQ(hmin(a), ExpectFloat{1.f});
        DOCTEST_CHECK_EQ(hmax(a), ExpectFloat{4.f});
        DOCTEST_CHECK_EQ(hsum(a), ExpectFloat{10.f});
    }

    DOCTEST_TEST_CASE("linear algebra") {
        Vec4 a{1.f, 2.f, 3.f, 4.f};
        Vec4 b{-1.f, 4.f, -6.f, 8.f};

        DOCTEST_CHECK_EQ(dot(a, b), ExpectFloat{-1.f + 8.f + -18.f + 32.f});
        DOCTEST_CHECK_EQ(dot3(a, b), ExpectFloat{-1.f + 8.f + -18.f});
        DOCTEST_CHECK_EQ(length(a), ExpectFloat{std::sqrt(1.f + 4.f + 9.f + 16.f)});
        DOCTEST_CHECK_EQ(reciprocal(a), ExpectVector{Vec4{1.f, 1 / 2.f, 1 / 3.f, 1 / 4.f}});
        DOCTEST_CHECK_EQ(normalize(a), ExpectVector{Vec4{0.1826f, 0.3651f, 0.5477f, 0.7303f}}); // https: //www.wolframalpha.com/input/?i=normalize(%7B1.f,+2.f,+3.f,+4.f%7D)
        DOCTEST_CHECK_EQ(lerp(a, b, 0.5f), ExpectVector{Vec4{0.f, 3.f, -1.5f, 6.f}});
    }
}
