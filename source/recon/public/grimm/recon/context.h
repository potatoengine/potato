// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/zstring_view.h"

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

    private:
        zstring_view _sourceFilePath;
        zstring_view _sourceFolderPath;
        zstring_view _destinationFolderPath;
    };
} // namespace gm::recon
