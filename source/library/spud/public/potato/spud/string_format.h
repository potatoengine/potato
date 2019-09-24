// Copyright (C) 2014,2015,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "string_view.h"
#include <formatxx/format.h>

namespace up {
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
        return ::formatxx::format_to(writer, ::formatxx::string_view(format.data(), format.size()), args...);
    }
} // namespace up