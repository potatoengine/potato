// Copyright (C) 2015,2019 Sean Middleditch, all rights reserverd.

#pragma once

namespace up {
    template <typename S, typename P>
    class out_ptr_t;

    template <typename S>
    auto out_ptr(S& out) -> out_ptr_t<S, typename S::pointer>;
} // namespace up

template <typename S, typename P>
class up::out_ptr_t {
public:
    using pointer = P;

    out_ptr_t(S& out) noexcept : _out(out){};
    ~out_ptr_t() noexcept { _out.reset(_ptr); }

    out_ptr_t(out_ptr_t const&) = delete;
    out_ptr_t& operator=(out_ptr_t&) = delete;

    operator P*() noexcept {
        delete _ptr;
        _ptr = nullptr;
        return &_ptr;
    }

    operator void* *() noexcept { return reinterpret_cast<void**>(operator P*()); }

private:
    P _ptr = nullptr;
    S& _out;
};

template <typename S>
auto up::out_ptr(S& out) -> out_ptr_t<S, typename S::pointer> {
    return out_ptr_t<S, typename S::pointer>(out);
}
