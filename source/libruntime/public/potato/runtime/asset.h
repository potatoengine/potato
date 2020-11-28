#pragma once

#include "resource_manifest.h"

#include "potato/spud/rc.h"

namespace up {
    using AssetId = ResourceId;
    // enum class AssetId : uint64 { Invalid };

    class Asset : public shared<Asset> {
    public:
        explicit Asset(AssetId id) noexcept : _id(id) {}
        virtual ~Asset() = default;

        AssetId assetId() const noexcept { return _id; }

    private:
        AssetId _id{};
    };
} // namespace up
