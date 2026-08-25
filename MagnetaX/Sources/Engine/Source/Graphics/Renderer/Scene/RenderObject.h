// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Assets/AssetHandle.h>
#include <MX/Assets/Material/MaterialAsset.h>
#include <MX/Assets/Mesh/MeshAsset.h>
#include <MX/Core/Math/Matrix.h>

struct RenderObject
{
    AssetHandle<MeshAsset> mesh;
    AssetHandle<MaterialAsset> material;

    Matrix4f model = Matrix4f::Identity();
    Matrix4f mvp = Matrix4f::Identity();
};
