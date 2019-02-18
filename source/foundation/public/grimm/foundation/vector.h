// Copyright (C) 2015 Sean Middleditch, all rights reserverd.

#pragma once

#include "span.h"
#include "assertion.h"
#include "iterator_range.h"
#include "memory_util.h"
#include "numeric_util.h"
#include "traits.h"

#include <initializer_list>
#include <type_traits>

namespace gm {
    template <typename T>
    class vector;

    template <typename T>
    vector(std::initializer_list<T>)->vector<T>;
    template <typename IteratorT, typename SentinelT>
    vector(IteratorT, SentinelT)->vector<decltype(*std::declval<IteratorT>())>;
} // namespace gm

template <typename T>
class gm::vector {
    T* _first = nullptr;
    T* _last = nullptr;
    T* _sentinel = nullptr;

    size_t _grow(size_t minimum = 4);
    void _rshift(T* pos, size_t shift);

public:
    using value_type = T;
    using iterator = T*;
    using const_iterator = T const*;
    using range = iterator_range<iterator>;
    using const_range = iterator_range<const_iterator>;
    using pointer = T*;
    using const_pointer = T const*;
    using reference = T&;
    using const_reference = T const&;
    using rvalue_reference = T&&;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    vector() = default;

    template <typename IteratorT, typename SentinelT>
    inline explicit vector(IteratorT begin, SentinelT end);
    inline explicit vector(std::initializer_list<T> initial);
    inline explicit vector(size_type size, const_reference initial);
    inline explicit vector(size_type size);

    vector(vector const&) = delete;
    vector& operator=(vector const&) = delete;

    inline vector(vector&& src);
    inline vector& operator=(vector&& src);

    inline ~vector();

    static vector acquire(T* memory, size_type count);

    iterator begin() { return _first; }
    iterator end() { return _last; }

    const_iterator begin() const { return _first; }
    const_iterator end() const { return _last; }

    const_iterator cbegin() const { return _first; }
    const_iterator cend() const { return _last; }

    pointer data() { return _first; }
    const_pointer data() const { return _first; }

    pointer release();

    bool empty() const { return _first == _last; }
    size_type size() const { return _last - _first; }
    size_type capacity() const { return _sentinel - _first; }

    reference operator[](size_type index) { return _first[index]; }
    const_reference operator[](size_type index) const { return _first[index]; }

    reference front() { return *_first; }
    const_reference front() const { return *_first; }

    reference back() { return *(_last - 1); }
    const_reference back() const { return *(_last - 1); }

    void reserve(size_type required);
    void resize(size_type new_size);
    void resize(size_type new_size, const_reference init);
    void clear();
    void shrink_to_fit();

    template <typename... ParamsT>
    auto emplace(const_iterator pos, ParamsT&&... params) -> enable_if_t<std::is_constructible<T, ParamsT...>::value, iterator>;
    template <typename... ParamsT>
    auto emplace_back(ParamsT&&... params) -> enable_if_t<std::is_constructible<T, ParamsT...>::value, iterator>;

    iterator insert(iterator pos, const_reference value) { return emplace(pos, value); }
    iterator insert(iterator pos, rvalue_reference value) { return emplace(pos, std::move(value)); }
    template <typename InsertT>
    iterator insert(const_iterator pos, InsertT&& value) { return emplace(pos, std::forward<InsertT>(value)); }

    template <typename IteratorT, typename SentinelT>
    iterator insert(const_iterator pos, IteratorT begin, SentinelT end);

    iterator push_back(const_reference value) { return emplace_back(value); }
    iterator push_back(rvalue_reference value) { return emplace_back(std::move(value)); }
    template <typename InsertT>
    iterator push_back(InsertT&& value) { return emplace_back(std::forward<InsertT>(value)); }

    iterator erase(const_iterator pos);
    iterator erase(const_iterator begin, const_iterator end);

    void pop_back();

    operator span<T>() { return span<T>(_first, _last); }
    operator span<T const>() const { return span<T const>(_first, _last); }
};

template <typename T>
template <typename IteratorT, typename SentinelT>
gm::vector<T>::vector(IteratorT begin, SentinelT end) {
    insert(_first, begin, end);
}

