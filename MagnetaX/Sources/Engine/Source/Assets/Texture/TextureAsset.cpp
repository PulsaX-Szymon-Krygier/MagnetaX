// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include <MX/Assets/Texture/TextureAsset.h>
#include "TextureAssetLoader.h"
#include <utility>

TextureAsset::TextureAsset(AssetSource _source, ImageFormat _format) : source(std::move(_source)), format(_format) {}

bool TextureAsset::Load()
{
    return TextureAssetLoader::LoadFromFile(source, format, width, height, pixels);
}

void TextureAsset::Unload()
{
    pixels.clear();
    width = height = 0;
}
