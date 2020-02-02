// Copyright (C) 2018,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include <utility>
#include <type_traits>

namespace up {
    template <typename T, auto D, auto Default = T{}>
    class unique_resource;
}

template <typename T, auto D, auto Default>
class up::unique_resource {
public:
    using value_type = T;
    using reference = T&;
    using const_reference = T const&;
    using rvalue_reference = T&&;

    unique_resource() = default;
    ~unique_resource() { reset(); }

    explicit unique_resource(rvalue_reference obj) noexcept(std::is_nothrow_move_constructible_v<T>) : _object(obj) {}

    unique_resource(unique_resource&& src) noexcept(std::is_nothrow_move_constructible_v<T>) : _object(std::move(src.get())) {}

    inline unique_resource& operator=(unique_resource&& src) noexcept(std::is_nothrow_move_assignable_v<T>);
    inline unique_resource& operator=(rvalue_reference obj) noexcept(std::is_nothrow_move_assignable_v<T>);

    bool empty() const { return _object == Default; }
    explicit operator bool() const { return _object != Default; }

    friend bool operator==(unique_resource const& lhs, T const& rhs) { return lhs._object == rhs; }
    friend bool operator!=(unique_resource const& lhs, T const& rhs) { return lhs._object != rhs; }

    const_reference get() const { return _object; }
    reference get() { return _object; }

    inline void reset(rvalue_reference obj);
    inline void reset();

private:
    T _object = Default;
};

template <typename T, auto D, auto Default>
auto up::unique_resource<T, D, Default>::operator=(unique_resource&& src) noexcept(std::is_nothrow_move_assignable_v<T>) -> up::unique_resource<T, D, Default>& {
    reset(std::move(src.get()));
    return *this;
}

template <typename T, auto D, auto Default>
auto up::unique_resource<T, D, Default>::operator=(rvalue_reference obj) noexcept(std::is_nothrow_move_assignable_v<T>) -> up::unique_resource<T, D, Default>& {
    D(_object);
    _object = std::move(obj);
    return *this;
}

template <typename T, auto D, auto Default>
void up::unique_resource<T, D, Default>::reset(rvalue_reference obj) {
    D(_object);
    _object = std::move(obj);
}

template <typename T, auto D, auto Default>
void up::unique_resource<T, D, Default>::reset() {
    D(_object);
    _object = Default;
}
