// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Assets/AssetSource.h>
#include <MX/Graphics/Resources/MeshVertex.h>
#include <vector>

struct MeshAssetLoader
{
    static bool LoadFromFile(const AssetSource& source, std::vector<MeshVertex>& vertices, std::vector<uint32>& indices);
};
