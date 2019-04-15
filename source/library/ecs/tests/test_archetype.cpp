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
        Archetype arch1{vector{getComponentId<Test1>(), getComponentId<Second>()}};
        Archetype arch2{vector{getComponentId<Second>(), getComponentId<Another>()}};

        Query queryTest1{getComponentId<Test1>()};
        Query queryTest1Second{getComponentId<Test1>(), getComponentId<Second>()};
        Query querySecond{getComponentId<Second>()};
        Query queryAnother{getComponentId<Another>()};

        DOCTEST_CHECK(arch1.matches(queryTest1.components()));
        DOCTEST_CHECK(arch1.matches(queryTest1Second.components()));
        DOCTEST_CHECK(arch1.matches(querySecond.components()));
        DOCTEST_CHECK_FALSE(arch1.matches(queryAnother.components()));

        DOCTEST_CHECK_FALSE(arch2.matches(queryTest1.components()));
        DOCTEST_CHECK_FALSE(arch2.matches(queryTest1Second.components()));
        DOCTEST_CHECK(arch2.matches(querySecond.components()));
        DOCTEST_CHECK(arch2.matches(queryAnother.components()));
    }

    DOCTEST_TEST_CASE("Archetype selects") {
        Archetype arch{vector{getComponentId<Test1>(), getComponentId<Second>()}};
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
