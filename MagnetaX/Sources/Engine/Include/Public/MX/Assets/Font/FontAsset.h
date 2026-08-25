// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include "../AbstractAsset.h"
#include "../AssetSource.h"
#include <vector>

class FontAsset : public AbstractAsset
{
public:
    explicit FontAsset(AssetSource _source);

    const std::vector<uint8>& GetData() const { return data; }

private:
    AssetSource source;
    std::vector<uint8> data;

    bool Load() override;
    void Unload() override;
};
