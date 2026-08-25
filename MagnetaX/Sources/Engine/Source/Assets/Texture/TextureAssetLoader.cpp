// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "TextureAssetLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

bool TextureAssetLoader::LoadFromFile(const AssetSource& source, uint32& width, uint32& height, std::vector<uint8>& pixels)
{
    int32 loadedWidth = 0;
    int32 loadedHeight = 0;

    stbi_uc* loadedPixels = stbi_load(source.GetPath().c_str(), &loadedWidth, &loadedHeight, nullptr, STBI_rgb_alpha);

    if (!loadedPixels) return false;

    if (loadedWidth <= 0 || loadedHeight <= 0)
    {
        stbi_image_free(loadedPixels);
        return false;
    }

    width = static_cast<uint32>(loadedWidth);
    height = static_cast<uint32>(loadedHeight);

    const usize pixelCount = static_cast<usize>(width) * static_cast<usize>(height) * 4;

    pixels.assign(loadedPixels, loadedPixels + pixelCount);

    stbi_image_free(loadedPixels);

    return true;
}
