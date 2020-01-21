#include "potato/format/format.h"
#include "potato/format/std_string.h"
#include <doctest/doctest.h>
#include <ostream>

enum class standard_enum { one, two };
enum class custom_enum { foo, bar };

class custom_type {};

void format_value(up::format_writer& writer, custom_enum value, up::format_options options) noexcept {
    switch (value) {
    case custom_enum::foo: format_value_to(writer, "foo", options); return;
    case custom_enum::bar: format_value_to(writer, "bar", options); return;
    }
}

void format_value(up::format_writer& writer, custom_type, up::format_options options) noexcept {
    format_value_to(writer, "custom", options);
}
void format_value(up::format_writer& writer, custom_type const*, up::format_options options) noexcept {
    format_value_to(writer, "custom pointer", options);
}

template <typename T>
std::string format_as_string(T const& value) {
    std::string result;
    up::append_writer writer(result);
    up::format_value_to(writer, value, {});
    return result;
}

DOCTEST_TEST_CASE("format") {
    using namespace up;

    DOCTEST_SUBCASE("integers") {
        DOCTEST_CHECK_EQ("123987", format_as<std::string>("{}", 123987));

        DOCTEST_CHECK_EQ("0", format_as<std::string>("{}", 0));
        DOCTEST_CHECK_EQ("-1", format_as<std::string>("{}", -1));
        DOCTEST_CHECK_EQ("1", format_as<std::string>("{}", +1));
        DOCTEST_CHECK_EQ("+1", format_as<std::string>("{:+}", +1));
        DOCTEST_CHECK_EQ(" 1", format_as<std::string>("{: }", +1));

        DOCTEST_CHECK_EQ("127", format_as<std::string>("{}", std::numeric_limits<std::int8_t>::max()));
        DOCTEST_CHECK_EQ("32767", format_as<std::string>("{}", std::numeric_limits<std::int16_t>::max()));
        DOCTEST_CHECK_EQ("2147483647", format_as<std::string>("{}", std::numeric_limits<std::int32_t>::max()));
        DOCTEST_CHECK_EQ("9223372036854775807", format_as<std::string>("{}", std::numeric_limits<std::int64_t>::max()));

        // assumes two's complement implementation
        DOCTEST_CHECK_EQ("-128", format_as<std::string>("{}", std::numeric_limits<std::int8_t>::min()));
        DOCTEST_CHECK_EQ("-32768", format_as<std::string>("{}", std::numeric_limits<std::int16_t>::min()));
        DOCTEST_CHECK_EQ("-2147483648", format_as<std::string>("{}", std::numeric_limits<std::int32_t>::min()));
        DOCTEST_CHECK_EQ("-9223372036854775808", format_as<std::string>("{}", std::numeric_limits<std::int64_t>::min()));

        DOCTEST_CHECK_EQ("0", format_as<std::string>("{:x}", 0));
        DOCTEST_CHECK_EQ("0x0", format_as<std::string>("{:#x}", 0));
        DOCTEST_CHECK_EQ("ff", format_as<std::string>("{:x}", 255));
        DOCTEST_CHECK_EQ("0xff", format_as<std::string>("{:#x}", 255));
        DOCTEST_CHECK_EQ("0x100", format_as<std::string>("{:#x}", 256));
        DOCTEST_CHECK_EQ("0x11", format_as<std::string>("{:#x}", 17));
        DOCTEST_CHECK_EQ("0xffffffef", format_as<std::string>("{:-#x}", -17));

        DOCTEST_CHECK_EQ("101", format_as<std::string>("{:b}", 5));
        DOCTEST_CHECK_EQ("-10", format_as<std::string>("{:b}", -2));
        DOCTEST_CHECK_EQ("-0b10", format_as<std::string>("{:#b}", -2));

        DOCTEST_CHECK_EQ("+1234", format_as<std::string>("{:+d}", 1234));
    }

    DOCTEST_SUBCASE("floats") {
        DOCTEST_CHECK_EQ("123987.456000", format_as<std::string>("{}", 123987.456));

        DOCTEST_CHECK_EQ("0.000000", format_as<std::string>("{}", 0.0));
        DOCTEST_CHECK_EQ("1.000000", format_as<std::string>("{}", 1.0));
        DOCTEST_CHECK_EQ("-1.000000", format_as<std::string>("{}", -1.0));

        DOCTEST_CHECK_EQ("12.34", format_as<std::string>("{:2.2}", 12.34));
        DOCTEST_CHECK_EQ("12.00", format_as<std::string>("{:#2.2}", 12.0));
        DOCTEST_CHECK_EQ(" 12.34", format_as<std::string>("{: 6.2}", 12.34));
        DOCTEST_CHECK_EQ("012.34", format_as<std::string>("{:06.2}", 12.34));
        DOCTEST_CHECK_EQ("+12.34", format_as<std::string>("{:+06.2}", 12.34));
        DOCTEST_CHECK_EQ("12.34 ;", format_as<std::string>("{:-6.2};", 12.34));

        // assumes IEEE754 single- and double-precision types
        DOCTEST_CHECK_EQ("340282346638528859811704183484516925440.000000", format_as<std::string>("{}", std::numeric_limits<float>::max()));
        DOCTEST_CHECK_EQ("17976931348623157081452742373170435679807056752584499659891747680315"
            "72607800285387605895586327668781715404589535143824642343213268894641827684675"
            "46703537516986049910576551282076245490090389328944075868508455133942304583236"
            "90322294816580855933212334827479782620414472316873817718091929988125040402618"
            "4124858368.000000", format_as<std::string>("{}", std::numeric_limits<double>::max()));

        DOCTEST_CHECK_EQ("234987324.454500", format_as<std::string>("{:f}", 234987324.4545));
        DOCTEST_CHECK_EQ("2.34987e+08", format_as<std::string>("{:g}", 234987324.4545));
        DOCTEST_CHECK_EQ("0x1.c033e78e8b439p+27", format_as<std::string>("{:a}", 234987324.4545));
    }

    DOCTEST_SUBCASE("strings") {
        DOCTEST_CHECK_EQ("test", format_as<std::string>("{}", "test"));
        DOCTEST_CHECK_EQ("test", format_as<std::string>("{}", std::string("test")));
        DOCTEST_CHECK_EQ("test", format_as<std::string>("{}", std::string_view("test")));
        DOCTEST_CHECK_EQ("test", format_as<std::string>("{}", up::string_view("test")));

        DOCTEST_CHECK_EQ("abcdef", format_as<std::string>("{}{}{}", up::string_view("ab"), std::string("cd"), "ef"));

        DOCTEST_CHECK_EQ("abc", format_as<std::string>(std::string("a{}c"), "b"));

        DOCTEST_CHECK_EQ("value42", format_as<std::string>("{:-8}{:05}", "value", 42));
    }

    DOCTEST_SUBCASE("booleans") {
        DOCTEST_CHECK_EQ("true", format_as<std::string>("{}", true));
        DOCTEST_CHECK_EQ("false", format_as<std::string>("{}", false));
    }

    DOCTEST_SUBCASE("pointers") {
        void const* ptr = reinterpret_cast<void const*>(static_cast<std::uintptr_t>(0xDEADC0DE));
        int const* iptr = reinterpret_cast<int const*>(static_cast<std::uintptr_t>(0xFEFEFEFE));

        DOCTEST_CHECK_EQ("deadc0de", format_as<std::string>("{:x}", ptr));
        DOCTEST_CHECK_EQ("fefefefe", format_as<std::string>("{:x}", iptr));
    }

    DOCTEST_SUBCASE("enums") {
        DOCTEST_CHECK_EQ("1", format_as<std::string>("{:0}", standard_enum::two));
        DOCTEST_CHECK_EQ("bar", format_as<std::string>("{}", custom_enum::bar));
    }

    DOCTEST_SUBCASE("custom") {
        custom_type local;
        custom_type& ref = local;
        custom_type* ptr = &local;

        DOCTEST_CHECK_EQ("custom", format_as<std::string>("{}", custom_type{}));
        DOCTEST_CHECK_EQ("custom", format_as<std::string>("{}", local));
        DOCTEST_CHECK_EQ("custom", format_as<std::string>("{}", ref));
        DOCTEST_CHECK_EQ("custom pointer", format_as<std::string>("{}", ptr));
    }

    DOCTEST_SUBCASE("errors") {
        char buffer[256];
        fixed_writer writer(buffer);

        DOCTEST_CHECK_EQ(up::result_code::success, format_to(writer, "{} {:4d} {:3.5f}", "abc", 9, 12.57));
        DOCTEST_CHECK_EQ(up::result_code::malformed_input, format_to(writer, "{} {:4d", "abc", 9));
        DOCTEST_CHECK_EQ(up::result_code::success, format_to(writer, "{0} {1}", "abc", 9));
        DOCTEST_CHECK_EQ(up::result_code::out_of_range, format_to(writer, "{0} {1} {5}", "abc", 9, 12.57));
    }

    DOCTEST_SUBCASE("format_value_into") {
        DOCTEST_CHECK_EQ("123", format_as_string(123));
    }
}
