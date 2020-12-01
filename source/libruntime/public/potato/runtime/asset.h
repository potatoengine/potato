// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "assertion.h"
#include "uuid.h"

#include "potato/spud/hash.h"
#include "potato/spud/int_types.h"
#include "potato/spud/rc.h"
#include "potato/spud/string.h"
#include "potato/spud/zstring_view.h"

namespace up {
    enum class AssetId : uint64 { Invalid };

    class Asset;
    class AssetTracker;
    template <typename AssetT>
    class AssetHandle;

    struct AssetKey {
        UUID uuid;
        string logical;

        AssetId makeAssetId() const noexcept {
            uint64 hash = hash_value(uuid);
            if (!logical.empty()) {
                hash = hash_combine(hash, hash_value(logical));
            }
            return static_cast<AssetId>(hash);
        }

        constexpr bool operator==(AssetKey const&) const noexcept = default;
    };

    class Asset : public shared<Asset> {
    public:
        explicit Asset(AssetKey key) noexcept : _key(std::move(key)) {}
        inline virtual ~Asset();

        virtual zstring_view assetType() const noexcept = 0;
        AssetKey const& assetKey() const noexcept { return _key; }
        AssetId assetId() const noexcept { return _key.makeAssetId(); }

    private:
        AssetKey _key{};
        AssetTracker* _tracker = nullptr;

        friend class AssetTracker;
    };

    class AssetTracker {
    protected:
        ~AssetTracker() = default;

    private:
        friend class Asset;

        virtual void onAssetReleased(Asset* asset) = 0;
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
        explicit UntypedAssetHandle(AssetKey key) noexcept : _key(std::move(key)), _id(_key.makeAssetId()) {}
        explicit UntypedAssetHandle(AssetId id) noexcept : _id(id) {}
        explicit UntypedAssetHandle(rc<Asset> asset) noexcept : _asset(std::move(asset)) {
            if (_asset != nullptr) {
                _id = _asset->assetId();
            }
        }
        UntypedAssetHandle(AssetKey key, rc<Asset> asset) noexcept : _key(std::move(key)), _asset(std::move(asset)) {
            UP_ASSERT(asset == nullptr || asset->assetKey() == key);
        }

        bool isSet() const noexcept { return _id != AssetId::Invalid; }
        bool ready() const noexcept { return _asset != nullptr; }

        AssetKey const& assetKey() const noexcept { return _key; }
        AssetId assetId() const noexcept { return _id; }

        template <typename AssetT>
        AssetHandle<AssetT> cast() const& noexcept;
        template <typename AssetT>
        AssetHandle<AssetT> cast() && noexcept;

        Asset* asset() const noexcept { return _asset.get(); }
        Asset* release() noexcept { return _asset.release(); }

    private:
        AssetKey _key{};
        AssetId _id = AssetId::Invalid;
        rc<Asset> _asset;
    };

    template <typename AssetT>
    class AssetHandle : public UntypedAssetHandle {
    public:
        using AssetType = AssetT;

        AssetHandle() = default;
        explicit AssetHandle(AssetId id) noexcept : UntypedAssetHandle(id) {}
        AssetHandle(AssetKey key, rc<AssetT> asset) noexcept : UntypedAssetHandle(std::move(key), std::move(asset)) {}
        explicit AssetHandle(rc<AssetT> asset) noexcept : UntypedAssetHandle(std::move(asset)) {}

        zstring_view typeName() const noexcept { return AssetT::assetTypeName; }

        AssetT* asset() const noexcept { return static_cast<AssetT*>(UntypedAssetHandle::asset()); }
        AssetT* release() noexcept { return static_cast<AssetT*>(UntypedAssetHandle::release()); }
    };

    Asset::~Asset() {
        if (_tracker != nullptr) {
            _tracker->onAssetReleased(this);
        }
    }

    template <typename AssetT>
    AssetHandle<AssetT> UntypedAssetHandle::cast() const& noexcept {
        return {_key, _asset};
    }

    template <typename AssetT>
    AssetHandle<AssetT> UntypedAssetHandle::cast() && noexcept {
        return {_key, rc{static_cast<AssetT*>(_asset.release())}};
    }
} // namespace up
