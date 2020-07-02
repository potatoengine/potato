// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "importer_context.h"

namespace up {
    class Importer {
    public:
        Importer() = default;
        virtual ~Importer() = default;

        Importer(Importer&&) = delete;
        Importer& operator=(Importer&&) = delete;

        virtual bool import(ImporterContext& ctx) = 0;

        virtual string_view generateSettings(ImporterContext& ctd) = 0;

        virtual string_view name() const noexcept = 0;
        virtual uint64 revision() const noexcept = 0;
    };
} // namespace up
