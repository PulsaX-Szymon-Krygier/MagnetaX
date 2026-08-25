// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include "../AbstractAsset.h"
#include "../AssetSource.h"
#include <vector>

class TextureAsset : public AbstractAsset
{
public:
    explicit TextureAsset(AssetSource _source);

    uint32 GetWidth() const { return width; }
    uint32 GetHeight() const { return height; }

    const std::vector<uint8>& GetPixels() const { return pixels; }

private:
    AssetSource source;

    uint32 width = 0;
    uint32 height = 0;

    std::vector<uint8> pixels;

    bool Load() override;
    void Unload() override;
};
