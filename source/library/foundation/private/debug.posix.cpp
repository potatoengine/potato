// Copyright (C) 2014 Sean Middleditch, all rights reserverd.

#include "grimm/foundation/string_format.h"
#include <cstdio>
#include <cstdlib>

namespace up::_detail {
    auto platform_fatal_error(char const* file, int line, char const* failedConditionText, char const* messageText, char const* callstackText) -> error_action {
        std::fprintf(stderr, "%s(%d): failed %s: %s\n%s\n", file, line, failedConditionText, messageText, callstackText);
        std::abort();
    }
} // namespace up::_detail
