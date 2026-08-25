// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include <MX/Assets/Texture/TextureAsset.h>
#include "TextureAssetLoader.h"
#include <utility>

TextureAsset::TextureAsset(AssetSource _source) : source(std::move(_source)) {}

bool TextureAsset::Load()
{
    return TextureAssetLoader::LoadFromFile(source, width, height, pixels);
}

void TextureAsset::Unload()
{
    pixels.clear();
    width = height = 0;
}
