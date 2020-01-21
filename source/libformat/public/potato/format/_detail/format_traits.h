// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#pragma once

namespace up::_detail {

	template <typename CharT> struct FormatTraits;

	template <> struct FormatTraits<char> {
		static constexpr char cFormatBegin = '{';
		static constexpr char cFormatEnd = '}';
		static constexpr char cFormatSep = ':';

		static constexpr char cDot = '.';
        static constexpr char cZero = '0';

		static constexpr string_view sTrue{ "true" };
		static constexpr string_view sFalse{ "false" };
        static constexpr string_view sNullptr{ "nullptr" };

        static constexpr string_view sFormatSpecifiers{ "bcsdxfeag" };
	};
} // namespace up::_detail
