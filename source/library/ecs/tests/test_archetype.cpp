#include "potato/ecs/archetype.h"
#include <doctest/doctest.h>

struct Test1 {
    char a;
};
struct Second {
    char a;
    float b;
};
struct Another {
    float a;
    float b;
};

DOCTEST_TEST_SUITE("[potato][ecs] Archetype") {
    using namespace up;

    DOCTEST_TEST_CASE("Archetype queries") {
        Archetype arch1{vector{getComponentInfo<Test1>(), getComponentInfo<Second>()}};
        Archetype arch2{vector{getComponentInfo<Second>(), getComponentInfo<Another>()}};

        Query queryTest1{getComponentId<Test1>()};
        Query queryTest1Second{getComponentId<Test1>(), getComponentId<Second>()};
        Query querySecond{getComponentId<Second>()};
        Query queryAnother{getComponentId<Another>()};

        DOCTEST_CHECK(arch1.matches(queryTest1));
        DOCTEST_CHECK(arch1.matches(queryTest1Second));
        DOCTEST_CHECK(arch1.matches(querySecond));
        DOCTEST_CHECK_FALSE(arch1.matches(queryAnother));

        DOCTEST_CHECK_FALSE(arch2.matches(queryTest1));
        DOCTEST_CHECK_FALSE(arch2.matches(queryTest1Second));
        DOCTEST_CHECK(arch2.matches(querySecond));
        DOCTEST_CHECK(arch2.matches(queryAnother));
    }

    DOCTEST_TEST_CASE("Archetype selects") {
        Archetype arch{vector{getComponentInfo<Test1>(), getComponentInfo<Second>()}};
        Query query{getComponentId<Test1>(), getComponentId<Second>()};

        arch.allocateEntity();
        arch.allocateEntity();
        arch.allocateEntity();
        arch.allocateEntity();
        arch.allocateEntity();

        size_t total = 0;
        arch.unsafeSelect(query, [&total](size_t count, view<void*> arrays) {
            total += count;
        });
        DOCTEST_CHECK_EQ(5, total);
    }
}
