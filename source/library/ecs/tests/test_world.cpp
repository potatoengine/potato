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

        world.createEntity(Test1{'f'}, Second{'g', 7.f});
        world.createEntity(Second{'g', 9.f}, Another{1.f, 2.f});
        world.createEntity(Second{'h', -2.f}, Another{2.f, 1.f});
        world.createEntity(Test1{'j'}, Another{3.f, 4.f});

        size_t invokeCount = 0;
        size_t entityCount = 0;
        float weight = 0;

        world.select<Second>([&](size_t count, Second* second) {
            ++invokeCount;
            entityCount += count;

            for (size_t index = 0; index != count; ++index) {
                weight += second[index].b;
            }
        });

        // Only two archetypes should have been created that match
        DOCTEST_CHECK_EQ(2, invokeCount);

        // Three total entities between the two archetypes should exist that match
        DOCTEST_CHECK_EQ(3, entityCount);

        // Ensure we're storing/retrieving correct values
        DOCTEST_CHECK_EQ(14.f, weight);
    }
}
