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

    template <typename T, typename E = void> struct Result {
        IOResult result;
        T value;
    };

    template <typename T> using IOReturn = Result<T, IOResult>;
} // namespace up
