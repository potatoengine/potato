// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "converter_context.h"

namespace up {
    class Converter {
    public:
        Converter() = default;
        virtual ~Converter() = default;

        Converter(Converter&&) = delete;
        Converter& operator=(Converter&&) = delete;

        virtual bool convert(ConverterContext& ctx) = 0;

        virtual string_view generateSettings(ConverterContext& ctd) = 0;

        virtual string_view name() const noexcept = 0;
        virtual uint64 revision() const noexcept = 0;
    };
} // namespace up
