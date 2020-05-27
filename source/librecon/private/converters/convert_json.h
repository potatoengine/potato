// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "converter.h"

namespace up::recon {
    class JsonConverter : public Converter {
    public:
        JsonConverter();
        ~JsonConverter();

        bool convert(Context& ctx) override;
        string_view generateSettings(Context& ctd) override { return {}; }

        string_view name() const noexcept override { return "json"; }
        uint64 revision() const noexcept override { return 0; }
    };
} // namespace up::recon
