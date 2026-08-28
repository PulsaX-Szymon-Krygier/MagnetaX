// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include <MX/Core/Math/Matrix.h>
#include <MX/Core/Math/Vector.h>
#include <MX/Assets/AssetHandle.h>
#include <MX/Assets/Texture/TextureAsset.h>
#include "RenderLight.h"
#include "RenderObject.h"
#include <vector>

struct RenderSceneData
{
    Vector3f backgroundColor{ 0.0f };

    Vector3f ambientLightColor{ 1.0f };
    float32 ambientLightIntensity = 0.1f;

    Matrix4f view = Matrix4f::Identity();
    Matrix4f viewProjectionInversed = Matrix4f::Identity();

    Vector3f cameraPosition{ 0.0f };
    float32 cameraNearPlane = 0.1f;
    float32 cameraFarPlane = 1000.0f;
    float32 exposureEV = 0.0f;

    std::vector<RenderLight> lights;
    std::vector<RenderObject> objects;

    AssetHandle<TextureAsset> environmentMap;
};
