// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "converter.h"

namespace up {
    class CopyConverter : public Converter {
    public:
        CopyConverter();
        ~CopyConverter() override;

        bool convert(ConverterContext& ctx) override;
        string_view generateSettings(ConverterContext& ctd) override { return {}; }

        string_view name() const noexcept override { return "copy"; }
        uint64 revision() const noexcept override { return 0; }
    };
} // namespace up
