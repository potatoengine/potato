// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/zstring_view.h"
#include "grimm/foundation/vector.h"
#include "grimm/library/asset_record.h"
#include <string>

namespace gm::recon {
    class Context {
    public:
        Context(zstring_view sourceFilePath,
                zstring_view sourceFolderPath,
                zstring_view destinationFolderPath)
            : _sourceFilePath(sourceFilePath),
              _sourceFolderPath(sourceFolderPath),
              _destinationFolderPath(destinationFolderPath) {}

        Context(Context&&) = delete;
        Context& operator=(Context&&) = delete;

        auto sourceFilePath() const noexcept { return _sourceFilePath; }
        auto sourceFolderPath() const noexcept { return _sourceFolderPath; }
        auto destinationFolderPath() const noexcept { return _destinationFolderPath; }

        void addSourceDependency(zstring_view path);
        void addOutput(zstring_view path);
        void addOutputDependency(zstring_view from, zstring_view on, AssetDependencyType type);

        span<std::string const> outputs() const noexcept {
            return span{_outputs.data(), _outputs.size()};
        }

    private:
        zstring_view _sourceFilePath;
        zstring_view _sourceFolderPath;
        zstring_view _destinationFolderPath;

        vector<AssetDependencyRecord> _dependencies;
        vector<std::string> _outputs;
    };
} // namespace gm::recon
