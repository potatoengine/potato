// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/format/format.h"
#include "potato/format/std_string.h"

#include <catch2/catch.hpp>

enum class standard_enum { one, two };
enum class custom_enum { foo, bar };

class custom_type {};

namespace up {
    template <>
    struct formatter<custom_enum> {
        template <typename OutputT>
        void format(OutputT& output, custom_enum value) {
            switch (value) {
                case custom_enum::foo:
                    format_write(output, "foo");
                    break;
                case custom_enum::bar:
                    format_write(output, "bar");
                    break;
            }
        }
    };

    template <>
    struct formatter<custom_type> {
        template <typename OutputT>
        void format(OutputT& output, custom_type) {
            format_write(output, "custom");
        }
    };
} // namespace up

TEST_CASE("potato.format.format", "[potato][format]") {
    using namespace up;

    SECTION("integers") {
        CHECK(format_as<std::string>("{}", 123987) == "123987");

        CHECK(format_as<std::string>("{}", 0) == "0");
        CHECK(format_as<std::string>("{}", -1) == "-1");
        CHECK(format_as<std::string>("{}", +1) == "1");

        CHECK(format_as<std::string>("{}", std::numeric_limits<std::int8_t>::max()) == "127");
        CHECK(format_as<std::string>("{}", std::numeric_limits<std::int16_t>::max()) == "32767");
        CHECK(format_as<std::string>("{}", std::numeric_limits<std::int32_t>::max()) == "2147483647");
        CHECK(format_as<std::string>("{}", std::numeric_limits<std::int64_t>::max()) == "9223372036854775807");

        // assumes two's complement implementation
        CHECK(format_as<std::string>("{}", std::numeric_limits<std::int8_t>::min()) == "-128");
        CHECK(format_as<std::string>("{}", std::numeric_limits<std::int16_t>::min()) == "-32768");
        CHECK(format_as<std::string>("{}", std::numeric_limits<std::int32_t>::min()) == "-2147483648");
        CHECK(format_as<std::string>("{}", std::numeric_limits<std::int64_t>::min()) == "-9223372036854775808");

        CHECK(format_as<std::string>("{:x}", 0) == "0");
        CHECK(format_as<std::string>("{:x}", 255) == "ff");
        CHECK(format_as<std::string>("{:x}", 256) == "100");
        CHECK(format_as<std::string>("{:3x}", 17) == " 11");
        CHECK(format_as<std::string>("{:x}", -17) == "-11");
        CHECK(format_as<std::string>("{:x}", ~16u) == "ffffffef");

        CHECK(format_as<std::string>("{:b}", 5) == "101");
        CHECK(format_as<std::string>("{:b}", -2) == "-10");

        CHECK(format_as<std::string>("{:6d}", 1234) == "  1234");
        CHECK(format_as<std::string>("{:06d}", 1234) == "001234");
        CHECK(format_as<std::string>("{:08x}", 0xDEAD) == "0000dead");
        CHECK(format_as<std::string>("{:08X}", 0xDEAD) == "0000DEAD");
    }

    SECTION("floats") {
        CHECK(format_as<std::string>("{}", 123987.456) == "123987.456000");

        CHECK(format_as<std::string>("{}", 0.0) == "0.000000");
        CHECK(format_as<std::string>("{}", 1.0) == "1.000000");
        CHECK(format_as<std::string>("{}", -1.0) == "-1.000000");

        CHECK(format_as<std::string>("{:2.2}", 12.34) == "12.34");
        CHECK(format_as<std::string>("{:2.2}", 12.0) == "12.00");
        CHECK(format_as<std::string>("{:6.2}", 12.34) == " 12.34");
        CHECK(format_as<std::string>("{:06.2}", 12.34) == "012.34");

        // assumes IEEE754 single- and double-precision types
        CHECK(
            format_as<std::string>("{}", std::numeric_limits<float>::max()) ==
            "340282346638528859811704183484516925440.000000");
        CHECK(
            format_as<std::string>("{}", std::numeric_limits<double>::max()) ==
            "17976931348623157081452742373170435679807056752584499659891747680315"
            "72607800285387605895586327668781715404589535143824642343213268894641827684675"
            "46703537516986049910576551282076245490090389328944075868508455133942304583236"
            "90322294816580855933212334827479782620414472316873817718091929988125040402618"
            "4124858368.000000");

        CHECK(format_as<std::string>("{:f}", 234987324.4545) == "234987324.454500");
        CHECK(format_as<std::string>("{:g}", 234987324.4545) == "2.34987e+08");
        CHECK(format_as<std::string>("{:a}", 234987324.4545) == "0x1.c033e78e8b439p+27");
    }

    SECTION("strings") {
        char const s[] = "array";

        CHECK(format_as<std::string>("{}", "test") == "test");
        CHECK(format_as<std::string>("{}", std::string("test")) == "test");
        CHECK(format_as<std::string>("{}", std::string_view("test")) == "test");
        CHECK(format_as<std::string>("{}", up::string_view("test")) == "test");

        CHECK(format_as<std::string>("{}", s) == "array");

        CHECK(format_as<std::string>("{}{}{}", up::string_view("ab"), std::string("cd"), "ef") == "abcdef");

        CHECK(format_as<std::string>(std::string("a{}c"), "b") == "abc");

        CHECK(format_as<std::string>("{:-8}{:05}", "value", 42) == "value00042");
    }

    SECTION("booleans") {
        CHECK(format_as<std::string>("{}", true) == "true");
        CHECK(format_as<std::string>("{}", false) == "false");
    }

    SECTION("pointers") {
        void const* ptr = reinterpret_cast<void const*>(static_cast<std::uintptr_t>(0xDEADC0DE));
        int const* iptr = reinterpret_cast<int const*>(static_cast<std::uintptr_t>(0xFEFEFEFE));

        CHECK(format_as<std::string>("{:x}", ptr) == "deadc0de");
        CHECK(format_as<std::string>("{:x}", iptr) == "fefefefe");
    }

    SECTION("enums") {
        CHECK(format_as<std::string>("{:0}", standard_enum::two) == "1");
        CHECK(format_as<std::string>("{}", custom_enum::bar) == "bar");
    }

    SECTION("custom") {
        custom_type local;
        custom_type& ref = local;

        CHECK(format_as<std::string>("{}", custom_type{}) == "custom");
        CHECK(format_as<std::string>("{}", local) == "custom");
        CHECK(format_as<std::string>("{}", ref) == "custom");
    }

    // SECTION("errors") {
    //    char buffer[256];

    //    CHECK(format_to(buffer, "{} {:4d} {:3.5f}", "abc", 9, 12.57) == up::format_result::success);
    //    CHECK(format_to(buffer, "{} {:4d", "abc", 9) == up::format_result::malformed_input);
    //    CHECK(format_to(buffer, "{0} {1}", "abc", 9) == up::format_result::success);
    //    CHECK(format_to(buffer, "{0} {1} {5}", "abc", 9, 12.57) == up::format_result::out_of_range);
    //}
}
