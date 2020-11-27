// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/spud/string.h"

namespace up {
    class AssetLoader;
}

namespace up {
    UP_EDITOR_API bool assetBrowserPopup(zstring_view, string& inout_asset, AssetLoader& assetLoader);
}
