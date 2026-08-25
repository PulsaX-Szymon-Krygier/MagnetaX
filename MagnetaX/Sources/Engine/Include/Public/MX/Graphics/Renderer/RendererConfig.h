// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include "PostFX/PostFXConfig.h"
#include "Shadow/ShadowConfig.h"

struct RendererConfig
{
    PostFXConfig postFX{};
    ShadowConfig shadows{};
};
