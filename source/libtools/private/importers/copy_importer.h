// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "importer.h"

namespace up {
    class CopyImporter : public Importer {
    public:
        CopyImporter();
        ~CopyImporter() override;

        bool import(ImporterContext& ctx) override;
        string_view generateSettings(ImporterContext& ctd) override { return {}; }

        string_view name() const noexcept override { return "copy"; }
        uint64 revision() const noexcept override { return 0; }
    };
} // namespace up
