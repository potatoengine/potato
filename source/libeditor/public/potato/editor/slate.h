// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/spud/rc.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"

#include <unordered_map>

namespace up {
    class SlateDocument;
    class SlateObject;
    class SlatePath;
    class SlateItem;
    class SlateRef;
    class SlateEdit;

    class SlateDocument {
    public:
        SlateDocument() = default;
        ~SlateDocument() = default;

        SlateDocument(SlateDocument const&) = delete;
        SlateDocument& operator=(SlateDocument const&) = delete;

        SlateObject const& root() const noexcept { return *_root; }

    private:
        rc<SlateObject> _root = new_shared<SlateObject>();
    };

    class SlateItem {
    public:
        enum class Type { Undefined, Null, Int32, Float32, Bool };

        constexpr bool isUndefined() const noexcept { return _type == Type::Undefined; }

        constexpr int asInt32() const noexcept {
            switch (_type) {
                case Type::Int32:
                    return _data.i32;
                case Type::Float32:
                    return static_cast<int>(_data.f32);
                case Type::Bool:
                    return _data.b ? 1 : 0;
                default:
                    return 0;
            }
        }

        constexpr float asFloat32() const noexcept {
            switch (_type) {
                case Type::Float32:
                    return _data.f32;
                case Type::Int32:
                    return static_cast<float>(_data.i32);
                case Type::Bool:
                    return _data.b ? 1.f : 0.f;
                default:
                    return 0.f;
            }
        }

        constexpr bool asBool() const noexcept {
            switch (_type) {
                case Type::Bool:
                    return _data.b;
                case Type::Float32:
                    return _data.f32 != 0.f;
                case Type::Int32:
                    return _data.i32 != 0;
                default:
                    return false;
            }
        }

    private:
        union Data {
            int i32 = 0;
            float f32;
            bool b;
        } _data;
        Type _type = Type::Undefined;
    };

    class SlateObject : public shared<SlateObject> {
    public:
        enum class Kind { Map, Array };

        SlateObject() = default;

        SlateObject(SlateDocument const&) = delete;
        SlateObject& operator=(SlateDocument const&) = delete;

        bool isEmpty() const noexcept { return true; }
        bool isMap() const noexcept { return _kind == Kind::Map; }
        bool isArray() const noexcept { return _kind == Kind::Array; }

        size_t size() const noexcept { return _kind == Kind::Map ? _map.size() : _array.size(); }

    private:
        Kind _kind = Kind::Map;

        std::unordered_map<string, SlateItem> _map;
        vector<SlateItem> _array;
    };
} // namespace up
