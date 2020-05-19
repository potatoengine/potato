// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/recon/converter.h"

namespace up::recon {
    class ModelConverter : public Converter {
    public:
        ModelConverter();
        ~ModelConverter();

        bool convert(Context& ctx) override;
        string_view generateSettings(Context& ctd) override { return {}; }

        string_view name() const noexcept override { return "model"; }
        uint64 revision() const noexcept override { return 2; }
    };
} // namespace up::recon
