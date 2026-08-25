// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Assets/AssetHandle.h>
#include <MX/Assets/Mesh/MeshAsset.h>

struct MeshComponent
{
    MeshComponent() = default;
    explicit MeshComponent(AssetHandle<MeshAsset> _mesh) : mesh(_mesh) {}

    AssetHandle<MeshAsset> mesh;
};
