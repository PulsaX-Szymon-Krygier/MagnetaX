// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "TextureAssetLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <cstring>

bool TextureAssetLoader::LoadFromFile(const AssetSource& source, ImageFormat format, uint32& width, uint32& height, std::vector<uint8>& pixels)
{
    bool hdr = false;

    if (format == ImageFormat::RGBA32_FLOAT)
    {
        if (!stbi_is_hdr(source.GetPath().c_str())) return false;

        hdr = true;
    }
    else if (format != ImageFormat::RGBA8_SRGB && format != ImageFormat::RGBA8_UNORM) return false;

    int32 loadedWidth = 0;
    int32 loadedHeight = 0;

    void* loadedPixels = nullptr;

    if (hdr)
    {
        loadedPixels = stbi_loadf(source.GetPath().c_str(), &loadedWidth, &loadedHeight, nullptr, STBI_rgb_alpha);
    }
    else
    {
        loadedPixels = stbi_load(source.GetPath().c_str(), &loadedWidth, &loadedHeight, nullptr, STBI_rgb_alpha);
    }

    if (!loadedPixels) return false;

    if (loadedWidth <= 0 || loadedHeight <= 0)
    {
        stbi_image_free(loadedPixels);
        return false;
    }

    width = static_cast<uint32>(loadedWidth);
    height = static_cast<uint32>(loadedHeight);

    const usize dataSize = static_cast<usize>(width) * static_cast<usize>(height) * ImageFormatUtils::GetBytesPerPixel(format);

    pixels.resize(dataSize);
    std::memcpy(pixels.data(), loadedPixels, dataSize);

    stbi_image_free(loadedPixels);

    return true;
}
