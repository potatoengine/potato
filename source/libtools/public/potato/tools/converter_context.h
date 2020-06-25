// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/assetdb/asset_record.h"
#include "potato/spud/std_iostream.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

#include <string>

namespace up {
    class FileSystem;
    class Logger;
} // namespace up

namespace up {
    class ConverterContext {
    public:
        ConverterContext(zstring_view sourceFilePath,
            zstring_view sourceFolderPath,
            zstring_view destinationFolderPath,
            FileSystem& fileSystem,
            Logger& logger)
            : _sourceFilePath(sourceFilePath)
            , _sourceFolderPath(sourceFolderPath)
            , _destinationFolderPath(destinationFolderPath)
            , _fileSystem(fileSystem)
            , _logger(logger) {}

        ConverterContext(ConverterContext&&) = delete;
        ConverterContext& operator=(ConverterContext&&) = delete;

        auto sourceFilePath() const noexcept { return _sourceFilePath; }
        auto sourceFolderPath() const noexcept { return _sourceFolderPath; }
        auto destinationFolderPath() const noexcept { return _destinationFolderPath; }

        UP_TOOLS_API void addSourceDependency(zstring_view path);
        void addOutput(zstring_view path);
        void addOutputDependency(zstring_view from, zstring_view on, AssetDependencyType type);

        span<string const> sourceDependencies() const noexcept { return span{_sourceDependencies.data(), _sourceDependencies.size()}; }
        span<string const> outputs() const noexcept { return span{_outputs.data(), _outputs.size()}; }

        FileSystem& fileSystem() noexcept { return _fileSystem; }
        Logger& logger() noexcept { return _logger; }

    private:
        zstring_view _sourceFilePath;
        zstring_view _sourceFolderPath;
        zstring_view _destinationFolderPath;

        vector<string> _sourceDependencies;
        vector<string> _outputs;

        FileSystem& _fileSystem;
        Logger& _logger;
    };
} // namespace up
