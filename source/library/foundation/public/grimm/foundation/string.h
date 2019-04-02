// Copyright (C) 2014,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "string_view.h"
#include "zstring_view.h"
#include "string_util.h"
#include <cstring>

namespace gm {
    class string;

    template <typename HashAlgorithm>
    inline void hash_append(HashAlgorithm& hasher, string const& string);

    inline string operator"" _s(char const* str, size_t size);
} // namespace gm

class gm::string {
public:
    using value_type = char;
    using iterator = char const*;
    using const_iterator = char const*;
    using pointer = char*;
    using const_pointer = char const*;
    using reference = char const&;
    using size_type = std::size_t;

    static constexpr size_type npos = ~size_type{0};

    string() = default;
    ~string() { _free(_data, _size); }

    /*implicit*/ string(string const& str) : _data(_copy(str._data, str._size)), _size(str._size) {}
    string(string&& rhs) : _data(rhs._data), _size(rhs._size) {
        rhs._data = nullptr;
        rhs._size = 0;
    }
    /*implicit*/ string(const_pointer zstr) : _size(zstr != nullptr ? stringLength(zstr) : 0) { _data = _copy(zstr, _size); }
    /*implicit*/ string(const_pointer data, size_type size) : _data(_copy(data, size)), _size(size) {}
    /*implicit*/ string(zstring_view view) : _data(_copy(view.data(), view.size())), _size(view.size()) {}
    /*implicit*/ string(string_view view) : _data(_copy(view.data(), view.size())), _size(view.size()) {}

    static string take_ownership(pointer str, size_type length) {
        string s;
        s._data = str;
        s._size = length;
        return s;
    }

    string& operator=(string const&) = delete;

    string& operator=(string&& rhs) noexcept {
        if (this != &rhs) {
            _free(_data, _size);
            _size = rhs._size;
            _data = rhs.release();
        }
        return *this;
    }

    const_pointer c_str() const noexcept {
        return _data != nullptr ? _data : _empty;
    }

    const_pointer data() const noexcept {
        return _data != nullptr ? _data : _empty;
    }
    size_type size() const noexcept {
        return _size;
    }

    bool empty() const noexcept {
        return _size == 0;
    }
    explicit operator bool() const noexcept {
        return _size != 0;
    }

    const_iterator begin() const noexcept {
        return _data;
    }
    const_iterator end() const noexcept {
        return _data + _size;
    }

    value_type front() const noexcept {
        return *_data;
    }
    value_type back() const noexcept {
        return *(_data + _size - 1);
    }

    value_type operator[](size_type index) const noexcept {
        return _data[index];
    }

    string_view first(size_type count) const noexcept {
        return {_data, count};
    }
    string_view last(size_type count) const noexcept {
        return {_data + _size - count, count};
    }

    string_view substr(size_type offset, size_type count = npos) const noexcept {
        if (offset > _size) {
            offset = _size;
        }
        auto remaining = _size - offset;
        if (count > remaining) {
            count = remaining;
        }
        return {_data + offset, count};
    }

    bool starts_with(string_view str) const noexcept {
        if (str.size() > _size) {
            return false;
        }
        return stringCompare(_data, str.data(), str.size()) == 0;
    }
    bool ends_with(string_view str) const noexcept {
        if (str.size() > _size) {
            return false;
        }
        return stringCompare(_data + _size - str.size(), str.data(), str.size()) == 0;
    }

    size_type find(value_type ch) const noexcept {
        auto iter = stringFindChar(_data, _size, ch);
        return iter != nullptr ? iter - _data : npos;
    }

    size_type find_first_of(string_view chars) const noexcept {
        for (size_type i = 0; i != _size; ++i) {
            if (chars.find(_data[i]) != string_view::npos) {
                return i;
            }
        }
        return npos;
    }

    size_type find_last_of(string_view chars) const noexcept {
        for (size_type i = _size; i != 0; --i) {
            if (chars.find(_data[i - 1]) != string_view::npos) {
                return i - 1;
            }
        }
        return npos;
    }

    friend bool operator==(string const& lhs, string const& rhs) noexcept {
        return lhs.size() == rhs.size() && stringCompare(lhs.data(), rhs.data(), lhs.size()) == 0;
    }
    friend bool operator==(string const& lhs, const_pointer rhs) noexcept {
        auto rhsSize = rhs != nullptr ? stringLength(rhs) : 0;
        return lhs.size() == rhsSize && stringCompare(lhs.data(), rhs, rhsSize) == 0;
    }
    friend bool operator!=(string const& lhs, string const& rhs) noexcept {
        return lhs.size() != rhs.size() || stringCompare(lhs.data(), rhs.data(), lhs.size()) != 0;
    }
    friend bool operator<(string const& lhs, string const& rhs) noexcept {
        auto len = lhs.size() < rhs.size() ? lhs.size() : rhs.size();
        auto rs = stringCompare(lhs.data(), rhs.data(), len);
        if (rs < 0) {
            return true;
        }
        else if (rs == 0 && lhs.size() < rhs.size()) {
            return true;
        }
        return false;
    }

    /*implicit*/ operator string_view() const noexcept {
        return {_data, _size};
    }

    /*implicit*/ operator zstring_view() const noexcept {
        return {_data};
    }

    string& assign(const_pointer str, size_type length) {
        // RAII ensures self-assign of a range works
        string tmp;
        tmp.swap(*this);

        if (length != 0) {
            _data = _copy(str, length);
            _size = length;
        }

        return *this;
    }

    string& assign(const_pointer zstr) {
        // RAII ensures self-assign of a range works
        string tmp;
        tmp.swap(*this);

        if (zstr != nullptr) {
            assign(zstr, stringLength(zstr));
        }

        return *this;
    }

    void reset() {
        _free(_data, _size);
        _data = nullptr;
        _size = 0;
    }

    [[nodiscard]] pointer release() noexcept {
        pointer tmp = _data;
        _data = nullptr;
        _size = 0;
        return tmp;
    }

    string& swap(string& other) noexcept {
        pointer tmp = other._data;
        size_type tmpSize = other._size;
        other._data = _data;
        other._size = _size;
        _data = tmp;
        _size = tmpSize;
        return *this;
    }

private:
    [[nodiscard]] static pointer _copy(const_pointer str, size_type length) {
        if (length != 0) {
            pointer p = new value_type[length + 1];
            std::memmove(p, str, length);
            p[length] = 0;
            return p;
        }
        else {
            return nullptr;
        }
    }

    static void _free(pointer data, size_type length) {
        delete[] data;
        data = nullptr;
    }

    inline static value_type _empty[] = "";

    pointer _data = nullptr;
    size_type _size = 0;
};

template <typename HashAlgorithm>
void gm::hash_append(HashAlgorithm& hasher, string const& string) {
    hasher({reinterpret_cast<std::byte const*>(string.data()), string.size()});
}

inline auto gm::operator"" _s(char const* str, size_t size) -> string {
    return string{str, size};
}
