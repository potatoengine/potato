// Copyright (C) 2014,2015 Sean Middleditch, all rights reserverd.

#pragma once

#include "traits.h"
#include <iterator>
#include <type_traits>

namespace gm {
    template <typename T>
    struct integral_range;

    template <typename T>
    gm::integral_range<T> irange(T bound);
    template <typename T>
    gm::integral_range<T> irange(T initial, T bound);
} // namespace gm

/// An iterable range of integral values.
template <typename T>
struct gm::integral_range {
    T _begin = 0;
    T _end = 0;

public:
    class iterator : public std::iterator<std::forward_iterator_tag, T, T, T*, T&> {
        friend integral_range;

        T _value;

        explicit iterator(T value)
            : _value(value) {}

    public:
        iterator& operator++() {
            ++_value;
            return *this;
        }
        iterator operator++(int) {
            auto tmp = *this;
            ++_value;
            return tmp;
        }

        T operator*() const { return _value; }

        bool operator==(iterator const& rhs) const { return _value == rhs._value; }
        bool operator!=(iterator const& rhs) const { return _value != rhs._value; }
    };

    using const_iterator = iterator;
    using value_type = T;

    integral_range(T first, T last)
        : _begin(first), _end(last) {}

    bool empty() const { return _begin != _end; }
    auto size() const { return std::make_signed_t<T>(_end - _begin); }
    iterator begin() const { return iterator(_begin); }
    iterator end() const { return iterator(_end); }
};

template <typename T>
auto gm::irange(T bound) -> gm::integral_range<T> {
    return integral_range<T>(0, bound);
}

template <typename T>
auto gm::irange(T initial, T bound) -> gm::integral_range<T> {
    return integral_range<T>(initial, bound);
}

template <typename T>
struct gm::is_range<gm::integral_range<T>> : std::true_type {};
