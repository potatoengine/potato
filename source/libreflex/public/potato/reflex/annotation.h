// Copyright (C) 2019 Potato Engine authors and contributors, all rights reserverd.

#pragma once

#include <potato/spud/traits.h>
#include <tuple>

namespace up::reflex {
    template <typename... AnnotationTypes>
    using Annotations = std::tuple<AnnotationTypes...>;

    namespace _detail {
        template <typename T, typename Tuple, int N>
        struct IndexOfAnnotationHelper;

        template <typename T, int N, typename... Types>
        struct IndexOfAnnotationHelper<T, std::tuple<T, Types...>, N> {
            static const int value = N;
        };

        template <typename T, int N, typename U, typename... Types>
        struct IndexOfAnnotationHelper<T, std::tuple<U, Types...>, N> {
            static const int value = IndexOfAnnotationHelper<T, std::tuple<Types...>, 1 + N>::value;
        };

        template <typename T, int N>
        struct IndexOfAnnotationHelper<T, std::tuple<>, N> {
            static const int value = -1;
        };

        template <typename T, typename Tuple>
        constexpr int IndexOfAnnotation = IndexOfAnnotationHelper<T, remove_cvref_t<Tuple>, 0>::value;
    } // namespace _detail

    template <typename Type, typename Callback, typename Annotations>
    constexpr bool ApplyAnnotation(Annotations&& annotations, Callback&& callback) {
        constexpr auto index = _detail::IndexOfAnnotation<Type, Annotations>;
        if constexpr (index != -1) {
            std::forward<Callback>(callback)(std::get<index>(annotations));
            return true;
        }
        else {
            return false;
        }
    }
} // namespace up::reflex
