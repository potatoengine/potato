#include "potato/ecs/world.h"
#include "potato/ecs/query.h"
#include "potato/ecs/entity.h"
#include <doctest/doctest.h>

struct Test1 {
    char a;
};
struct Second {
    float b;
    char a;
};
struct Another {
    double a;
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

        world.createEntity(Test1{'f'}, Second{7.f, 'g'});
        world.createEntity(Another{1.f, 2.f}, Second{9.f, 'g'});
        world.createEntity(Second{-2.f, 'h'}, Another{2.f, 1.f});
        world.createEntity(Test1{'j'}, Another{3.f, 4.f});

        // Exactly two of the entities should be in the same archetype
        DOCTEST_CHECK_EQ(3, world.archetypes().size());

        size_t invokeCount = 0;
        size_t entityCount = 0;
        float weight = 0;

        Query<Second> query;
        query.selectChunks(world, [&](size_t count, Second* second) {
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

        world.createEntity(Test1{'f'}, Second{7.f, 'g'});
        world.createEntity(Another{1.f, 2.f}, Second{9.f, 'g'});
        auto id = world.createEntity(Second{-2.f, 'h'}, Another{2.f, 1.f});

        Second* test = world.getComponentSlow<Second>(id);
        DOCTEST_CHECK(test != nullptr);
        DOCTEST_CHECK_EQ('h', test->a);
        DOCTEST_CHECK_EQ(-2.f, test->b);
    }

    DOCTEST_TEST_CASE("EntityId management") {
        World world;

        EntityId first = world.createEntity(Test1{'f'}, Second{7.f, 'g'});
        EntityId second = world.createEntity(Test1{'h'}, Second{-1.f, 'i'});

        Query<Entity const, Test1> query;
        query.selectChunks(world, [&](size_t count, Entity const* entities, Test1*) {
            DOCTEST_CHECK_EQ(2, count);
            DOCTEST_CHECK_EQ(first, entities[0].id);
            DOCTEST_CHECK_EQ(second, entities[1].id);
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

        Query<Counter> query;
        query.selectChunks(world, [&](size_t count, Counter* counters) {
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

        // create some dummy entities
        //
        EntityId foo = world.createEntity(Test1{'a'});
        world.createEntity(Test1{'b'});
        EntityId bar = world.createEntity(Test1{'c'});
        world.createEntity(Test1{'d'});
        EntityId last = world.createEntity(Test1{'e'});

        // delete some entities (not the last one!)
        //
        world.deleteEntity(foo);
        world.deleteEntity(bar);

        // ensure deleted entities are gone
        //
        DOCTEST_CHECK_EQ(nullptr, world.getComponentSlow<Test1>(foo));
        DOCTEST_CHECK_EQ(nullptr, world.getComponentSlow<Test1>(bar));

        // overwrite emptied locations
        //
        world.createEntity(Test1{'x'});
        world.createEntity(Test1{'x'});

        // ensure that the last entity was moved properly
        //
        DOCTEST_CHECK_EQ('e', world.getComponentSlow<Test1>(last)->a);
    }

    DOCTEST_TEST_CASE("Remove Component") {
        bool found = false;
        World world;
        Query<Test1> queryTest1;
        Query<Second> querySecond;

        EntityId id = world.createEntity(Test1{}, Second{});

        querySecond.select(world, [&found](Second&) { found = true; });
        DOCTEST_CHECK(found);

        world.removeComponent(id, getComponentId<Second>());

        found = false;
        querySecond.select(world, [&found](Second&) { found = true; });
        DOCTEST_CHECK_FALSE(found);

        found = false;
        queryTest1.select(world, [&found](Test1&) { found = true; });
        DOCTEST_CHECK(found);
    }

    DOCTEST_TEST_CASE("Add Component") {
        bool found = false;
        World world;
        Query<Test1> queryTest1;
        Query<Second> querySecond;

        EntityId id = world.createEntity(Test1{});

        querySecond.select(world, [&found](Second&) { found = true; });
        DOCTEST_CHECK_FALSE(found);

        world.addComponent(id, Second{});

        found = false;
        querySecond.select(world, [&found](Second&) { found = true; });
        DOCTEST_CHECK(found);

        found = false;
        queryTest1.select(world, [&found](Test1&) { found = true; });
        DOCTEST_CHECK(found);
    }
}
