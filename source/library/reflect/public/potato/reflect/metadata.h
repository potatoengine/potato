// Copyright (C) 2019 Potato Engine authors and contributors, all rights reserverd.

#pragma once

#include "_tag.h"
#include <potato/foundation/zstring_view.h>

namespace up::reflex {
    /// Metadata about a specific type.
    ///
    /// Only contains metadata for types, not class templates or other C++ entities.
    struct TypeInfo {
        zstring_view name;
        int size = 0;
        int alignment = 0;
    };

    namespace _detail {
        template <typename T>
        constexpr TypeInfo getTypeInfo(_detail::TypeTag<T>) noexcept {
            return TypeInfo{
                {},
                sizeof(T),
                alignof(T)};
        }
    } // namespace _detail

    /// Lookup the metadata for a given type.
    template <typename T>
    constexpr TypeInfo getTypeInfo() noexcept {
        using _detail::getTypeInfo;
        return getTypeInfo(_detail::TypeTag<T>{});
    }
} // namespace up::reflex
