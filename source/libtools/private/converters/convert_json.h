// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "converter.h"

namespace up {
    class JsonConverter : public Converter {
    public:
        JsonConverter();
        ~JsonConverter() override;

        bool convert(ConverterContext& ctx) override;
        string_view generateSettings(ConverterContext& ctd) override { return {}; }

        string_view name() const noexcept override { return "json"; }
        uint64 revision() const noexcept override { return 0; }
    };
} // namespace up
