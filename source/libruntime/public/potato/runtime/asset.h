// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "assertion.h"

#include "potato/spud/int_types.h"
#include "potato/spud/rc.h"
#include "potato/spud/zstring_view.h"

namespace up {
    enum class AssetId : uint64 { Invalid };

    class Asset;
    template <typename AssetT>
    class AssetHandle;

    class Asset : public shared<Asset> {
    public:
        explicit Asset(AssetId id) noexcept : _id(id) {}
        virtual ~Asset() = default;

        virtual zstring_view assetType() const noexcept = 0;
        AssetId assetId() const noexcept { return _id; }

    private:
        AssetId _id{};
    };

    template <typename DerivedT>
    class AssetBase : public Asset {
    public:
        using Asset::Asset;

        using Handle = AssetHandle<DerivedT>;

        zstring_view assetType() const noexcept final { return DerivedT::assetTypeName; }
    };

    class UntypedAssetHandle {
    public:
        UntypedAssetHandle() = default;
        explicit UntypedAssetHandle(AssetId id) noexcept : _id(id) {}
        explicit UntypedAssetHandle(rc<Asset> asset) noexcept : _asset(std::move(asset)) {
            if (_asset != nullptr) {
                _id = _asset->assetId();
            }
        }
        UntypedAssetHandle(AssetId id, rc<Asset> asset) noexcept : _id(id), _asset(std::move(asset)) {
            UP_ASSERT(asset == nullptr || asset->assetId() == id);
        }

        bool isSet() const noexcept { return _id != AssetId::Invalid; }
        bool ready() const noexcept { return _asset != nullptr; }

        AssetId assetId() const noexcept { return _id; }

        template <typename AssetT>
        AssetHandle<AssetT> cast() const& noexcept;
        template <typename AssetT>
        AssetHandle<AssetT> cast() && noexcept;

        Asset* asset() const noexcept { return _asset.get(); }
        Asset* release() noexcept { return _asset.release(); }

    private:
        AssetId _id = AssetId::Invalid;
        rc<Asset> _asset;
    };

    template <typename AssetT>
    class AssetHandle : public UntypedAssetHandle {
    public:
        using AssetType = AssetT;

        using UntypedAssetHandle::UntypedAssetHandle;
        explicit AssetHandle(rc<AssetT> asset) noexcept : UntypedAssetHandle(std::move(asset)) {}

        zstring_view typeName() const noexcept { return AssetT::assetTypeName; }

        AssetT* asset() const noexcept { return static_cast<AssetT*>(UntypedAssetHandle::asset()); }
        AssetT* release() noexcept { return static_cast<AssetT*>(UntypedAssetHandle::release()); }
    };

    template <typename AssetT>
    AssetHandle<AssetT> UntypedAssetHandle::cast() const& noexcept {
        return {_id, _asset};
    }

    template <typename AssetT>
    AssetHandle<AssetT> UntypedAssetHandle::cast() && noexcept {
        return {_id, std::move(_asset)};
    }
} // namespace up
