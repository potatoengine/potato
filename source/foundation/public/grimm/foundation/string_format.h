// Copyright (C) 2014,2015 Sean Middleditch, all rights reserverd.

#pragma once

#include "debug.h"
#include "string_view.h"

// needed by fmtlib
#if !defined(FMT_ASSERT)
#    define FMT_ASSERT(cond, message) \
        do { \
            if (!(cond)) { \
                ::gm::fatal_error(__FILE__, __LINE__, #cond, (message)); \
                std::abort(); \
            } \
        } while (false)
#endif
#if !defined(assert)
#    define assert(cond) \
        FMT_ASSERT((cond), "assertion failed")
#endif

#include <formatxx/format.h>

namespace gm {
    template <typename Buffer>
    class format_writer : public formatxx::format_writer {
    public:
        constexpr format_writer(Buffer& buffer) noexcept : _buffer(buffer) {}

        void write(formatxx::string_view str) noexcept override {
            for (char ch : str) {
                _buffer.push_back(ch);
            }
        }

    private:
        Buffer& _buffer;
    };

    template <typename Buffer, typename... Args>
    constexpr decltype(auto) format_into(Buffer& buffer, string_view format, Args const&... args) {
        format_writer<Buffer> writer(buffer);
        return ::formatxx::format(writer, ::formatxx::string_view(format.data(), format.size()), args...);
    }
} // namespace gm
