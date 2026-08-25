// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include "Renderer/RendererConfig.h"

struct TextureConfig
{
    bool mipmaps = true;
    float32 anisotropy = 8.0f;
};

struct GraphicsConfig
{
    TextureConfig texture{};
    RendererConfig renderer{};
};
