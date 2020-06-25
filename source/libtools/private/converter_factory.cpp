// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "converter_factory.h"
#include "converters/convert_copy.h"
#include "converters/convert_hlsl.h"
#include "converters/convert_ignore.h"
#include "converters/convert_json.h"
#include "converters/convert_material.h"
#include "converters/convert_model.h"

#include "potato/runtime/assertion.h"

up::ConverterFactory::ConverterFactory() = default;

up::ConverterFactory::~ConverterFactory() = default;

auto up::ConverterFactory::findConverterByName(string_view name) const noexcept -> Converter* {
    for (auto const& conv : _converters) {
        if (conv->name() == name) {
            return conv.get();
        }
    }
    return nullptr;
}

void up::ConverterFactory::registerConverter(box<Converter> converter) {
    UP_ASSERT(!converter.empty());
    UP_ASSERT(findConverterByName(converter->name()) == nullptr);

    _converters.push_back(std::move(converter));
}

void up::ConverterFactory::registerDefaultConverters() {
    registerConverter(new_box<CopyConverter>());
    registerConverter(new_box<HlslConverter>());
    registerConverter(new_box<IgnoreConverter>());
    registerConverter(new_box<JsonConverter>());
    registerConverter(new_box<MaterialConverter>());
    registerConverter(new_box<ModelConverter>());
}
