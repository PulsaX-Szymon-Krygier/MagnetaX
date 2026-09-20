// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include <MX/Assets/Mesh/MeshAsset.h>
#include "MeshAssetLoader.h"
#include <utility>
#include <algorithm>

MeshAsset::MeshAsset(AssetSource _source, bool _flipWinding) : source(std::move(_source)), flipWinding(_flipWinding) {}

bool MeshAsset::Load()
{
    if (!MeshAssetLoader::LoadFromFile(source, vertices, indices, flipWinding)) return false;

    if (vertices.empty())
    {
        localBoundsMin = {};
        localBoundsMax = {};
        return true;
    }

    localBoundsMin = vertices[0].position;
    localBoundsMax = vertices[0].position;

    for (const MeshVertex& vertex : vertices)
    {
        localBoundsMin.x = std::min(localBoundsMin.x, vertex.position.x);
        localBoundsMin.y = std::min(localBoundsMin.y, vertex.position.y);
        localBoundsMin.z = std::min(localBoundsMin.z, vertex.position.z);

        localBoundsMax.x = std::max(localBoundsMax.x, vertex.position.x);
        localBoundsMax.y = std::max(localBoundsMax.y, vertex.position.y);
        localBoundsMax.z = std::max(localBoundsMax.z, vertex.position.z);
    }

    return true;
}

void MeshAsset::Unload()
{
    vertices.clear();
    indices.clear();

    localBoundsMin = {};
    localBoundsMax = {};
}
