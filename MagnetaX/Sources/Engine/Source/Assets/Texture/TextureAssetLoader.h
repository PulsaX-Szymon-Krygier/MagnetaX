// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Assets/AssetSource.h>
#include <MX/Core/CoreMinimal.h>
#include <MX/Graphics/Resources/ImageFormat.h>
#include <vector>

struct TextureAssetLoader
{
    static bool LoadFromFile(const AssetSource& source, ImageFormat format, uint32& width, uint32& height, std::vector<uint8>& pixels);
};
