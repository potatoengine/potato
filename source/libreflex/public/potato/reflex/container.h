// Copyright (C) 2019 Potato Engine authors and contributors, all rights reserverd.

#pragma once

#include "reflect.h"
#include "potato/spud/vector.h"
#include "potato/spud/preprocessor.h"

namespace up::reflex {
    template <typename T>
    constexpr auto getTypeInfo(_detail::TypeTag<vector<T>>) noexcept -> TypeInfo {
        return reflex::TypeInfo{"vector", sizeof(vector<T>), alignof(vector<T>)};
    }

    template <typename R, typename T>
    void serialize_value(_detail::TypeTag<vector<T>>, R& reflect, zstring_view name = "vector") {
        reflect("size",
            [](vector<T> const& obj) noexcept -> typename vector<T>::size_type { return obj.size(); },
            [](vector<T>* obj, typename vector<T>::size_type size) { obj->resize(size); });
    }
}
