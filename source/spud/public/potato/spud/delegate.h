// Copyright (C) 2015,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_assertion.h"
#include "traits.h"
#include "functional.h"

#include <new>

namespace up {
    template <typename Signature>
    class delegate;

    template <typename T>
    delegate(T)->delegate<signature_t<T>>;

    template <typename ClassType, typename ReturnType, typename... ParamTypes>
    delegate(ClassType& object, ReturnType (ClassType::*)(ParamTypes...))->delegate<ReturnType(ParamTypes...)>;

    template <typename ClassType, typename ReturnType, typename... ParamTypes>
    delegate(ClassType const& object, ReturnType (ClassType::*)(ParamTypes...) const)->delegate<ReturnType(ParamTypes...) const>;

    template <typename ClassType, typename ReturnType, typename... ParamTypes>
    delegate(ClassType&& object, ReturnType (ClassType::*)(ParamTypes...) const)->delegate<ReturnType(ParamTypes...) const>;

    template <typename ClassType, typename ReturnType, typename... ParamTypes>
    delegate(ClassType* object, ReturnType (ClassType::*)(ParamTypes...))->delegate<ReturnType(ParamTypes...)>;

    template <typename ClassType, typename ReturnType, typename... ParamTypes>
    delegate(ClassType const* object, ReturnType (ClassType::*)(ParamTypes...) const)->delegate<ReturnType(ParamTypes...) const>;

    namespace _detail {
        static constexpr size_t delegate_size_c = 3;

        struct delegate_vtable_base {
            using move_t = void (*)(void* dst, void* src);
            using destruct_t = void (*)(void* obj);

            move_t move = nullptr;
            destruct_t destruct = nullptr;
        };

        template <typename R, bool C, typename... P>
        struct delegate_vtable_typed : delegate_vtable_base {
            using call_t = R (*)(std::conditional_t<C, void const*, void*> obj, P&&... params);

            call_t call = nullptr;
        };

        template <typename F>
        struct delegate_vtable_impl {
            static void move(void* dst, void* src) noexcept { new (dst) F(std::move(*static_cast<F*>(src))); }
            static void destruct(void* obj) noexcept { static_cast<F*>(obj)->~F(); }
            template <typename R, typename... P>
            static R call(std::conditional_t<std::is_const_v<F>, void const*, void*> obj, P&&... params) {
                F& f = *static_cast<F*>(obj);
                if constexpr (std::is_void_v<R>) {
                    invoke(std::forward<F>(f), std::forward<P>(params)...);
                }
                else {
                    return invoke(std::forward<F>(f), std::forward<P>(params)...);
                }
            }
        };

        template <typename F, typename R, bool C, typename... P>
        constexpr auto vtable_c = delegate_vtable_typed<R, C, P...>{
            {&delegate_vtable_impl<F>::move,
             &delegate_vtable_impl<F>::destruct},
            &delegate_vtable_impl<F>::template call<R, P...>};

        class delegate_base {
        protected:
            using storage_t = std::aligned_storage_t<_detail::delegate_size_c * sizeof(void*), std::alignment_of<double>::value>;

        public:
            delegate_base() noexcept = default;

            delegate_base(const delegate_base& rhs) = delete;
            delegate_base& operator=(const delegate_base& rhs) = delete;

            inline delegate_base(delegate_base&& rhs) noexcept;
            inline delegate_base& operator=(delegate_base&& rhs) noexcept;

            /*implicit*/ delegate_base(std::nullptr_t null) noexcept {}
            delegate_base& operator=(std::nullptr_t null) {
                reset();
                return *this;
            }

            /// <summary> Check if the delegate is currently bound to a function. </summary>
            /// <returns> True if a delegate is bound. </returns>
            explicit operator bool() const noexcept { return _vtable != nullptr; }

            /// <summary> Check if the delegate is not bound to a function. </summary>
            /// <returns> True if no delegate. </returns>
            bool empty() const noexcept { return _vtable == nullptr; }

            inline void reset(std::nullptr_t = nullptr);

            bool operator!=(std::nullptr_t) const noexcept { return _vtable != nullptr; }
            bool operator==(std::nullptr_t) const noexcept { return _vtable == nullptr; }

        protected:
            delegate_base(delegate_vtable_base const* vtable) noexcept : _vtable(vtable) {}
            ~delegate_base() = default;

        protected:
            // we will overwrite this with an object with just a vtable - if we are nullptr, we have no real vtable
            delegate_vtable_base const* _vtable = nullptr;
            storage_t _storage = {};
        };

        template <typename ReturnType, bool Const, typename... ParamTypes>
        class delegate_typed : public delegate_base {
        protected:
            using vtable_c = _detail::delegate_vtable_typed<ReturnType, Const, ParamTypes...>;
            using storage_t = typename delegate_base::storage_t;

        public:
            using delegate_base::delegate_base;

            /// <summary> Construct a new delegate from a function object, such as a lambda or function pointer. </summary>
            /// <param name="function"> The function to bind. </param>
            template <typename Functor, typename = enable_if_t<is_invocable_v<Functor, ParamTypes...> && !std::is_base_of_v<delegate_typed, std::decay_t<Functor>>>>
            /*implicit*/ delegate_typed(Functor&& functor) { assign(std::forward<Functor>(functor)); }

            template <typename Functor, typename = enable_if_t<is_invocable_v<Functor, ParamTypes...> && !std::is_base_of_v<delegate_typed, std::decay_t<Functor>>>>
            delegate_typed& operator=(Functor&& functor);

