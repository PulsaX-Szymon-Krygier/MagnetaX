// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Graphics/Resources/MeshVertex.h>
#include "../AssetSource.h"
#include "../AbstractAsset.h"
#include <vector>

class MeshAsset : public AbstractAsset
{
public:
    explicit MeshAsset(AssetSource _source);

    const std::vector<MeshVertex>& GetVertices() const { return vertices; }
    const std::vector<uint32>& GetIndices() const { return indices; }

private:
    AssetSource source;

    std::vector<MeshVertex> vertices;
    std::vector<uint32> indices;

    bool Load() override;
    void Unload() override;
};
