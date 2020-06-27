// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/spud/box.h"
#include "potato/spud/string_view.h"
#include "potato/spud/vector.h"

namespace up {
    class Converter;

    class ConverterFactory {
    public:
        UP_TOOLS_API ConverterFactory();
        UP_TOOLS_API ~ConverterFactory();

        ConverterFactory(ConverterFactory const&) = delete;
        ConverterFactory& operator=(ConverterFactory const&) = delete;

        UP_TOOLS_API Converter* findConverterByName(string_view name) const noexcept;

        UP_TOOLS_API void registerConverter(box<Converter> converter);

        UP_TOOLS_API void registerDefaultConverters();

    private:
        vector<box<Converter>> _converters;
    };
} // namespace up
