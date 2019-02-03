// Copyright (C) 2014 Sean Middleditch, all rights reserverd.

#pragma once

#include <utility>
#include <atomic>

namespace gm {
    template <typename T>
    class rc;

    template <typename Derived>
    class shared {
    public:
        shared(shared const&) = delete;
        shared& operator=(shared const&) = delete;

        // moving is allowed, but does not impact references
        shared(shared&&) noexcept {}
        shared& operator=(shared&&) noexcept { return *this; }

    protected:
        shared() noexcept = default;

        // explicitly not virtual, since nobody can delete via shared<T>
        ~shared() = default;

    private:
        template <typename U>
        friend class rc;

        void addRef() const noexcept { ++_refs; }
        void removeRef() const noexcept {
            if (--_refs == 0) {
                delete static_cast<Derived const*>(this);
            }
        }

    private:
        mutable std::atomic<int> _refs = 1;
    };

    template <typename T>
    class rc {
    public:
        using pointer = T*;
        using reference = T&;

        rc() = default;
        explicit rc(pointer ptr) : _ptr(ptr) {}
        ~rc() { _removeRef(); }

        rc(rc const& rhs) : _ptr(rhs._ptr) { _addRef(); }
        rc(rc&& rhs) : _ptr(rhs.release()) {}
        template <typename U>
        /*implicit*/ rc(rc<U> const& rhs) : _ptr(rhs.get()) { _addRef(); }
        template <typename U>
        /*implicit*/ rc(rc<U>&& rhs) : _ptr(rhs.release()) {}
        /*implicit*/ rc(std::nullptr_t) {}

        inline rc& operator=(rc const& rhs);
        inline rc& operator=(rc&& rhs);
        template <typename U>
        inline rc& operator=(rc<U> const& rhs);
        template <typename U>
        inline rc& operator=(rc<U>&& rhs);
        inline rc& operator=(std::nullptr_t);

        explicit operator bool() const { return _ptr != nullptr; }
        bool empty() const { return _ptr == nullptr; }

        inline void reset(pointer ptr = pointer{});

        [[nodiscard]] inline pointer release();

        pointer get() const { return _ptr; }
        pointer operator->() const { return _ptr; }
        reference operator*() const { return *_ptr; }

        friend bool operator==(rc const& lhs, rc const& rhs) { return lhs.get() == rhs.get(); }
        friend bool operator!=(rc const& lhs, rc const& rhs) { return lhs.get() != rhs.get(); }
        friend bool operator==(rc const& lhs, std::nullptr_t) { return lhs.get() == nullptr; }
        friend bool operator!=(rc const& lhs, std::nullptr_t) { return lhs.get() != nullptr; }
        friend bool operator==(std::nullptr_t, rc const& rhs) { return nullptr == rhs.get(); }
        friend bool operator!=(std::nullptr_t, rc const& rhs) { return nullptr != rhs.get(); }

    private:
        void _addRef() {
            if (_ptr != nullptr) {
                _ptr->addRef();
            }
        }
        void _removeRef() {
            if (_ptr != nullptr) {
                _ptr->removeRef();
            }
        }

        pointer _ptr = nullptr;
    };

    template <typename T>
    auto rc<T>::operator=(rc const& rhs) -> rc& {
        if (this != std::addressof(rhs)) {
            _removeRef();
            _ptr = rhs._ptr;
            _addRef();
        }
        return *this;
    }

    template <typename T>
    template <typename U>
    auto rc<T>::operator=(rc<U> const& rhs) -> rc& {
        if (this != std::addressof(rhs)) {
            _removeRef();
            _ptr = rhs.get();
            _addRef();
        }
        return *this;
    }

    template <typename T>
    auto rc<T>::operator=(rc&& rhs) -> rc& {
        if (this != std::addressof(rhs)) {
            _removeRef();
            _ptr = rhs.release();
        }
        return *this;
    }

    template <typename T>
    template <typename U>
    auto rc<T>::operator=(rc<U>&& rhs) -> rc& {
        if (this != std::addressof(rhs)) {
            _removeRef();
            _ptr = rhs.release();
        }
        return *this;
    }

    template <typename T>
    auto rc<T>::operator=(std::nullptr_t) -> rc& {
        _removeRef();
        _ptr = nullptr;
        return *this;
    }

    template <typename T>
    void rc<T>::reset(pointer ptr) {
        if (ptr != _ptr) {
            _removeRef();
            _ptr = ptr;
        }
    }

    template <typename T>
    auto rc<T>::release() -> pointer {
        pointer tmp = _ptr;
        _ptr = nullptr;
        return tmp;
    }

    template <typename T, typename... A>
    rc<T> make_shared(A&&... args) {
        return rc<T>(new T(std::forward<A>(args)...));
    }

} // namespace gm
