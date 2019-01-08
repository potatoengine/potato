// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/recon/context.h"

namespace gm::recon {
    class Converter {
    public:
        Converter() = default;
        virtual ~Converter() = default;

        Converter(Converter&&) = delete;
        Converter& operator=(Converter&&) = delete;

        virtual bool convert(Context& ctx) = 0;
    };
} // namespace gm::recon
