// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/tools/evaluator.h"

#include <doctest/doctest.h>

DOCTEST_TEST_SUITE("[potato][tools] Evaluator") {
    using namespace up;
    using namespace up::tools;

    DOCTEST_TEST_CASE("empty") {
        EvalEngine eval;
        EvalContext ctx;

        DOCTEST_CHECK_EQ(false, eval.evaluate(ctx, {}));
    }

    DOCTEST_TEST_CASE("boolean values") {
        EvalEngine eval;
        EvalContext ctx;

        ctx.set("t", true);
        ctx.set("f", false);

        DOCTEST_CHECK_EQ(true, eval.evaluate(ctx, eval.compile("t")));
        DOCTEST_CHECK_EQ(false, eval.evaluate(ctx, eval.compile("f")));

        DOCTEST_CHECK_EQ(false, eval.evaluate(ctx, eval.compile("!t")));
        DOCTEST_CHECK_EQ(true, eval.evaluate(ctx, eval.compile("!f")));

        DOCTEST_CHECK_EQ(true, eval.evaluate(ctx, eval.compile("t || f")));
        DOCTEST_CHECK_EQ(false, eval.evaluate(ctx, eval.compile("t && f")));

        ctx.clear("t");

        DOCTEST_CHECK_EQ(false, eval.evaluate(ctx, eval.compile("t")));

        ctx.set("t", true);
        ctx.set("t", false);

        DOCTEST_CHECK_EQ(false, eval.evaluate(ctx, eval.compile("t")));
    }

    DOCTEST_TEST_CASE("string comparison") {
        EvalEngine eval;
        EvalContext ctx;

        ctx.set("color", "red");
        ctx.set("type", "int");

        DOCTEST_CHECK_EQ(true, eval.evaluate(ctx, eval.compile("color == 'red'")));
        DOCTEST_CHECK_EQ(false, eval.evaluate(ctx, eval.compile("color != 'red'")));

        DOCTEST_CHECK_EQ(false, eval.evaluate(ctx, eval.compile("color == 'blue'")));
        DOCTEST_CHECK_EQ(true, eval.evaluate(ctx, eval.compile("color != 'blue'")));

        DOCTEST_CHECK_EQ(false, eval.evaluate(ctx, eval.compile("color == type")));
        DOCTEST_CHECK_EQ(true, eval.evaluate(ctx, eval.compile("color != type")));

        DOCTEST_CHECK_EQ(true, eval.evaluate(ctx, eval.compile("type == type")));
        DOCTEST_CHECK_EQ(false, eval.evaluate(ctx, eval.compile("type != type")));
    }

    DOCTEST_TEST_CASE("parenthesis") {
        EvalEngine eval;
        EvalContext ctx;

        ctx.set("color", "red");
        ctx.set("bool", true);

        DOCTEST_CHECK_EQ(true, eval.evaluate(ctx, eval.compile("bool && (color == 'red')")));
        DOCTEST_CHECK_EQ(true, eval.evaluate(ctx, eval.compile("!bool || (color == 'red')")));

        DOCTEST_CHECK_EQ(true, eval.evaluate(ctx, eval.compile("(color == 'green') || (color == 'red')")));
        DOCTEST_CHECK_EQ(true, eval.evaluate(ctx, eval.compile("(!bool || color == 'red') && bool")));
        DOCTEST_CHECK_EQ(false, eval.evaluate(ctx, eval.compile("(!bool || color == 'green') && bool")));
    }

    DOCTEST_TEST_CASE("multiple contexts, same expression") {
        EvalEngine eval;
        EvalContext ctx1;
        EvalContext ctx2;

        ctx1.set("first", true);
        ctx2.set("first", false);

        auto const expr = eval.compile("first");

        DOCTEST_CHECK_EQ(true, eval.evaluate(ctx1, expr));
        DOCTEST_CHECK_EQ(false, eval.evaluate(ctx2, expr));
    }
}
