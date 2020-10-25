// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/span.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

namespace up::reflex {
    struct SchemaType;
    struct Schema;

    enum class SchemaPrimitive {
        Null,
        Int8,
        Int16,
        Int32,
        Int64,
        UInt8,
        UInt16,
        UInt32,
        UInt64,
        Float,
        Double,
        Pointer,
        String,
        Array,
        Object
    };

    struct SchemaField {
        zstring_view name;
        Schema const* schema = nullptr;
        int offset = 0;
    };

    struct Schema {
        zstring_view name;
        SchemaPrimitive primitive = SchemaPrimitive::Null;
        Schema const* elementType = nullptr;
        view<SchemaField> fields;
    };

    template <typename T>
    struct SchemaHolder;

    template <typename T>
    constexpr Schema const& getSchema() noexcept {
        using Type = std::remove_cv_t<std::decay_t<T>>;
        if constexpr (std::is_same_v<Type, int8>) {
            static constexpr Schema schema{.name = "int8"_zsv, .primitive = SchemaPrimitive::Int8};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, int16>) {
            static constexpr Schema schema{.name = "int16"_zsv, .primitive = SchemaPrimitive::Int16};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, int32>) {
            static constexpr Schema schema{.name = "int32"_zsv, .primitive = SchemaPrimitive::Int32};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, int64>) {
            static constexpr Schema schema{.name = "int64"_zsv, .primitive = SchemaPrimitive::Int64};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, string>) {
            static constexpr Schema schema{.name = "string"_zsv, .primitive = SchemaPrimitive::String};
            return schema;
        }
        else if constexpr (is_vector_v<Type>) {
            static Schema const& elementSchema = getSchema<typename T::value_type>();
            static Schema const schema{
                .name = "vector"_zsv,
                .primitive = SchemaPrimitive::Array,
                .elementType = &elementSchema};
            return schema;
        }
        else {
            return SchemaHolder<T>::get();
        }
    }
} // namespace up::reflex
