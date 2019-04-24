#include "potato/ecs/world.h"
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
struct Counter {
    int value;
    char padding[128];
};

UP_COMPONENT(Test1);
UP_COMPONENT(Second);
UP_COMPONENT(Another);
UP_COMPONENT(Counter);

DOCTEST_TEST_SUITE("[potato][ecs] World") {
    using namespace up;

    DOCTEST_TEST_CASE("Archetype selects") {
        World world;

        world.createEntity(Test1{'f'}, Second{'g', 7.f});
        world.createEntity(Another{1.f, 2.f}, Second{'g', 9.f});
        world.createEntity(Second{'h', -2.f}, Another{2.f, 1.f});
        world.createEntity(Test1{'j'}, Another{3.f, 4.f});

        // Exactly two of the entities should be in the same archetype
        DOCTEST_CHECK_EQ(3, world.archetypes().size());

        size_t invokeCount = 0;
        size_t entityCount = 0;
        float weight = 0;

        world.select<Second>([&](size_t count, EntityId const*, Second* second) {
            ++invokeCount;
            entityCount += count;

            for (size_t index = 0; index != count; ++index) {
                weight += second[index].b;
            }
        });

        // Only two archetypes should have matches
        DOCTEST_CHECK_EQ(2, invokeCount);

        // Three total entities between the two archetypes should exist that match
        DOCTEST_CHECK_EQ(3, entityCount);

        // Ensure we're storing/retrieving correct values
        DOCTEST_CHECK_EQ(14.f, weight);
    }

    DOCTEST_TEST_CASE("Direct component access") {
        World world;

        auto i1 = world.createEntity(Test1{'f'}, Second{'g', 7.f});
        auto i2 = world.createEntity(Another{1.f, 2.f}, Second{'g', 9.f});
        auto id = world.createEntity(Second{'h', -2.f}, Another{2.f, 1.f});

        Second* test = world.getComponentSlow<Second>(id);
        DOCTEST_CHECK(test != nullptr);
        DOCTEST_CHECK_EQ('h', test->a);
        DOCTEST_CHECK_EQ(-2.f, test->b);
    }

    DOCTEST_TEST_CASE("EntityId management") {
        World world;

        EntityId first = world.createEntity(Test1{'f'}, Second{'g', 7.f});
        EntityId second = world.createEntity(Test1{'h'}, Second{'i', -1.f});

        world.select([&](size_t count, EntityId const* ids) {
            DOCTEST_CHECK_EQ(2, count);
            DOCTEST_CHECK_EQ(first, ids[0]);
            DOCTEST_CHECK_EQ(second, ids[1]);
        });
    }

    DOCTEST_TEST_CASE("Chunks") {
        constexpr int count = 100000;
        World world;

        uint64 expectedSum = 0;
        for (int i = 0; i != count; ++i) {
            expectedSum += i;
            world.createEntity(Counter{i});
        }

        size_t chunks = 0;
        size_t total = 0;
        uint64 sum = 0;
        world.select<Counter>([&](size_t count, EntityId const*, Counter* counters) {
            ++chunks;
            total += count;
            for (size_t i = 0; i != count; ++i) {
                sum += counters[i].value;
            }
        });
        DOCTEST_CHECK_NE(0, chunks);
        DOCTEST_CHECK_EQ(count, total);
        DOCTEST_CHECK_EQ(expectedSum, sum);
    }

    DOCTEST_TEST_CASE("Creates and Deletes") {
        World world;

        EntityId foo = world.createEntity(Test1{'a'});
        world.createEntity(Test1{'b'});
        EntityId bar = world.createEntity(Test1{'c'});
        world.createEntity(Test1{'d'});
        world.createEntity(Test1{'e'});

        world.deleteEntity(foo);
        world.deleteEntity(bar);
    }
}
