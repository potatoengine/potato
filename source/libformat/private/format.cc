// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#include <potato/format/format.h>

#include <potato/format/_detail/format_arg_impl.h>
#include <potato/format/_detail/format_impl.h>
#include <potato/format/_detail/parse_format.h>
#include <potato/format/_detail/write_string.h>

namespace up {
    UP_FORMAT_API void format_value(format_writer& output, string_view value, string_view spec_string) noexcept {
        _detail::write_string(output, value, spec_string);
    }

	UP_FORMAT_API result_code _detail::format_impl(format_writer& out, string_view format, format_arg_list args);
} // namespace formatxx
