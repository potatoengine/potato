// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/recon/converter.h"

namespace up::recon {
    class IgnoreConverter : public Converter {
    public:
        IgnoreConverter() = default;
        ~IgnoreConverter() = default;

        bool convert(Context& ctx) override { return true; }
        string_view generateSettings(Context& ctd) override { return {}; }

        string_view name() const noexcept override { return "ignore"; }
        uint64 revision() const noexcept override { return 0; }
    };
} // namespace up::recon