        private:
            template <typename Functor>
            void assign(Functor&& functor);
        };
    } // namespace _detail
} // namespace up

template <typename ReturnType, typename... ParamTypes>
class up::delegate<ReturnType(ParamTypes...)> : public _detail::delegate_typed<ReturnType, false, ParamTypes...> {
    using vtable_c = _detail::delegate_vtable_typed<ReturnType, false, ParamTypes...>;
    using storage_t = typename _detail::delegate_typed<ReturnType, false, ParamTypes...>::storage_t;

public:
    using _detail::delegate_typed<ReturnType, false, ParamTypes...>::delegate_typed;

    template <typename ClassType>
    delegate(ClassType& object, ReturnType (ClassType::*method)(ParamTypes...)) noexcept
        : delegate([&object, method](ParamTypes&&... params) { return (object.*method)(std::forward<ParamTypes>(params)...); }) {}

    template <typename ClassType>
    delegate(ClassType&& object, ReturnType (ClassType::*method)(ParamTypes...) const) noexcept
        : delegate([object = std::forward<ClassType>(object), method](ParamTypes&&... params) { return (object.*method)(std::forward<ParamTypes>(params)...); }) {}

    template <typename ClassType>
    delegate(ClassType* object, ReturnType (ClassType::*method)(ParamTypes...)) noexcept
        : delegate([object, method](ParamTypes&&... params) { return (object->*method)(std::forward<ParamTypes>(params)...); }) {}

    auto operator()(ParamTypes... params) -> ReturnType {
        UP_SPUD_ASSERT(this->_vtable != nullptr, "Invoking an empty delegate");
        return static_cast<vtable_c const*>(this->_vtable)->call(&this->_storage, std::forward<ParamTypes>(params)...);
    }
};

template <typename ReturnType, typename... ParamTypes>
class up::delegate<ReturnType(ParamTypes...) const> : public _detail::delegate_typed<ReturnType, true, ParamTypes...> {
    using vtable_c = _detail::delegate_vtable_typed<ReturnType, true, ParamTypes...>;
    using storage_t = typename _detail::delegate_typed<ReturnType, true, ParamTypes...>::storage_t;

public:
    using _detail::delegate_typed<ReturnType, true, ParamTypes...>::delegate_typed;

    template <typename ClassType>
    delegate(ClassType const& object, ReturnType (ClassType::*method)(ParamTypes...) const)
        : delegate([&object, method](ParamTypes&&... params) { return (object.*method)(std::forward<ParamTypes>(params)...); }) {}

    template <typename ClassType>
    delegate(ClassType const* object, ReturnType (ClassType::*method)(ParamTypes...) const)
        : delegate([object, method](ParamTypes&&... params) { return (object->*method)(std::forward<ParamTypes>(params)...); }) {}

    auto operator()(ParamTypes... params) const -> ReturnType {
        UP_SPUD_ASSERT(this->_vtable != nullptr, "Invoking an empty delegate");
        return static_cast<vtable_c const*>(this->_vtable)->call(&this->_storage, std::forward<ParamTypes>(params)...);
    }
};

up::_detail::delegate_base::delegate_base(delegate_base&& rhs) noexcept : _vtable(rhs._vtable) {
    if (_vtable != nullptr) {
        _vtable->move(&_storage, &rhs._storage);
        _vtable->destruct(&rhs._storage);

        rhs._vtable = nullptr;
    }
}

auto up::_detail::delegate_base::operator=(delegate_base&& rhs) noexcept -> delegate_base& {
    if (this != &rhs) {
        if (_vtable != nullptr) {
            _vtable->destruct(&_storage);
        }

        _vtable = rhs._vtable;

        if (_vtable != nullptr) {
            _vtable->move(&_storage, &rhs._storage);
            _vtable->destruct(&rhs._storage);

            rhs._vtable = nullptr;
        }
    }

    return *this;
}

void up::_detail::delegate_base::reset(std::nullptr_t) {
    if (_vtable != nullptr) {
        _vtable->destruct(&_storage);
        _vtable = nullptr;
    }
}
template <typename ReturnType, bool Const, typename... ParamTypes>
template <typename Functor, typename>
auto up::_detail::delegate_typed<ReturnType, Const, ParamTypes...>::operator=(Functor&& functor) -> delegate_typed& {
    if (this->_vtable != nullptr) {
        this->_vtable->destruct(&this->_storage);
    }

    assign(std::forward<Functor>(functor));
    return *this;
}

template <typename ReturnType, bool Const, typename... ParamTypes>
template <typename Functor>
void up::_detail::delegate_typed<ReturnType, Const, ParamTypes...>::assign(Functor&& functor) {
    using FunctorType = std::decay_t<Functor>;

    static_assert(alignof(FunctorType) <= alignof(storage_t), "Alignment of the functor given to delegate is too strict");
    static_assert(sizeof(FunctorType) <= sizeof(storage_t), "Size of the functor given to delegate is too wide");

    if constexpr (Const) {
        this->_vtable = &_detail::vtable_c<FunctorType const, ReturnType, true, ParamTypes...>;
    }
    else {
        this->_vtable = &_detail::vtable_c<FunctorType, ReturnType, false, ParamTypes...>;
    }
    new (&this->_storage) FunctorType(std::forward<Functor>(functor));
}
