// Copyright (C) 2015,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_assertion.h"
#include "int_types.h"
#include <cstring>

namespace up {
    class bit_set {
    public:
        using index_type = size_t;
        using size_type = size_t;

        bit_set() = default;
        inline explicit bit_set(size_type capacity);
        inline ~bit_set();

        bit_set(bit_set const&) = delete;
        bit_set& operator=(bit_set const&) = delete;

        inline bit_set(bit_set&& rhs) noexcept;
        inline bit_set& operator=(bit_set&&) noexcept;

        inline bool test(index_type index) const noexcept;
        inline bit_set& set(index_type index);
        inline bit_set& reset(index_type index) noexcept;

        inline bool operator==(bit_set const& rhs) const noexcept;

        inline bit_set& resize(size_type capacity);

    private:
        using element = uint64;
        static constexpr size_t bits = sizeof(element) * 8;

        element* _elems = nullptr;
        size_type _capacity = 0;
    };

    bit_set::bit_set(size_type capacity) {
        resize(capacity);
    }

    bit_set::~bit_set() {
        delete[] _elems;
    }

    bit_set::bit_set(bit_set&& rhs) noexcept : _elems(rhs._elems), _capacity(rhs._capacity) {
        rhs._elems = nullptr;
        rhs._capacity = 0;
    }

    bit_set& bit_set::operator=(bit_set&& rhs) noexcept {
        UP_SPUD_ASSERT(this != &rhs, "cannot move a bit_set over itself");
        _elems = rhs._elems;
        _capacity = rhs._capacity;
        rhs._elems = nullptr;
        rhs._capacity = 0;
    }

    bool bit_set::test(index_type index) const noexcept {
        if (index * bits >= _capacity) {
            return false;
        }
        size_t elem = index / bits;
        size_t bit = index - (elem * bits);
        element mask = element(1) << bit;
        element& data = _elems[elem];
        return (data & mask) != 0;   
    }

    bit_set& bit_set::set(index_type index) {
        if (index * bits >= _capacity) {
            resize(index * bits + 1);
        }
        size_t elem = index / bits;
        size_t bit = index - (elem * bits);
        element mask = element(1) << bit;
        element& data = _elems[elem];
        data |= mask;
        return *this;
    }

    bit_set& bit_set::reset(index_type index) noexcept {
        if (index * bits >= _capacity) {
            return *this;
        }
        size_t elem = index / bits;
        size_t bit = index - (elem * bits);
        element mask = element(1) << bit;
        element& data = _elems[elem];
        data &= ~mask;
        return *this;
    }

    bool bit_set::operator==(bit_set const& rhs) const noexcept {
        size_type common = _capacity > rhs._capacity ? rhs._capacity : _capacity;
        for (size_type index = 0; index != common; ++index) {
            element& left = _elems[index];
            element& right = rhs._elems[index];
            if (left != right) {
                return false;
            }
        }

        bit_set const& remainder = _capacity > rhs._capacity ? *this : rhs;
        for (size_t index = common; index != remainder._capacity; ++index) {
            element& el = remainder._elems[index];
            if (el != element(0)) {
                return false;
            }
        }

        return true;
    }

    bit_set& bit_set::resize(size_type capacity) {
        if (capacity >= _capacity) {
            element* elems = new element[capacity];

            std::memcpy(elems, _elems, _capacity * sizeof(element));

            std::memset(elems + _capacity, 0, (capacity - _capacity) * sizeof(element));

            delete[] _elems;

            _elems = elems;
            _capacity = capacity;
        }
        return *this;
    }
} // namespace up
