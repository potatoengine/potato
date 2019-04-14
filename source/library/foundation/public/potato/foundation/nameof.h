// https://stackoverflow.com/a/20170989
// By Howard Hinnant

#pragma once

#include "potato/foundation/string_view.h"

namespace up {
    template <class T>
    constexpr string_view nameof() {
#ifdef __clang__
        string_view p = __PRETTY_FUNCTION__;
        return string_view(p.data() + 34, p.size() - 34 - 1);
#elif defined(__GNUC__)
        string_view p = __PRETTY_FUNCTION__;
#    if __cplusplus < 201402
        return string_view(p.data() + 36, p.size() - 36 - 1);
#    else
        return string_view(p.data() + 49, p.find(';', 49) - 49);
#    endif
#elif defined(_MSC_VER)
        string_view p = __FUNCSIG__;
        return string_view(p.data() + 84, p.size() - 84 - 7);
#endif
    }
} // namespace up
