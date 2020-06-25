// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "converter.h"

namespace up {
    class IgnoreConverter : public Converter {
    public:
        IgnoreConverter() = default;

        bool convert(ConverterContext& ctx) override { return true; }
        string_view generateSettings(ConverterContext& ctd) override { return {}; }

        string_view name() const noexcept override { return "ignore"; }
        uint64 revision() const noexcept override { return 0; }
    };
} // namespace up
