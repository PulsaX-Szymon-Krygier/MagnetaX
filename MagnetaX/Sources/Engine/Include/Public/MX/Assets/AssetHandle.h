// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include "AbstractAsset.h"
#include <concepts>

template<typename T>
concept AssetType = std::derived_from<T, AbstractAsset>;

template<AssetType T>
class AssetHandle
{
public:
    AssetHandle() = default;

    bool IsValid() const { return id != INVALID_ASSET_ID; }
    uint64 GetID() const { return id; }

    explicit operator bool() const { return IsValid(); }

    bool operator==(const AssetHandle&) const = default;

private:
    static constexpr uint64 INVALID_ASSET_ID = 0;

    friend class AssetManager;

    uint64 id = INVALID_ASSET_ID;

    explicit AssetHandle(uint64 _id) : id(_id) {}
};
