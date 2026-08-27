// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Graphics/Resources/ImageFormat.h>
#include "../AbstractAsset.h"
#include "../AssetSource.h"
#include <vector>

class TextureAsset : public AbstractAsset
{
public:
    explicit TextureAsset(AssetSource _source, ImageFormat _format = ImageFormat::RGBA8_SRGB);

    uint32 GetWidth() const { return width; }
    uint32 GetHeight() const { return height; }

    const std::vector<uint8>& GetPixels() const { return pixels; }

    ImageFormat GetFormat() const { return format; }

private:
    AssetSource source;

    ImageFormat format = ImageFormat::RGBA8_SRGB;

    uint32 width = 0;
    uint32 height = 0;

    std::vector<uint8> pixels;

    bool Load() override;
    void Unload() override;
};
