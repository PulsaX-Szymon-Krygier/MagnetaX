// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include "UITexture.h"

class AbstractUIRenderer
{
public:
    virtual ~AbstractUIRenderer() = default;

    virtual UITextureHandle CreateTexture(const UITextureCreateInfo& createInfo) = 0;
    virtual void DestroyTexture(UITextureHandle texture) = 0;
};
