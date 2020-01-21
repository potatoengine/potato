// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#pragma once

namespace up::_detail {

	template <typename CharT> struct FormatTraits;

	template <> struct FormatTraits<char> {
		static constexpr char cFormatBegin = '{';
		static constexpr char cFormatEnd = '}';
		static constexpr char cFormatSep = ':';

		static constexpr char cPlus = '+';
		static constexpr char cMinus = '-';
		static constexpr char cSpace = ' ';
		static constexpr char cHash = '#';
		static constexpr char cDot = '.';
        static constexpr char cZero = '0';

		static constexpr string_view sTrue{ "true" };
		static constexpr string_view sFalse{ "false" };
        static constexpr string_view sNullptr{ "nullptr" };

        static constexpr string_view sFormatSpecifiers{ "bcsdioxXfFeEaAgG" };

		static constexpr char const sDecimalPairs[] =
			"00010203040506070809"
			"10111213141516171819"
			"20212223242526272829"
			"30313233343536373839"
			"40414243444546474849"
			"50515253545556575859"
			"60616263646566676869"
			"70717273747576777879"
			"80818283848586878889"
			"90919293949596979899";
		static constexpr char const sHexadecimalLower[] = "0123456789abcdef";
		static constexpr char const sHexadecimalUpper[] = "0123456789ABCDEF";
	};
} // namespace up::_detail
