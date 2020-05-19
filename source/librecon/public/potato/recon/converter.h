// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/recon/context.h"

namespace up::recon {
    class Converter {
    public:
        Converter() = default;
        virtual ~Converter() = default;

        Converter(Converter&&) = delete;
        Converter& operator=(Converter&&) = delete;

        virtual bool convert(Context& ctx) = 0;

        virtual string_view generateSettings(Context& ctd) = 0;

        virtual string_view name() const noexcept = 0;
        virtual uint64 revision() const noexcept = 0;
    };
} // namespace up::recon
