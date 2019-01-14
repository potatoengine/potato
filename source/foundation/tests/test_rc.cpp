#include "grimm/foundation/rc.h"
#include "doctest.h"

namespace {
    struct TestCounted {
        void addRef() { ++refs; }
        void removeRef() { --refs; }
        int refs = 1;
    };

    struct TestShared : gm::shared<TestShared> {
        TestShared(bool& bind) : active(bind) { active = true; }
        ~TestShared() { active = false; }

        bool& active;
    };
} // namespace

DOCTEST_TEST_SUITE("[grimm][foundation] gm::rc") {
    using namespace gm;

    DOCTEST_TEST_CASE("empty rc") {
        rc<TestCounted> r;

        DOCTEST_CHECK(r.empty());
        DOCTEST_CHECK(!r);
    }

    DOCTEST_TEST_CASE("custom rc") {
        TestCounted test;

        rc r(&test);

        DOCTEST_CHECK(!r.empty());
        DOCTEST_CHECK_EQ(test.refs, 1);

        rc r2 = r;

        DOCTEST_CHECK(!r.empty());
        DOCTEST_CHECK(!r2.empty());
        DOCTEST_CHECK_EQ(test.refs, 2);

        r2.reset();

        DOCTEST_CHECK(!r.empty());
        DOCTEST_CHECK(r2.empty());
        DOCTEST_CHECK_EQ(test.refs, 1);

        r.reset();

        DOCTEST_CHECK(r.empty());
        DOCTEST_CHECK_EQ(test.refs, 0);
    }

    DOCTEST_TEST_CASE("custom rc") {
        bool active;
        rc r = make_shared<TestShared>(active);

        DOCTEST_CHECK(!r.empty());
        DOCTEST_CHECK(active);

        rc r2 = r;

        DOCTEST_CHECK(!r.empty());
        DOCTEST_CHECK(!r2.empty());
        DOCTEST_CHECK(active);

        r2.reset();

        DOCTEST_CHECK(!r.empty());
        DOCTEST_CHECK(r2.empty());
        DOCTEST_CHECK(active);

        r.reset();

        DOCTEST_CHECK(r.empty());
        DOCTEST_CHECK(!active);
    }
}
