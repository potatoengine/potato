// Copyright (C) 2014,2015,2019 Sean Middleditch, all rights reserverd.

#pragma once

namespace gm::_detail {
    template <template <auto, auto> typename F, auto... V>
    struct reduce_t;

    template <template <auto, auto> typename F, auto V>
    struct reduce_t<F, V> { static constexpr auto value = V; };

    template <template <auto, auto> typename F, auto V, auto... R>
    struct reduce_t<F, V, R...> { static constexpr auto value = F<V, reduce_t<F, R...>::value>::value; };
} // namespace gm::_detail

namespace gm {
    template <auto L, auto R>
    struct max_f { static constexpr auto value = L < R ? R : L; };

    template <auto L, auto R>
    struct min_f { static constexpr auto value = L < R ? L : R; };

    template <auto L, auto R>
    struct sum_f { static constexpr auto value = L + R; };

    template <template <auto, auto> typename F, auto... V>
    using reduce_t = typename _detail::reduce_t<F, V...>;

    template <template <auto, auto> typename F, auto... V>
    constexpr auto reduce_v = reduce_t<F, V...>::value;

    static_assert(reduce_v<max_f, -2, 3, 1> == 3);

    template <auto... V>
    constexpr auto max_v = reduce_v<max_f, V...>;

    static_assert(max_v<1, -7, 3> == 3);
    static_assert(max_v<-3, 7, 1> == 7);

    template <auto... V>
    constexpr auto min_v = reduce_v<min_f, V...>;

    static_assert(min_v<1, -7, 3> == -7);
    static_assert(min_v<1, 2, 3> == 1);

    template <auto... V>
    constexpr auto sum_v = reduce_v<sum_f, V...>;

    static_assert(sum_v<1, -7, 3> == -3);
    static_assert(sum_v<1, -7, -3> == -9);
} // namespace gm
