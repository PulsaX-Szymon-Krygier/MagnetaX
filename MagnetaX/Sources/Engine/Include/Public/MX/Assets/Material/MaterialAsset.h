// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/Math/Vector.h>
#include "../AbstractAsset.h"
#include "../AssetHandle.h"
#include "../Texture/TextureAsset.h"

class MaterialAsset : public AbstractAsset
{
public:
    MaterialAsset() = default;

    Vector4f baseColor{ 1.0f };

    AssetHandle<TextureAsset> baseColorTexture;
    Vector2f uvScale{ 1.0f };

    float32 roughness = 0.5f;
    float32 metallic = 0.0f;
    float32 ambientOcclusion = 1.0f;

private:
    bool Load() override;
    void Unload() override;
};
