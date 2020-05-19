// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/recon/converter.h"

namespace up::recon {
    class MaterialConverter : public Converter {
    public:
        MaterialConverter();
        ~MaterialConverter();

        bool convert(Context& ctx) override;
        string_view generateSettings(Context& ctd) override { return {}; }

        string_view name() const noexcept override { return "mat"; }
        uint64 revision() const noexcept override { return 1; }
    };
} // namespace up::recon
