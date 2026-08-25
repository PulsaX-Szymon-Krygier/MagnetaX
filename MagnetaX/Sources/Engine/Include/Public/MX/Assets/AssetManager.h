// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include "AbstractAsset.h"
#include "AssetHandle.h"
#include "AssetState.h"
#include <memory>
#include <utility>

class AssetManager
{
public:
    AssetManager();
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    ~AssetManager();

    template<AssetType T, typename... Args>
    AssetHandle<T> CreateAsset(Args&&... args)
    {
        std::unique_ptr<T> asset = std::make_unique<T>(std::forward<Args>(args)...);
        const uint64 id = RegisterAsset(std::move(asset));

        return AssetHandle<T>(id);
    }

    template<AssetType T>
    bool LoadAsset(AssetHandle<T> handle) { return LoadAsset(handle.GetID()); }

    template<AssetType T>
    T* GetAsset(AssetHandle<T> handle) { return static_cast<T*>(GetAsset(handle.GetID())); }

    template<AssetType T>
    AssetState GetAssetState(AssetHandle<T> handle) const { return GetAssetState(handle.GetID()); }

    template<AssetType T>
    bool UnloadAsset(AssetHandle<T> handle) { return UnloadAsset(handle.GetID()); }

private:
    struct AssetManagerImpl;
    std::unique_ptr<AssetManagerImpl> _impl;

    uint64 RegisterAsset(std::unique_ptr<AbstractAsset> asset);
    AbstractAsset* GetAsset(uint64 id);
    AssetState GetAssetState(uint64 id) const;
    bool LoadAsset(uint64 id);
    bool UnloadAsset(uint64 id);
};
