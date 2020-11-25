// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/spud/box.h"
#include "potato/spud/concepts.h"
#include "potato/spud/std_iostream.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

#include <string>

namespace up {
    class Logger;
    struct ImporterConfig;

    class ImporterContext {
    public:
        struct Output {
            string logicalAsset;
            string path;
            string type;
        };

        UP_TOOLS_API ImporterContext(
            zstring_view sourceFilePath,
            zstring_view sourceFolderPath,
            zstring_view destinationFolderPath,
            ImporterConfig const& config,
            Logger& logger);
        UP_TOOLS_API ~ImporterContext();

        ImporterContext(ImporterContext&&) = delete;
        ImporterContext& operator=(ImporterContext&&) = delete;

        auto sourceFilePath() const noexcept { return _sourceFilePath; }
        auto sourceFolderPath() const noexcept { return _sourceFolderPath; }
        auto destinationFolderPath() const noexcept { return _destinationFolderPath; }

        UP_TOOLS_API void addSourceDependency(zstring_view path);
        void addOutput(string logicalAsset, string path, string type);
        void addMainOutput(string path, string type);

        view<string> sourceDependencies() const noexcept { return _sourceDependencies; }
        view<Output> outputs() const noexcept { return _outputs; }

        Logger& logger() noexcept { return _logger; }

        UP_TOOLS_API ImporterConfig const& config() const noexcept;
        template <derived_from<ImporterConfig> ImporterConfigT>
        ImporterConfigT const& config() const noexcept {
            return static_cast<ImporterConfigT const&>(config());
        }

    private:
        ImporterConfig const* _config = nullptr;
        zstring_view _sourceFilePath;
        zstring_view _sourceFolderPath;
        zstring_view _destinationFolderPath;

        vector<string> _sourceDependencies;
        vector<Output> _outputs;

        Logger& _logger;
    };
} // namespace up
