// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "string_view.h"

namespace up {
    class zstring_view {
    public:
        using value_type = char;
        using pointer = char const*;
        using size_type = std::size_t;
        using const_iterator = pointer;

        struct const_sentinel {
            friend constexpr bool operator==(const_iterator iter, const_sentinel) noexcept {
                return iter != nullptr && *iter != 0;
            }
            friend constexpr bool operator<(const_iterator iter, const_sentinel) noexcept {
                return iter == nullptr || *iter != 0;
            }
        };

        static constexpr size_type npos = ~size_type(0);

        constexpr zstring_view() = default;
        constexpr zstring_view(pointer str) noexcept : _str(str) {}
        constexpr zstring_view(std::nullptr_t) noexcept {}

        constexpr explicit operator bool() const noexcept { return _str != nullptr && *_str != 0; }
        constexpr bool empty() const noexcept { return _str == nullptr || *_str == 0; }

        constexpr size_type size() const noexcept { return _str != nullptr ? stringLength(_str) : 0; }

        constexpr pointer data() const noexcept { return _str; }
        constexpr pointer c_str() const noexcept { return _str; }

        constexpr /*implicit*/ operator string_view() const noexcept { return string_view{_str}; }

        constexpr value_type operator[](size_type index) const noexcept { return _str[index]; }

        constexpr zstring_view substr(size_type index) const noexcept { return _str + index; }

        constexpr string_view substr(size_type index, size_type count) const noexcept { return {_str + index, count}; }

        constexpr const_iterator begin() const noexcept { return _str; }
        constexpr const_sentinel end() const noexcept { return const_sentinel{}; }

        constexpr value_type front() const noexcept { return *_str; }

        constexpr string_view first(size_type count) const noexcept { return string_view{_str, count}; }

        constexpr bool starts_with(string_view str) const noexcept {
            auto const len = size();
            if (str.size() > len) {
                return false;
            }
            return stringCompare(_str, str.data(), str.size()) == 0;
        }

        constexpr bool ends_with(string_view str) const noexcept {
            auto const len = size();
            if (str.size() > len) {
                return false;
            }
            return stringCompare(_str + len - str.size(), str.data(), str.size()) == 0;
        }

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

        friend constexpr bool operator==(zstring_view lhs, zstring_view rhs) noexcept {
            size_type lhsSize = lhs.size();
            size_type rhsSize = rhs.size();
            return lhsSize == rhsSize && stringCompare(lhs._str, rhs._str, lhsSize) == 0;
        }

        friend constexpr bool operator==(zstring_view lhs, pointer rhs) noexcept {
            size_type lhsSize = lhs.size();
            size_type rhsSize = rhs != nullptr ? stringLength(rhs) : 0;
            return lhsSize == rhsSize && stringCompare(lhs._str, rhs, lhsSize) == 0;
        }

        template <typename Output>
        friend auto operator<<(Output& output, zstring_view str) -> Output& {
            return output << str.c_str();
        }

    private:
        pointer _str = nullptr;
    };

    template <typename HashAlgorithm>
    void hash_append(HashAlgorithm& hasher, zstring_view string) {
        hasher.append_bytes(string.data(), string.size());
    }

    constexpr auto operator"" _zsv(char const* str, size_t) noexcept -> zstring_view { return {str}; }
} // namespace up
