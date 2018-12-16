// Copyright (C) 2014 Sean Middleditch, all rights reserverd.

#include "string_format.h"

namespace gm::_detail {
    auto platform_fatal_error(char const* file, int line, char const* failedConditionText, char const* messageText, char const* callstackText) -> error_action {
        fprintf(stderr, "%s(%d): failed %s: %s\n%s\n", file, line, failedConditionText, messageText, callstackText);
        std::abort();
    }
} // namespace gm::_detail
