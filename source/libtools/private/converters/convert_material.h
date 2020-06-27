// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "converter.h"

namespace up {
    class MaterialConverter : public Converter {
    public:
        MaterialConverter();
        ~MaterialConverter() override;

        bool convert(ConverterContext& ctx) override;
        string_view generateSettings(ConverterContext& ctd) override { return {}; }

        string_view name() const noexcept override { return "material"; }
        uint64 revision() const noexcept override { return 1; }
    };
} // namespace up
