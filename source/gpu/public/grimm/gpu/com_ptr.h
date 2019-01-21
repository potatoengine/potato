// Copyright (C) 2014 Sean Middleditch, all rights reserverd.

#pragma once

#include <utility>
#include <type_traits>

namespace gm {

    template <typename T>
    class com_ptr {
    public:
        using pointer = T*;
        using reference = T&;

        com_ptr() = default;
        ~com_ptr() { _decRef(); }

        explicit com_ptr(T* ptr) : _ptr(ptr) {}
        template <typename U>
        explicit com_ptr(std::enable_if_t<std::is_assignable_v<pointer, U*>, U*> ptr) : _ptr(ptr) {}
        com_ptr(std::nullptr_t) {}

        com_ptr(com_ptr const& rhs) : _ptr(rhs._ptr) { _addRef(); }
        template <typename U>
        com_ptr(std::enable_if_t<std::is_assignable_v<pointer, U*>, com_ptr<U> const&> rhs) : _ptr(rhs.get()) { _addRef(); }

        com_ptr(com_ptr&& rhs) : _ptr(rhs._ptr) { rhs._ptr = nullptr; }
        template <typename U>
        com_ptr(std::enable_if_t<std::is_assignable_v<pointer, U*>, com_ptr<U>&&> rhs) : _ptr(rhs.release()) {}

        inline com_ptr& operator=(com_ptr const& rhs);
        inline com_ptr& operator=(com_ptr&& rhs);
        inline com_ptr& operator=(std::nullptr_t);

        explicit operator bool() const { return _ptr != nullptr; }
        bool empty() const { return _ptr == nullptr; }

        inline void reset(pointer ptr = pointer{});

        [[nodiscard]] inline pointer release();

        pointer get() const { return _ptr; }
        pointer operator->() const { return _ptr; }
        reference operator*() const { return *_ptr; }

        friend bool operator==(com_ptr const& lhs, com_ptr const& rhs) { return lhs.get() == rhs.get(); }
        friend bool operator!=(com_ptr const& lhs, com_ptr const& rhs) { return lhs.get() != rhs.get(); }
        friend bool operator==(com_ptr const& lhs, std::nullptr_t) { return lhs.get() == nullptr; }
        friend bool operator!=(com_ptr const& lhs, std::nullptr_t) { return lhs.get() != nullptr; }
        friend bool operator==(std::nullptr_t, com_ptr const& rhs) { return nullptr == rhs.get(); }
        friend bool operator!=(std::nullptr_t, com_ptr const& rhs) { return nullptr != rhs.get(); }

    private:
        void _addRef() {
            if (_ptr != nullptr)
                _ptr->AddRef();
        }
        void _decRef() {
            if (_ptr != nullptr)
                _ptr->Release();
        }

        pointer _ptr = nullptr;
    };

    template <typename T>
    auto com_ptr<T>::operator=(com_ptr const& rhs) -> com_ptr& {
        if (this != std::addressof(rhs)) {
            _decRef();
            _ptr = rhs._ptr;
            _addRef();
        }
        return *this;
    }

    template <typename T>
    auto com_ptr<T>::operator=(com_ptr&& rhs) -> com_ptr& {
        if (this != std::addressof(rhs)) {
            _decRef();
            _ptr = rhs._ptr;
            rhs._ptr = nullptr;
        }
        return *this;
    }

    template <typename T>
    auto com_ptr<T>::operator=(std::nullptr_t) -> com_ptr& {
        _decRef();
        _ptr = nullptr;
        return *this;
    }

    template <typename T>
    void com_ptr<T>::reset(pointer ptr) {
        if (ptr != _ptr) {
            _decRef();
            _ptr = ptr;
        }
    }

    template <typename T>
    auto com_ptr<T>::release() -> pointer {
        pointer tmp = _ptr;
        _ptr = nullptr;
        return tmp;
    }

} // namespace gm
