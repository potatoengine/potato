// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

namespace up {
    enum class IOResult {
        Success,
        AccessDenied,
        FileNotFound,
        System,
        InvalidArgument,
        UnsupportedOperation,
        Malformed,
        Unknown,
    };

    template <typename T> struct IOResultValue {
        IOResult result;
        T value;
    };
} // namespace up
