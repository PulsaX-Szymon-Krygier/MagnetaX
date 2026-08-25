// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include <MX/Assets/AssetManager.h>
#include <unordered_map>

namespace
{
    struct AssetRecord
    {
        std::unique_ptr<AbstractAsset> asset;
        AssetState state = AssetState::UNLOADED;
    };
}

struct AssetManager::AssetManagerImpl
{
    uint64 nextAssetID = 1;
    std::unordered_map<uint64, AssetRecord> assets;
};

AssetManager::AssetManager() : _impl(std::make_unique<AssetManagerImpl>()) {}

AssetManager::~AssetManager() = default;

uint64 AssetManager::RegisterAsset(std::unique_ptr<AbstractAsset> asset)
{
    const uint64 id = _impl->nextAssetID++;

    AssetRecord record;
    record.asset = std::move(asset);

    _impl->assets.emplace(id, std::move(record));

    return id;
}

AbstractAsset* AssetManager::GetAsset(uint64 id)
{
    auto it = _impl->assets.find(id);
    if (it == _impl->assets.end()) return nullptr;

    return it->second.asset.get();
}

AssetState AssetManager::GetAssetState(uint64 id) const
{
    auto it = _impl->assets.find(id);
    if (it == _impl->assets.end()) return AssetState::INVALID;

    return it->second.state;
}

bool AssetManager::LoadAsset(uint64 id)
{
    auto it = _impl->assets.find(id);
    if (it == _impl->assets.end()) return false;

    AssetRecord& record = it->second;

    if (record.state == AssetState::LOADED) return true;
    if (record.state == AssetState::LOADING) return false;

    record.state = AssetState::LOADING;

    if (!record.asset->Load())
    {
        record.asset->Unload();
        record.state = AssetState::FAILED;

        return false;
    }

    record.state = AssetState::LOADED;

    return true;
}

bool AssetManager::UnloadAsset(uint64 id)
{
    auto it = _impl->assets.find(id);
    if (it == _impl->assets.end()) return false;

    AssetRecord& record = it->second;

    if (record.state == AssetState::UNLOADED) return true;
    if (record.state == AssetState::LOADING) return false;

    record.asset->Unload();
    record.state = AssetState::UNLOADED;

    return true;
}
