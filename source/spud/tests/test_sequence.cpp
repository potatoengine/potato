#include "potato/spud/sequence.h"
#include <doctest/doctest.h>

namespace {
    enum WeakEnum {
        A,
        B,
        C,
        Max
    };
    enum class StrongEnum {
        A,
        B,
        C,
        Max
    };
} // namespace

DOCTEST_TEST_SUITE("[potato][spud] up::sequence") {
    using namespace up;

    DOCTEST_TEST_CASE("sequence range") {
        DOCTEST_CHECK_EQ(sequence(10).size(), 10);
        DOCTEST_CHECK_FALSE(sequence(10).empty());

        if (auto seq = sequence(3); !seq.empty()) {
            auto it = seq.begin();
            DOCTEST_CHECK_EQ(*(it++), 0);
            DOCTEST_CHECK_EQ(*(it++), 1);
            DOCTEST_CHECK_EQ(*(it++), 2);

            DOCTEST_CHECK_EQ(it, seq.end());
        }
    }

    DOCTEST_TEST_CASE("sequence with initial value") {
        DOCTEST_CHECK_EQ(sequence(5, 10).size(), 5);
        DOCTEST_CHECK_FALSE(sequence(5, 10).empty());

        DOCTEST_CHECK_EQ(sequence(10, 10).size(), 0);
        DOCTEST_CHECK(sequence(10, 10).empty());

        if (auto seq = sequence(5, 10); !seq.empty()) {
            auto it = seq.begin();
            DOCTEST_CHECK_EQ(*(it++), 5);
            DOCTEST_CHECK_EQ(*(it++), 6);
            DOCTEST_CHECK_EQ(*(it++), 7);
            DOCTEST_CHECK_EQ(*(it++), 8);
            DOCTEST_CHECK_EQ(*(it++), 9);

            DOCTEST_CHECK_EQ(it, seq.end());
        }
    }

    DOCTEST_TEST_CASE("sequence with weak enum") {
        auto seq = sequence<WeakEnum>(A, Max);

        DOCTEST_CHECK_EQ(seq.size(), 3);

        auto it = seq.begin();
        DOCTEST_CHECK_EQ(*(it++), A);
        DOCTEST_CHECK_EQ(*(it++), B);
        DOCTEST_CHECK_EQ(*(it++), C);

        DOCTEST_CHECK_EQ(it, seq.end());
    }

    DOCTEST_TEST_CASE("sequence with strong enum") {
        auto seq = sequence<StrongEnum>(StrongEnum::A, StrongEnum::Max);

        DOCTEST_CHECK_EQ(seq.size(), 3);

        auto it = seq.begin();
        DOCTEST_CHECK_EQ(*(it++), StrongEnum::A);
        DOCTEST_CHECK_EQ(*(it++), StrongEnum::B);
        DOCTEST_CHECK_EQ(*(it++), StrongEnum::C);

        DOCTEST_CHECK_EQ(it, seq.end());
    }
}
