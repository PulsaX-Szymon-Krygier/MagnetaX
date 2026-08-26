// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include <MX/Assets/Mesh/MeshAsset.h>
#include "MeshAssetLoader.h"
#include <utility>

MeshAsset::MeshAsset(AssetSource _source, bool _flipWinding) : source(std::move(_source)), flipWinding(_flipWinding) {}

bool MeshAsset::Load()
{
    return MeshAssetLoader::LoadFromFile(source, vertices, indices, flipWinding);
}

void MeshAsset::Unload()
{
    vertices.clear();
    indices.clear();
}
