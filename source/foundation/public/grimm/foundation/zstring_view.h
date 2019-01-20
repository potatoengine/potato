// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "string_view.h"
#include <string_view>
#include <iosfwd>

namespace gm {
    class zstring_view {
    public:
        using value_type = char;
        using pointer = char const*;
        using size_type = std::size_t;
        using const_iterator = pointer;

        using traits = std::char_traits<value_type>;

        struct const_sentinel {
            friend constexpr bool operator==(const_iterator iter, const_sentinel) noexcept { return iter != nullptr && *iter != 0; }
            friend constexpr bool operator<(const_iterator iter, const_sentinel) noexcept { return iter == nullptr || *iter != 0; }
        };

        static constexpr size_type npos = ~size_type(0);

        constexpr zstring_view() = default;
        constexpr zstring_view(pointer str) noexcept : _str(str) {}
        constexpr zstring_view(std::nullptr_t) noexcept {}

        constexpr explicit operator bool() const noexcept { return _str != nullptr && *_str != 0; }
        constexpr bool empty() const noexcept { return _str == nullptr || *_str == 0; }

        constexpr size_type size() const noexcept { return _str != nullptr ? traits::length(_str) : 0; }

        constexpr pointer data() const noexcept { return _str; }
        constexpr pointer c_str() const noexcept { return _str; }

        constexpr /*implicit*/ operator string_view() const noexcept { return string_view{_str}; }
        constexpr /*implicit*/ operator std::string_view() const noexcept { return std::string_view{_str}; }

        constexpr value_type operator[](size_type index) const noexcept { return _str[index]; }

        constexpr zstring_view substr(size_type index) const noexcept { return _str + index; }

        constexpr string_view substr(size_type index, size_type count) const noexcept { return {_str + index, count}; }

        constexpr const_iterator begin() const noexcept { return _str; }
        constexpr const_sentinel end() const noexcept { return const_sentinel{}; }

        constexpr value_type front() const noexcept { return *_str; }

        constexpr string_view first(size_type count) const noexcept { return string_view{_str, count}; }

        constexpr size_type find(value_type ch) const noexcept {
            if (_str != nullptr) {
                for (pointer p = _str; *p != 0; ++p) {
                    if (*p == ch) {
                        return p - _str;
                    }
                }
            }
            return npos;
        }

        constexpr size_type find_first_of(string_view chars) const noexcept {
            if (_str != nullptr) {
                for (pointer p = _str; *p != 0; ++p) {
                    if (chars.find(*p) != string_view::npos) {
                        return p - _str;
                    }
                }
            }
            return npos;
        }

        constexpr size_type find_last_of(string_view chars) const noexcept {
            if (_str != nullptr) {
                for (pointer p = _str + size(); p != _str; --p) {
                    if (chars.find(*(p - 1)) != string_view::npos) {
                        return p - _str - 1;
                    }
                }
            }
            return npos;
        }

        template <typename T>
        friend auto& operator<<(std::basic_ostream<value_type, T>& os, zstring_view sv) {
            if (sv._str != nullptr) {
                os << sv._str;
            }
            return os;
        }

        friend constexpr bool operator==(zstring_view lhs, zstring_view rhs) noexcept {
            size_type lhsSize = lhs.size();
            size_type rhsSize = rhs.size();
            return lhsSize == rhsSize && traits::compare(lhs._str, rhs._str, lhsSize) == 0;
        }
        friend constexpr bool operator==(zstring_view lhs, pointer rhs) noexcept {
            size_type lhsSize = lhs.size();
            size_type rhsSize = rhs != nullptr ? traits::length(rhs) : 0;
            return lhsSize == rhsSize && traits::compare(lhs._str, rhs, lhsSize) == 0;
        }
        friend constexpr bool operator==(pointer lhs, zstring_view rhs) noexcept { return rhs == lhs; }

    private:
        pointer _str = nullptr;
    };

    template <typename HashAlgorithm>
    void hash_append(HashAlgorithm& hasher, zstring_view string) {
        hasher(span<char const>(string.data(), string.size()).as_bytes());
    }
} // namespace gm
