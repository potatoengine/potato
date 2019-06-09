// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "potato/foundation/string.h"
#include "potato/foundation/zstring_view.h"
#include "potato/foundation/vector.h"
#include "potato/foundation/std_iostream.h"
#include "potato/assetdb/asset_record.h"
#include <string>

namespace up {
    class FileSystem;
    class Logger;
} // namespace up

namespace up::recon {
    class Context {
    public:
        Context(zstring_view sourceFilePath,
                zstring_view sourceFolderPath,
                zstring_view destinationFolderPath,
                FileSystem& fileSystem,
                Logger& logger)
            : _sourceFilePath(sourceFilePath),
              _sourceFolderPath(sourceFolderPath),
              _destinationFolderPath(destinationFolderPath),
              _fileSystem(fileSystem),
              _logger(logger) {}

        Context(Context&&) = delete;
        Context& operator=(Context&&) = delete;

        auto sourceFilePath() const noexcept { return _sourceFilePath; }
        auto sourceFolderPath() const noexcept { return _sourceFolderPath; }
        auto destinationFolderPath() const noexcept { return _destinationFolderPath; }

        void addSourceDependency(zstring_view path);
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
} // namespace up::recon
