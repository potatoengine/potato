// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#pragma once

#include "potato/format/format.h"
#include <type_traits>
#include <limits>
#include <climits>

namespace up::_detail {

	template <typename T>
    constexpr void write_integer(format_writer& out, T value, format_options const& options);

	struct prefix_helper {
		// type prefix (2), sign (1)
		static constexpr std::size_t buffer_size() { return 3; }

		static constexpr string_view write(char* buffer, format_options const& options, bool negative, bool add_sign) {
			char* const end = buffer + buffer_size();
			char* ptr = end;

			// add numeric type prefix (2)
			if (options.alternate_form) {
				*--ptr = options.specifier;
				*--ptr = FormatTraits<char>::cZero;
			}

			// add sign (1)
            if (add_sign) {
                if (negative) {
                    *--ptr = FormatTraits<char>::cMinus;
                }
                else if (options.sign == format_sign::always) {
                    *--ptr = FormatTraits<char>::cPlus;
                }
                else if (options.sign == format_sign::space) {
                    *--ptr = FormatTraits<char>::cSpace;
                }
            }

			return { ptr, end };
		}
	};

	struct decimal_helper {
		// buffer must be one larger than digits10, as that trait is the maximum number of 
		// base-10 digits represented by the type in their entirety, e.g. 8-bits can store
		// 99 but not 999, so its digits10 is 2, even though the value 255 could be stored
		// and has 3 digits.
		template <typename UnsignedT>
		static constexpr std::size_t buffer_size = std::numeric_limits<UnsignedT>::digits10 + 1;

        static constexpr bool use_signs = true;

		template <typename UnsignedT>
		static constexpr string_view write(char* buffer, UnsignedT value) {
			// we'll work on every two decimal digits (groups of 100). notes taken from cppformat,
			// which took the notes from Alexandrescu from "Three Optimization Tips for C++"
			char const* const table = FormatTraits<char>::sDecimalPairs;

			char* const end = buffer + buffer_size<UnsignedT>;
			char* ptr = end;

			// work on every two decimal digits (groups of 100). notes taken from cppformat,
			// which took the notes from Alexandrescu from "Three Optimization Tips for C++"
			while (value >= 100) {
				// I feel like we could do the % and / better... somehow
				// we multiply the index by two to find the pair of digits to index
				unsigned const digit = (value % 100) << 1;
				value /= 100;

				// write out both digits of the given index
				*--ptr = table[digit + 1];
				*--ptr = table[digit];
			}

			if (value >= 10) {
				// we have two digits left; this is identical to the above loop, but without the division
				unsigned const digit = static_cast<unsigned>(value << 1);
				*--ptr = table[digit + 1];
				*--ptr = table[digit];
			}
			else {
				// we have but a single digit left, so this is easy
				*--ptr = static_cast<char>(FormatTraits<char>::cZero + value);
			}

			return { ptr, end };
		}
	};

	template <bool LowerCase>
	struct hexadecimal_helper {
		// 2 hex digits per octet
		template <typename UnsignedT>
		static constexpr std::size_t buffer_size = 2 * sizeof(UnsignedT);

        static constexpr bool use_signs = false;

		template <typename UnsignedT>
		static constexpr string_view write(char* buffer, UnsignedT value) {
			char* const end = buffer + buffer_size<UnsignedT>;
			char* ptr = end;

			char const* const alphabet = LowerCase ?
				FormatTraits<char>::sHexadecimalLower :
				FormatTraits<char>::sHexadecimalUpper;

			do {
				*--ptr = alphabet[value & 0xF];
			} while ((value >>= 4) != 0);

			return { ptr, end };
		}
	};

	struct octal_helper {
		// up to three 3 octal digits per octet - FIXME is that right? I don't think that's right
		template <typename UnsignedT>
		static constexpr std::size_t buffer_size = 3 * sizeof(UnsignedT);

        static constexpr bool use_signs = true;

		template <typename UnsignedT>
		static constexpr string_view write(char* buffer, UnsignedT value) {
			char* const end = buffer + buffer_size<UnsignedT>;
			char* ptr = end;

			// the octal alphabet is a subset of hexadecimal,
			// and doesn't depend on casing.
			char const* const alphabet = FormatTraits<char>::sHexadecimalLower;

			do {
				*--ptr = alphabet[value & 0x7];
			} while ((value >>= 3) != 0);

			return { ptr, end };
		}
	};

	struct binary_helper {
		// one digit per bit of the input
		template <typename UnsignedT>
		static constexpr std::size_t buffer_size = std::numeric_limits<UnsignedT>::digits;

        static constexpr bool use_signs = true;

		template <typename UnsignedT>
		static constexpr string_view write(char* buffer, UnsignedT value) {
			char* const end = buffer + buffer_size<UnsignedT>;
			char* ptr = end;

			do {
				*--ptr = static_cast<char>(FormatTraits<char>::cZero + (value & 1));
			} while ((value >>= 1) != 0);

			return { ptr, end };
		}
	};

	template <typename HelperT, typename ValueT>
	constexpr void write_integer_helper(format_writer & out, ValueT raw_value, format_options const& options) {
		using unsigned_type = std::make_unsigned_t<ValueT>;

		// convert to an unsigned value to make the formatting easier; note that must
		// subtract from 0 _after_ converting to deal with 2's complement format
		// where (abs(min) > abs(max)), otherwise we'd not be able to format -min<T>
		unsigned_type const unsigned_value = raw_value >= 0 ? raw_value : 0 - static_cast<unsigned_type>(raw_value);

		// calculate prefixes like signs
		char prefix_buffer[prefix_helper::buffer_size()] = {0,};
		auto const prefix = prefix_helper::write(prefix_buffer, options, raw_value < 0, HelperT::use_signs);

		// generate the actual number
		char value_buffer[HelperT::template buffer_size<unsigned_type>];
		auto const result = HelperT::write(value_buffer, unsigned_value);

		if (options.justify == format_justify::left) {
			out.write(prefix);
			out.write(result);
		}
		else if (options.leading_zeroes) {
			out.write(prefix);
			out.write(result);
		}
		else {
			out.write(prefix);
			out.write(result);
		}
	}

	template <typename T>
	constexpr void write_integer(format_writer & out, T raw, format_options const& options) {
		switch (options.specifier) {
		default:
		case 0:
		case 'i':
		case 'd':
		case 'D':
			return write_integer_helper<decimal_helper>(out, raw, options);
		case 'x':
			return write_integer_helper<hexadecimal_helper</*lower=*/true>>(out, std::make_unsigned_t<T>(raw), options);
		case 'X':
			return write_integer_helper<hexadecimal_helper</*lower=*/false>>(out, std::make_unsigned_t<T>(raw), options);
		case 'o':
			return write_integer_helper<octal_helper>(out, raw, options);
			break;
		case 'b':
			return write_integer_helper<binary_helper>(out, raw, options);
			break;
		}
	}

} // namespace up::_detail
