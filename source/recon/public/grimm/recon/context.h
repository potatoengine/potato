// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include <filesystem>

namespace gm::recon {
    using path = std::filesystem::path;

    class Context {
    public:
        Context(path sourceFilePath,
                path const& sourceFolderPath,
                path const& destinationFolderPath)
            : _sourceFilePath(std::move(sourceFilePath)),
              _sourceFolderPath(std::move(sourceFolderPath)),
              _destinationFolderPath(std::move(destinationFolderPath)) {}

        Context(Context&&) = delete;
        auto operator=(Context&&) = delete;

        path const& sourceFilePath() const noexcept { return _sourceFilePath; }
        path const& sourceFolderPath() const noexcept { return _sourceFolderPath; }
        path const& destinationFolderPath() const noexcept { return _destinationFolderPath; }

    private:
        path _sourceFilePath;
        path const& _sourceFolderPath;
        path const& _destinationFolderPath;
    };
} // namespace gm::recon
