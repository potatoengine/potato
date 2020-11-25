// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "importer.h"

namespace up {
    struct CopyImporterConfig : ImporterConfig {
        string type;
    };

    class CopyImporter : public Importer {
    public:
        explicit CopyImporter();
        ~CopyImporter() override;

        bool import(ImporterContext& ctx) override;
        string_view generateSettings(ImporterContext& ctd) override { return {}; }

        reflex::TypeInfo const& configType() const noexcept override;

        string_view name() const noexcept override { return "copy"; }
        uint64 revision() const noexcept override { return 0; }
    };
} // namespace up
