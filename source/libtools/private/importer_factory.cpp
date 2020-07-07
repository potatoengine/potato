// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "importer_factory.h"
#include "importers/copy_importer.h"
#include "importers/hlsl_importer.h"
#include "importers/ignore_importer.h"
#include "importers/json_importer.h"
#include "importers/material_importer.h"
#include "importers/model_importer.h"

#include "potato/runtime/assertion.h"

up::ImporterFactory::ImporterFactory() = default;

up::ImporterFactory::~ImporterFactory() = default;

auto up::ImporterFactory::findImporterByName(string_view name) const noexcept -> Importer* {
    for (auto const& conv : _importers) {
        if (conv->name() == name) {
            return conv.get();
        }
    }
    return nullptr;
}

void up::ImporterFactory::registerImporter(box<Importer> importer) {
    UP_ASSERT(!importer.empty());
    UP_ASSERT(findImporterByName(importer->name()) == nullptr);

    _importers.push_back(std::move(importer));
}

void up::ImporterFactory::registerDefaultImporters() {
    registerImporter(new_box<CopyImporter>());
    registerImporter(new_box<HlslImporter>());
    registerImporter(new_box<IgnoreImporter>());
    registerImporter(new_box<JsonImporter>());
    registerImporter(new_box<MaterialImporter>());
    registerImporter(new_box<ModelImporter>());
}
