// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include <MX/Assets/Font/FontAsset.h>
#include <MX/Core/IO/IOUtils.h>
#include <utility>

FontAsset::FontAsset(AssetSource _source) : source(std::move(_source)) {}

bool FontAsset::Load()
{
    return IOUtils::ReadBinaryFile(source.GetPath(), data);
}

void FontAsset::Unload()
{
    data.clear();
}
