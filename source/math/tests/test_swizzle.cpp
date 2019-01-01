#include "doctest.h"
#include <iostream>

#include "grimm/math/packed.h"
#include "grimm/math/swizzle.h"

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

DOCTEST_TEST_SUITE("[grimm][math] swizzle") {
    using namespace gm;
    using namespace gm::swizzle;

    DOCTEST_TEST_CASE("swizzle2") {
        PackedVector pv{1.f, 2.f, 3.f, 4.f};

        DOCTEST_CHECK_EQ(xy(pv), PackedVector{1.f, 2.f});
        DOCTEST_CHECK_EQ(yx(pv), PackedVector{2.f, 1.f});
        DOCTEST_CHECK_EQ(yz(pv), PackedVector{2.f, 3.f});
        DOCTEST_CHECK_EQ(zw(pv), PackedVector{3.f, 4.f});
        DOCTEST_CHECK_EQ(wz(pv), PackedVector{4.f, 3.f});
    }

    DOCTEST_TEST_CASE("swizzle3") {
        PackedVector pv{1.f, 2.f, 3.f, 4.f};

        DOCTEST_CHECK_EQ(xyz(pv), PackedVector{1.f, 2.f, 3.f});
        DOCTEST_CHECK_EQ(yzw(pv), PackedVector{2.f, 3.f, 4.f});
        DOCTEST_CHECK_EQ(zyx(pv), PackedVector{3.f, 2.f, 1.f});
    }

    DOCTEST_TEST_CASE("swizzle4") {
        PackedVector pv{1.f, 2.f, 3.f, 4.f};

        DOCTEST_CHECK_EQ(xyzw(pv), PackedVector{1.f, 2.f, 3.f, 4.f});
        DOCTEST_CHECK_EQ(wzyx(pv), PackedVector{4.f, 3.f, 2.f, 1.f});
    }
}
