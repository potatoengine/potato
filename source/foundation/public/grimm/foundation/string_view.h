// Copyright (C) 2014,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "numeric_util.h"
#include "string_util.h"

namespace gm {
    class string_view;

    template <typename HashAlgorithm>
    inline void hash_append(HashAlgorithm& hasher, string_view const& string);

    inline string_view operator"" _sv(char const* str, size_t size) noexcept;
} // namespace gm

class gm::string_view {
public:
    using value_type = char;
    using iterator = char const*;
    using const_iterator = char const*;
    using pointer = char const*;
    using reference = char const&;
    using size_type = size_t;

    static constexpr size_type npos = ~size_type{0};

    constexpr string_view() = default;
    ~string_view() = default;

    constexpr string_view(string_view const&) = default;
    /*implicit*/ constexpr string_view(pointer zstr) noexcept : _data(zstr), _size(zstr != nullptr ? stringLength(zstr) : 0) {}
    /*implicit*/ constexpr string_view(pointer data, size_type size) noexcept : _data(data), _size(size) {}

    constexpr string_view& operator=(string_view const&) = default;
    constexpr string_view& operator=(pointer zstr) noexcept {
        _data = zstr;
        _size = zstr != nullptr ? stringLength(zstr) : 0;
        return *this;
    }

    constexpr pointer data() const noexcept {
        return _data;
    }
    constexpr size_type size() const noexcept {
        return _size;
    }

    constexpr bool empty() const noexcept {
        return _size == 0;
    }
    constexpr explicit operator bool() const noexcept {
        return _size != 0;
    }

    constexpr const_iterator begin() const noexcept {
        return _data;
    }
    constexpr const_iterator end() const noexcept {
        return _data + _size;
    }

    constexpr value_type front() const noexcept {
        return *_data;
    }
    constexpr value_type back() const noexcept {
        return *(_data + _size - 1);
    }

    constexpr value_type operator[](size_type index) const noexcept {
        return _data[index];
    }

    constexpr void pop_front() noexcept {
        ++_data;
        --_size;
    }
    constexpr void pop_back() noexcept {
        --_size;
    }

    constexpr void remove_prefix(size_type count) noexcept {
        _data += count;
        _size -= count;
    }
    constexpr void remove_suffix(size_type count) noexcept {
        _size -= count;
    }

    constexpr string_view first(size_type count) const noexcept {
        return {_data, count};
    }
    constexpr string_view last(size_type count) const noexcept {
        return {_data + _size - count, count};
    }

    constexpr string_view substr(size_type offset, size_type count = npos) const noexcept {
        if (offset > _size) {
            offset = _size;
        }
        auto remaining = _size - offset;
        if (count > remaining) {
            count = remaining;
        }
        return {_data + offset, count};
    }

    constexpr bool starts_with(string_view str) const noexcept {
        if (str.size() > _size) {
            return false;
        }
        return stringCompare(_data, str.data(), str.size()) == 0;
    }
    constexpr bool ends_with(string_view str) const noexcept {
        if (str.size() > _size) {
            return false;
        }
        return stringCompare(_data + _size - str.size(), str.data(), str.size()) == 0;
    }

    constexpr size_type find(value_type ch) const noexcept {
        auto iter = stringFindChar(_data, _size, ch);
        return iter != nullptr ? iter - _data : npos;
    }

    constexpr size_type find_first_of(string_view chars) const noexcept {
        for (size_type i = 0; i != _size; ++i) {
            if (stringFindChar(chars._data, chars._size, _data[i]) != nullptr) {
                return i;
            }
        }
        return npos;
    }

    constexpr size_type find_last_of(string_view chars) const noexcept {
        for (size_type i = _size; i != 0; --i) {
            if (stringFindChar(chars._data, chars._size, _data[i - 1]) != nullptr) {
                return i - 1;
            }
        }
        return npos;
    }

    friend bool constexpr operator==(string_view lhs, string_view rhs) noexcept {
        return lhs.size() == rhs.size() && stringCompare(lhs.data(), rhs.data(), lhs.size()) == 0;
    }
    friend bool constexpr operator!=(string_view lhs, string_view rhs) noexcept {
        return lhs.size() != rhs.size() || stringCompare(lhs.data(), rhs.data(), lhs.size()) != 0;
    }
    friend bool constexpr operator<(string_view lhs, string_view rhs) noexcept {
        auto len = lhs.size() < rhs.size() ? lhs.size() : rhs.size();
        auto rs = stringCompare(lhs.data(), rhs.data(), len);
        if (rs < 0) {
            return true;
        }
        else if (rs == 0 || lhs.size() < rhs.size()) {
            return true;
        }
        return false;
    }

private:
    pointer _data = nullptr;
    size_type _size = 0;
};

template <typename HashAlgorithm>
void gm::hash_append(HashAlgorithm& hasher, string_view const& string) {
    hasher(string.data(), string.size());
}

inline auto gm::operator"" _sv(char const* str, size_t size) noexcept -> string_view {
    return {str, size};
}
