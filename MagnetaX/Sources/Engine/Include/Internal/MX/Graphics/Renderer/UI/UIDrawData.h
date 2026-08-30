// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include <MX/Core/Math/Vector.h>
#include "UITexture.h"
#include <vector>

struct UIDrawVertex
{
    Vector2f position{ 0.0f };
    Vector2f uv{ 0.0f };
    Vector4f color{ 1.0f };
};

struct UIDrawCommand
{
    Vector2f clipMin{ 0.0f };
    Vector2f clipMax{ 0.0f };

    UITextureHandle texture;

    uint32 indexOffset = 0;
    uint32 indexCount = 0;
    uint32 vertexOffset = 0;
};

struct UIDrawData
{
    std::vector<UIDrawVertex> vertices;
    std::vector<uint32> indices;
    std::vector<UIDrawCommand> commands;
};
