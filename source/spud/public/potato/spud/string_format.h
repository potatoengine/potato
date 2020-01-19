// Copyright (C) 2014,2015,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "string_view.h"
#include <potato/format/format.h>

namespace up {
    template <typename Buffer>
    using format_writer = formatxx::append_writer<Buffer>;

    template <typename Buffer, typename... Args>
    constexpr decltype(auto) format_into(Buffer& buffer, string_view format, Args const&... args) {
        format_writer<Buffer> writer(buffer);
        return ::formatxx::format_to(writer, ::formatxx::string_view(format.data(), format.size()), args...);
    }
} // namespace up
