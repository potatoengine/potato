// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "importer.h"

namespace up {
    struct JsonImporterConfig : ImporterConfig {
        string type;
    };

    class JsonImporter : public Importer {
    public:
        JsonImporter();
        ~JsonImporter() override;

        bool import(ImporterContext& ctx) override;
        string_view generateSettings(ImporterContext& ctd) const override { return {}; }
        string_view assetType(ImporterContext& ctx) const noexcept override;

        reflex::TypeInfo const& configType() const noexcept override;

        string_view name() const noexcept override { return "json"; }
        uint64 revision() const noexcept override { return 0; }
    };
} // namespace up
