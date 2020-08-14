// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/tools/evaluator.h"

#include <catch2/catch.hpp>

TEST_CASE("Evaluator", "[potato][tools]") {
    using namespace up;
    using namespace up::tools;

    SECTION("empty") {
        EvalEngine eval;
        EvalContext ctx;

        CHECK_FALSE(eval.evaluate(ctx, {}));
    }

    SECTION("boolean values") {
        EvalEngine eval;
        EvalContext ctx;

        ctx.set("t", true);
        ctx.set("f", false);

        CHECK(eval.evaluate(ctx, eval.compile("t")));
        CHECK_FALSE(eval.evaluate(ctx, eval.compile("f")));

        CHECK_FALSE(eval.evaluate(ctx, eval.compile("!t")));
        CHECK(eval.evaluate(ctx, eval.compile("!f")));

        CHECK(eval.evaluate(ctx, eval.compile("t || f")));
        CHECK_FALSE(eval.evaluate(ctx, eval.compile("t && f")));

        ctx.clear("t");

        CHECK_FALSE(eval.evaluate(ctx, eval.compile("t")));

        ctx.set("t", true);
        ctx.set("t", false);

        CHECK_FALSE(eval.evaluate(ctx, eval.compile("t")));
    }

    SECTION("string comparison") {
        EvalEngine eval;
        EvalContext ctx;

        ctx.set("color", "red");
        ctx.set("type", "int");

        CHECK(eval.evaluate(ctx, eval.compile("color == 'red'")));
        CHECK_FALSE(eval.evaluate(ctx, eval.compile("color != 'red'")));

        CHECK_FALSE(eval.evaluate(ctx, eval.compile("color == 'blue'")));
        CHECK(eval.evaluate(ctx, eval.compile("color != 'blue'")));

        CHECK_FALSE(eval.evaluate(ctx, eval.compile("color == type")));
        CHECK(eval.evaluate(ctx, eval.compile("color != type")));

        CHECK(eval.evaluate(ctx, eval.compile("type == type")));
        CHECK_FALSE(eval.evaluate(ctx, eval.compile("type != type")));
    }

    SECTION("parenthesis") {
        EvalEngine eval;
        EvalContext ctx;

        ctx.set("color", "red");
        ctx.set("bool", true);

        CHECK(eval.evaluate(ctx, eval.compile("bool && (color == 'red')")));
        CHECK(eval.evaluate(ctx, eval.compile("!bool || (color == 'red')")));

        CHECK(eval.evaluate(ctx, eval.compile("(color == 'green') || (color == 'red')")));
        CHECK(eval.evaluate(ctx, eval.compile("(!bool || color == 'red') && bool")));
        CHECK_FALSE(eval.evaluate(ctx, eval.compile("(!bool || color == 'green') && bool")));
    }

    SECTION("multiple contexts, same expression") {
        EvalEngine eval;
        EvalContext ctx1;
        EvalContext ctx2;

        ctx1.set("first", true);
        ctx2.set("first", false);

        auto const expr = eval.compile("first");

        CHECK(eval.evaluate(ctx1, expr));
        CHECK_FALSE(eval.evaluate(ctx2, expr));
    }
}
