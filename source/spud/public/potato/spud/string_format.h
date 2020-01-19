// Copyright (C) 2014,2015,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "string_view.h"
#include <potato/format/format.h>

namespace up {
    template <typename Buffer>
    using format_writer = format::append_writer<Buffer>;

    template <typename Buffer, typename... Args>
    constexpr decltype(auto) format_append(Buffer& buffer, string_view format, Args const&... args) {
        return format::format_append(buffer, format, args...);
    }
} // namespace up
