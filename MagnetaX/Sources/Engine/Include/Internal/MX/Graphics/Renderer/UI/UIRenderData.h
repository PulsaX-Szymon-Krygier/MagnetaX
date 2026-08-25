// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include "UIVertex.h"
#include <vector>

class UIFont;

struct UIRenderData
{
    const UIFont* font = nullptr;
    uint64 fontVersion = 0;

    std::vector<UIVertex> vertices;
};
