// Copyright (C) 2015 Sean Middleditch, all rights reserverd.

#pragma once

#include "memory.h"
#include "traits.h"

namespace gm {
    template <typename>
    class box;
    template <typename T, typename... Args>
    auto make_box(Args&&... args) -> enable_if_t<std::is_constructible_v<T, Args&&...>, box<T>>;

    namespace _detail {
        template <typename>
        struct box_traits;
    }
} // namespace gm

template <typename T>
struct gm::_detail::box_traits {
    using pointer = T*;
    using reference = T&;

    static void _deallocate(T* ptr) {
        delete ptr;
    }
};

/// <summary> An owning non-copyable pointer to a heap-allocated object, analogous to std::unique_ptr but without custom deleter support. </summary>
/// <typeparam name="T"> Type of the object owned by the box. </typeparam>
template <typename T>
class gm::box : private gm::_detail::box_traits<T> {
public:
    using pointer = typename gm::_detail::box_traits<T>::pointer;
    using reference = typename gm::_detail::box_traits<T>::reference;

    box() = default;
    ~box() { reset(); }

    box(std::nullptr_t) {}
    box& operator=(std::nullptr_t) {
        reset();
        return *this;
    }

    explicit box(pointer ptr) : _ptr(ptr) {}

    box(box&& src) : _ptr(src.release()) {}
    box& operator=(box&& src) {
        if (this != &src)
            reset(src.release());
        return *this;
    }

    template <typename U>
    box(box<U>&& src) : _ptr(src.release()) {}
    template <typename U>
    enable_if_t<std::is_assignable_v<T*, U*>, box&> operator=(box<U>&& src) {
        if (this != &src)
            reset(src.release());
        return *this;
    }

    pointer get() const { return _ptr; }
    pointer operator->() const { return _ptr; }

    reference operator*() const { return *_ptr; }
    reference operator[](size_t index) const { return _ptr[index]; }

    explicit operator bool() const { return _ptr != nullptr; }
    bool empty() const { return _ptr == nullptr; }

    inline void reset(pointer ptr = pointer{});
    [[nodiscard]] pointer release() {
        pointer tmp = _ptr;
        _ptr = nullptr;
        return tmp;
    }

    template <typename U>
    friend class box;

    template <typename U>
    friend bool operator==(box const& lhs, box<U> const& rhs) { return lhs._ptr == rhs._ptr; }
    template <typename U>
    friend bool operator!=(box const& lhs, box<U> const& rhs) { return lhs._ptr != rhs._ptr; }
    template <typename U>
    friend bool operator<=(box const& lhs, box<U> const& rhs) { return lhs._ptr <= rhs._ptr; }
    template <typename U>
    friend bool operator<(box const& lhs, box<U> const& rhs) { return lhs._ptr < rhs._ptr; }

    template <typename U>
    friend bool operator==(box const& lhs, U const* rhs) { return lhs._ptr == rhs; }
    template <typename U>
    friend bool operator!=(box const& lhs, U const* rhs) { return lhs._ptr != rhs; }
    template <typename U>
    friend bool operator<=(box const& lhs, U const* rhs) { return lhs._ptr <= rhs; }
    template <typename U>
    friend bool operator<(box const& lhs, U const* rhs) { return lhs._ptr < rhs; }

    template <typename U>
    friend bool operator==(U const* lhs, box const& rhs) { return lhs == rhs._ptr; }
    template <typename U>
    friend bool operator!=(U const* lhs, box const& rhs) { return lhs != rhs._ptr; }
    template <typename U>
    friend bool operator<=(U const* lhs, box const& rhs) { return lhs <= rhs._ptr; }
    template <typename U>
    friend bool operator<(U const* lhs, box const& rhs) { return lhs < rhs._ptr; }

    friend bool operator==(box const& lhs, std::nullptr_t) { return lhs._ptr == nullptr; }
    friend bool operator!=(box const& lhs, std::nullptr_t) { return lhs._ptr != nullptr; }
    friend bool operator<=(box const& lhs, std::nullptr_t) { return lhs._ptr == nullptr; }
    friend bool operator<(box const&, std::nullptr_t) { return false; }

    friend bool operator==(std::nullptr_t, box const& rhs) { return rhs._ptr == nullptr; }
    friend bool operator!=(std::nullptr_t, box const& rhs) { return rhs._ptr != nullptr; }
    friend bool operator<=(std::nullptr_t, box const&) { return true; }
    friend bool operator<(std::nullptr_t, box const& rhs) { return rhs._ptr != nullptr; }

private:
    pointer _ptr = nullptr;
};

template <typename T>
void gm::box<T>::reset(pointer ptr) {
    static_assert(sizeof(T) > 0, "box can not delete incomplete type");
    this->_deallocate(_ptr);
    _ptr = ptr;
}

/// <summary> Box an object; allocates a new instance and returns a box that owns the allocation </summary>
/// <typeparam name="T"> Type of object to box. </typeparam>
/// <param name="args"> Parameters to pass to the constructor. </param>
/// <returns> A box containing a new instance of the requested object. </returns>
template <typename T, typename... Args>
auto gm::make_box(Args&&... args) -> enable_if_t<std::is_constructible_v<T, Args&&...>, box<T>> {
    return box<T>(new T(std::forward<Args>(args)...));
}