template <typename T>
gm::vector<T>::vector(std::initializer_list<T> initial) {
    insert(_first, initial.begin(), initial.end());
}

template <typename T>
gm::vector<T>::vector(size_type size, const_reference initial) {
    resize(size, initial);
}

template <typename T>
gm::vector<T>::vector(size_type size) {
    resize(size);
}

template <typename T>
gm::vector<T>::vector(vector&& src) : _first(src._first), _last(src._last), _sentinel(src._sentinel) {
    src._sentinel = src._last = src._first = nullptr;
}

template <typename T>
gm::vector<T>::~vector() {
    clear();
    shrink_to_fit();
}

template <typename T>
auto gm::vector<T>::operator=(vector&& src) -> vector& {
    if (this != &src) {
        clear();
        shrink_to_fit();

        _first = src._first;
        _last = src._last;
        _sentinel = src._sentinel;

        src._sentinel = src._last = src._first = nullptr;
    }
    return *this;
}

template <typename T>
auto gm::vector<T>::acquire(T* memory, size_type count) -> vector {
    vector rs;
    rs._first = memory;
    rs._last = rs._sentinel = count;
    return rs;
}

template <typename T>
T* gm::vector<T>::release() {
    GM_ASSERT(_last == _sentinel, "Releasing memory from a vector that has uninitialized capacity; call resize(capacity()) first!");
    T* tmp = _first;
    _first = _last = _sentinel = nullptr;
    return tmp;
}

template <typename T>
size_t gm::vector<T>::_grow(size_t minimum) {
    size_type capacity = _sentinel - _first;
    capacity += capacity >> 1;

    return gm::max(minimum, capacity); // grow by 50%
}

template <typename T>
void gm::vector<T>::_rshift(T* pos, size_t shift) {
    size_t size = _last - pos;

    // copy elements to the new area, as needed
    auto const tail = gm::min(size, shift);
    gm::unitialized_move_n(_last - tail, tail, _last + shift - tail);

    _last += shift;

    // shift elements inside the already-initialized parts of the vector
    auto const head = size - tail;
    gm::move_backwards_n(pos, head, pos + head);
}

template <typename T>
void gm::vector<T>::reserve(size_type required) {
    size_type const capacity = _sentinel - _first;
    if (capacity < required) {
        T* tmp = new T[required];
        auto const count = _last - _first;
        gm::unitialized_move_n(_first, count, tmp);
        gm::destruct_n(_first, count);
        delete[] _first;
        _first = tmp;
        _last = _first + count;
        _sentinel = _first + required;
    }
}

template <typename T>
void gm::vector<T>::resize(size_type new_size) {
    size_type const size = _last - _first;
    if (size < new_size) {
        reserve(new_size);
        for (T* new_end = _first + new_size; _last != new_end; ++_last)
            new (_last) T{};
    }
    else if (size > new_size) {
        gm::destruct_n(_first + new_size, _last - _first);
        _last = _first + new_size;
    }
}

template <typename T>
void gm::vector<T>::resize(size_type new_size, const_reference init) {
    size_type const count = _last - _first;
    if (count < new_size) {
        reserve(new_size);
        for (T* new_end = _first + new_size; _last != new_end; ++_last)
            new (_last) T(init);
    }
    else if (count > new_size) {
        gm::destruct_n(_first + new_size, _last - _first);
        _last = _first + new_size;
    }
}

template <typename T>
void gm::vector<T>::clear() {
    gm::destruct_n(_first, _last - _first);
    _last = _first;
}

template <typename T>
void gm::vector<T>::shrink_to_fit() {
    if (_sentinel == nullptr) { /* do nothing */
    }
    else if (_first == _last) {
        delete[] _first;
        _first = _last = _sentinel = nullptr;
    }
    else if (_sentinel > _last) {
        auto const size = _last - _first;
        T* tmp = new T[size];
        gm::unitialized_move_n(_first, size, tmp);
        gm::destruct_n(_first, size);
        delete[] _first;
        _first = tmp;
        _sentinel = _last = _first + size;
    }
}

