// Copyright (C) 2018,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include <utility>

namespace gm {
    template <typename T, auto D, auto Default = T{}>
    class unique_resource;
}

template <typename T, auto D, auto Default>
class gm::unique_resource {
public:
    using value_type = T;
    using reference = T&;
    using const_reference = T const&;
    using rvalue_reference = T&&;

    unique_resource() = default;
    ~unique_resource() { reset(); }

    explicit unique_resource(rvalue_reference obj) : _object(obj) {}

    unique_resource(unique_resource&& src) : _object(std::move(src.get())) {}
    inline unique_resource& operator=(unique_resource&& src);

    inline unique_resource& operator=(rvalue_reference obj);

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
auto gm::unique_resource<T, D, Default>::operator=(unique_resource&& src) -> gm::unique_resource<T, D, Default>& {
    if (this != &src) {
        reset(std::move(src.get()));
    }
    return *this;
}

template <typename T, auto D, auto Default>
auto gm::unique_resource<T, D, Default>::operator=(rvalue_reference obj) -> gm::unique_resource<T, D, Default>& {
    if (obj != _object) {
        D(_object);
        _object = std::move(obj);
    }
    return *this;
}

template <typename T, auto D, auto Default>
void gm::unique_resource<T, D, Default>::reset(rvalue_reference obj) {
    if (obj != _object) {
        D(_object);
        _object = std::forward<T>(obj);
    }
}

template <typename T, auto D, auto Default>
void gm::unique_resource<T, D, Default>::reset() {
    D(_object);
    _object = Default;
}
