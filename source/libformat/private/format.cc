// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#include <potato/format/format.h>

#include <potato/format/_detail/format_arg_impl.h>
#include <potato/format/_detail/format_impl.h>

namespace up {
	UP_FORMAT_API format_result _detail::format_impl(format_writer& out, string_view format, format_arg_list args);
} // namespace formatxx
