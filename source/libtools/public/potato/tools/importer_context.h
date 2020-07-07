// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/spud/std_iostream.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

#include <string>

namespace up {
    class Logger;
} // namespace up

namespace up {
    class ImporterContext {
    public:
        struct Output {
            string logicalAsset;
            string path;
        };

        ImporterContext(
            zstring_view sourceFilePath,
            zstring_view sourceFolderPath,
            zstring_view destinationFolderPath,
            Logger& logger)
            : _sourceFilePath(sourceFilePath)
            , _sourceFolderPath(sourceFolderPath)
            , _destinationFolderPath(destinationFolderPath)
            , _logger(logger) {}

        ImporterContext(ImporterContext&&) = delete;
        ImporterContext& operator=(ImporterContext&&) = delete;

        auto sourceFilePath() const noexcept { return _sourceFilePath; }
        auto sourceFolderPath() const noexcept { return _sourceFolderPath; }
        auto destinationFolderPath() const noexcept { return _destinationFolderPath; }

        UP_TOOLS_API void addSourceDependency(zstring_view path);
        void addOutput(string logicalAsset, string path);
        void addMainOutput(string path);

        view<string> sourceDependencies() const noexcept { return _sourceDependencies; }
        view<Output> outputs() const noexcept { return _outputs; }

        Logger& logger() noexcept { return _logger; }

    private:
        zstring_view _sourceFilePath;
        zstring_view _sourceFolderPath;
        zstring_view _destinationFolderPath;

        vector<string> _sourceDependencies;
        vector<Output> _outputs;

        Logger& _logger;
    };
} // namespace up
