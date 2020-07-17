// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/tools/evaluator.h"

#include <doctest/doctest.h>

DOCTEST_TEST_SUITE("[potato][tools] Evaluator") {
    using namespace up;
    using namespace up::tools;

    DOCTEST_TEST_CASE("empty") {
        Evaluator eval;

        DOCTEST_CHECK_EQ(false, eval.evaluate({}));
    }

    DOCTEST_TEST_CASE("boolean values") {
        Evaluator eval;

        eval.set("t", true);
        eval.set("f", false);

        DOCTEST_CHECK_EQ(true, eval.evaluate(eval.compile("t")));
        DOCTEST_CHECK_EQ(false, eval.evaluate(eval.compile("f")));

        DOCTEST_CHECK_EQ(false, eval.evaluate(eval.compile("!t")));
        DOCTEST_CHECK_EQ(true, eval.evaluate(eval.compile("!f")));

        DOCTEST_CHECK_EQ(true, eval.evaluate(eval.compile("t || f")));
        DOCTEST_CHECK_EQ(false, eval.evaluate(eval.compile("t && f")));

        eval.clear("t");

        DOCTEST_CHECK_EQ(false, eval.evaluate(eval.compile("t")));

        eval.set("t", true);
        eval.set("t", false);

        DOCTEST_CHECK_EQ(false, eval.evaluate(eval.compile("t")));
    }

    DOCTEST_TEST_CASE("string comparison") {
        Evaluator eval;

        eval.set("color", "red");
        eval.set("type", "int");

        DOCTEST_CHECK_EQ(true, eval.evaluate(eval.compile("color == 'red'")));
        DOCTEST_CHECK_EQ(false, eval.evaluate(eval.compile("color != 'red'")));

        DOCTEST_CHECK_EQ(false, eval.evaluate(eval.compile("color == 'blue'")));
        DOCTEST_CHECK_EQ(true, eval.evaluate(eval.compile("color != 'blue'")));

        DOCTEST_CHECK_EQ(false, eval.evaluate(eval.compile("color == type")));
        DOCTEST_CHECK_EQ(true, eval.evaluate(eval.compile("color != type")));

        DOCTEST_CHECK_EQ(true, eval.evaluate(eval.compile("type == type")));
        DOCTEST_CHECK_EQ(false, eval.evaluate(eval.compile("type != type")));
    }

    DOCTEST_TEST_CASE("parenthesis") {
        Evaluator eval;

        eval.set("color", "red");
        eval.set("bool", true);

        DOCTEST_CHECK_EQ(true, eval.evaluate(eval.compile("bool && (color == 'red')")));
        DOCTEST_CHECK_EQ(true, eval.evaluate(eval.compile("!bool || (color == 'red')")));

        DOCTEST_CHECK_EQ(true, eval.evaluate(eval.compile("(color == 'green') || (color == 'red')")));
        DOCTEST_CHECK_EQ(true, eval.evaluate(eval.compile("(!bool || color == 'red') && bool")));
        DOCTEST_CHECK_EQ(false, eval.evaluate(eval.compile("(!bool || color == 'green') && bool")));
    }
}
