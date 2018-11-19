// Copyright (C) 2015 Sean Middleditch, all rights reserverd.

#pragma once

#include "stdfwd.h"
#include "traits.h"
#include <initializer_list>
#include <type_traits>

namespace gm
{
    template <typename T, typename A>
    class vector;

    template <typename T>
    struct array_view;

    template <typename HashAlgorithm, typename T>
    inline void hash_append(HashAlgorithm&, gm::array_view<T> const&);
} // namespace gm

/// <summary> A non-owning slice of an array. </summary>
/// <typeparam name="T"> Type of the elements in the array. </typeparam>
template <typename T>
struct gm::array_view
{
public:
    using iterator = T * ;
    using sentinel = T * ;
    using reference = T & ;
    using size_type = std::size_t;

    array_view() = default; // #FIXME: figure out why this is needed even though I've inherited constructors
    /*implicit*/ template <typename U>
    array_view(array_view<U> src) : _begin(src.begin()), _end(src.end())
    {
    }
    /*implicit*/ template <std::size_t N>
    array_view(T(&src)[N]) : _begin(src), _end(src + N)
    {
    }
    /*implicit*/ array_view(T* begin, T* end) : _begin(begin), _end(end) {}
    /*implicit*/ array_view(std::initializer_list<T> src) : _begin(src.begin()), _end(src.end()) {}
    explicit array_view(T* ptr, std::size_t size) : _begin(ptr), _end(ptr + size) {}

    iterator begin() const { return _begin; }
    sentinel end() const { return _end; }

    T* data() const { return _begin; }

    bool empty() const { return _begin == _end; }
    explicit operator bool() const { return _begin != _end; }

    reference operator[](size_type index) const { return _begin[index]; }

    size_type size() const { return static_cast<size_type>(_end - _begin); }

    reference front() const { return *_begin; }
    reference back() const { return *(_end - 1); }

    void pop_front() { ++_begin; }
    void pop_back() { --_end; }

private:
    T* _begin = nullptr;
    T* _end = nullptr;
};

template <typename HashAlgorithm, typename T>
void gm::hash_append(HashAlgorithm& hasher, gm::array_view<T> const& view)
{
    if constexpr (gm::is_contiguous_v<T>)
    {
        hasher(view.data(), view.size() * sizeof(T));
    }
    else
    {
        for (auto&& value : view)
        {
            hash_append(hasher, value);
        }
    }
}
