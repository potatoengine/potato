#include "potato/ecs/world.h"
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

DOCTEST_TEST_SUITE("[potato][ecs] World") {
    using namespace up;

    DOCTEST_TEST_CASE("Archetype selects") {
        World world;
        Query query{getComponentId<Test1>(), getComponentId<Second>()};

        world.createEntity(Test1{'f'}, Second{'g', 7.f});

        size_t total = 0;
        float weight = 0;
        world.unsafeSelect(query, [&total, &weight](size_t count, view<void*> arrays) {
            total += count;
            weight += static_cast<Second const*>(arrays[1])->b;
        });
        DOCTEST_CHECK_EQ(1, total);
        DOCTEST_CHECK_EQ(7.f, weight);
    }
}
