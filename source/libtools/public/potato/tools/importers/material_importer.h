// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "importer.h"

namespace up {
    class MaterialImporter : public Importer {
    public:
        MaterialImporter();
        ~MaterialImporter() override;

        bool import(ImporterContext& ctx) override;
        string_view generateSettings(ImporterContext& ctd) const override { return {}; }

        string_view name() const noexcept override { return "material"; }
        uint64 revision() const noexcept override { return 3; }
    };
} // namespace up
