// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/recon/context.h"

namespace up::recon {
    class Converter {
    public:
        Converter() = default;
        virtual ~Converter() = default;

        Converter(Converter&&) = delete;
        Converter& operator=(Converter&&) = delete;

        virtual bool convert(Context& ctx) = 0;

        virtual string_view name() const noexcept = 0;
        virtual uint64 revision() const noexcept = 0;
    };
} // namespace up::recon
