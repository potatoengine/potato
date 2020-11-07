// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/reflex/schema.h"

#include <nlohmann/json_fwd.hpp>

namespace up {
    class JsonDeserializer {
    public:
        JsonDeserializer(nlohmann::json const& json) : _json(json) {}

        UP_TOOLS_API bool deserializeRaw(reflex::Schema const& schema, void* target);

        template <reflex::has_schema T>
        bool deserialize(T& target) {
            return deserializeRaw(reflex::getSchema<T>(), &target);
        }

    private:
        nlohmann::json const& _json;
    };

    class JsonSerializer {
    public:
        JsonSerializer(nlohmann::json& json) : _json(json) {}

        UP_TOOLS_API bool serializeRaw(reflex::Schema const& schema, void const* target);

        template <reflex::has_schema T>
        bool serialize(T const& target) {
            return serializeRaw(reflex::getSchema<T>(), &target);
        }

    private:
        bool serializeObject(reflex::Schema const& schema, void const* target);

        nlohmann::json& _json;
    };
} // namespace up