template <typename T>
template <typename... ParamsT>
auto gm::vector<T>::emplace(const_iterator pos, ParamsT&&... params) -> gm::enable_if_t<std::is_constructible<T, ParamsT...>::value, iterator> {
    if (_last == _sentinel) {
        auto const offset = pos - _first;

        // grow
        auto const newCapacity = _grow(_last - _first + 1);
        T* tmp = new T[newCapacity];

        // insert new elements
        new (tmp + offset) value_type(std::forward<ParamsT>(params)...);

        // move over elements before insertion point
        gm::unitialized_move_n(_first, offset, tmp);

        // move over elements after insertion point
        gm::unitialized_move_n(_first + offset, _last - _first - offset, tmp + offset + 1);

        auto const new_size = _last - _first + 1;

        // free up old space
        delete[] _first;

        // commit new space
        _first = tmp;
        _last = _first + new_size;
        _sentinel = _first + newCapacity;

        return _first + offset;
    }
    else if (pos == _last) {
        return new (_last++) value_type(std::forward<ParamsT>(params)...);
    }
    else {
        iterator mpos = const_cast<iterator>(pos);
        _rshift(mpos, 1);
        *mpos = value_type(std::forward<ParamsT>(params)...);
        return mpos;
    }
}

template <typename T>
template <typename... ParamsT>
auto gm::vector<T>::emplace_back(ParamsT&&... params) -> gm::enable_if_t<std::is_constructible<T, ParamsT...>::value, iterator> {
    if (_last == _sentinel) {
        auto const size = _last - _first;

        // grow
        auto const new_capacity = _grow(size + 1);
        T* tmp = new T[new_capacity];

        // insert new elements
        new (tmp + size) value_type(std::forward<ParamsT>(params)...);

        // move over old elements
        gm::unitialized_move_n(_first, size, tmp);

        // free up old space
        delete[] _first;

        // commit new space
        _first = tmp;
        _last = _first + size + 1;
        _sentinel = _first + new_capacity;

        return _first + size;
    }
    else {
        return new (_last++) value_type(std::forward<ParamsT>(params)...);
    }
}

template <typename T>
template <typename IteratorT, typename SentinelT>
auto gm::vector<T>::insert(const_iterator pos, IteratorT begin, SentinelT end) -> iterator {
    GM_ASSERT(begin < _first || begin >= _last, "Inserting a sub-range of a vector into itself is not supported");

    auto const count = std::distance(begin, end);

    if (_sentinel - _last < count) {
        auto const offset = pos - _first;

        // grow
        auto const newCapacity = _grow(_last - _first + count);
        T* tmp = new T[newCapacity];

        // insert new elements
        gm::unitialized_copy_n(begin, count, tmp + offset);

        // move over elements before insertion point
        gm::unitialized_copy_n(_first, offset, tmp);

        // move over elements after insertion point
        gm::unitialized_copy_n(_first + offset, _last - _first - offset, tmp + offset + count);

        auto const new_size = _last - _first + count;

        // free up old space
        delete[] _first;

        // commit new space
        _first = tmp;
        _last = _first + new_size;
        _sentinel = _first + newCapacity;

        return _first + offset;
    }
    else {
        iterator mpos = const_cast<iterator>(pos);
        _rshift(mpos, count);
        gm::move_n(begin, count, mpos);
        return mpos;
    }
}

template <typename T>
auto gm::vector<T>::erase(const_iterator pos) -> iterator {
    iterator mpos = const_cast<iterator>(pos);
    gm::move_n(mpos + 1, _last - mpos - 1, mpos);
    pop_back();
    return mpos;
}

template <typename T>
auto gm::vector<T>::erase(const_iterator begin, const_iterator end) -> iterator {
    iterator mbegin = const_cast<iterator>(begin);
    auto const count = end - begin;
    gm::move_n(mbegin + count, _last - begin - count, mbegin);
    gm::destruct_n(_last - count, count);
    _last -= count;
    return mbegin;
}

template <typename T>
void gm::vector<T>::pop_back() {
    (--_last)->~value_type();
}
