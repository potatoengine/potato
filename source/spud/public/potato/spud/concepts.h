// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include <type_traits>

namespace up {
    template <typename T, typename U>
    concept same_as = std::is_same_v<T, U>;

    template <typename F, typename... Args>
    concept callable = std::is_invocable_v<F, Args...>;

    template <typename F, typename R, typename... Args>
    concept callable_r = (callable<F, Args...>)&&std::is_convertible_v<std::invoke_result_t<F, Args...>, R>;

    template <typename F, typename... Args>
    concept predicate = callable_r<F, bool, Args...>;
} // namespace up
