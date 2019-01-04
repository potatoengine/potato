// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#define _gm_PP_CAT(x, ...) x##__VA_ARGS__
#define GM_PP_CAT(x, ...) _gm_PP_CAT(x, __VA_ARGS__)

static_assert(GM_PP_CAT(tr, ue));

#define GM_PP_COMMA ,

static_assert((false GM_PP_COMMA true));

#define _gm_STRINGIFY(x) #x
#define GM_STRINGIFY(x) _gm_STRINGIFY(x)

static_assert(GM_STRINGIFY(abcd)[2] == 'c');

#define GM_PP_IF(cond) GM_PP_CAT(_gm_PP_IF, cond)
#define _gm_PP_IF0(yes, no) no
#define _gm_PP_IF1(yes, no) yes

static_assert(GM_PP_IF(1)(42, 0) == 42);
static_assert(GM_PP_IF(0)(42, 0) == 0);

#define GM_PP_IGNORE(...)
#define GM_PP_EXPAND(...) __VA_ARGS__

static_assert(GM_PP_IGNORE(2) + 2 == 2);
static_assert((GM_PP_EXPAND(7, 2) + 2) == 4);

#define GM_PP_SELECT(cond) GM_PP_IF(cond)(GM_PP_EXPAND, GM_PP_IGNORE)

static_assert(GM_PP_SELECT(1)(2) + 2 == 4);
static_assert(GM_PP_SELECT(0)(2) + 2 == 2);

#define _gm_PP_NUM_ARITY(_9, _8, _7, _6, _5, _4, _3, _2, _1, _0, N, ...) N
#define GM_PP_ARITY(...) GM_PP_EXPAND(_gm_PP_NUM_ARITY(ignore, ##__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

static_assert(GM_PP_ARITY(a, b, c, d) == 4);
static_assert(GM_PP_ARITY(a) == 1);

#define _gm_PP_APPLY0(t, ignore)
#define _gm_PP_APPLY1(t, a) t(a)
#define _gm_PP_APPLY2(t, a, b) t(a) t(b)
#define _gm_PP_APPLY3(t, a, b, c) t(a) t(b) t(c)
#define _gm_PP_APPLY4(t, a, b, c, d) t(a) t(b) t(c) t(d)
#define _gm_PP_APPLY5(t, a, b, c, d, e) t(a) t(b) t(c) t(d) t(e)
#define _gm_PP_APPLY6(t, a, b, c, d, e, f) t(a) t(b) t(c) t(d) t(e) t(f)

#define _gm_PP_APPLY_INVOKE(t, n, ...) GM_PP_EXPAND(_gm_PP_APPLY##n(t, __VA_ARGS__))
#define _gm_PP_APPLY(t, n, ...) _gm_PP_APPLY_INVOKE(t, n, __VA_ARGS__)
#define GM_PP_APPLY(t, ...) _gm_PP_APPLY(t, GM_PP_ARITY(__VA_ARGS__), __VA_ARGS__)

#define _gm_PP_APPLY_TEST(x) +x
static_assert(GM_PP_APPLY(_gm_PP_APPLY_TEST, 1, 2, 3) == 6);
