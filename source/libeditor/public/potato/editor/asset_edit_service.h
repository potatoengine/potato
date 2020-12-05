// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/spud/string.h"
#include "potato/spud/zstring_view.h"

namespace up {
    class AssetEditService {
    public:
        void setAssetRoot(string folder) noexcept { _assetRoot = std::move(folder); }

        UP_EDITOR_API char8_t const* getIconForAssetTypeHash(uint64 typeHash) const noexcept;
        UP_EDITOR_API zstring_view getEditorForAssetTypeHash(uint64 typeHash) const noexcept;
        UP_EDITOR_API string makeFullPath(zstring_view filename) const;

    private:
        string _assetRoot;
    };
} // namespace up
