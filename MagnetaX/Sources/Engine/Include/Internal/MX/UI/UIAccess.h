// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/UI/UI.h>

class AssetManager;
struct UIRenderData;

class UIAccess
{
public:
    static void Bind(UI& ui, AssetManager* assetManager);

    static void BeginFrame(UI& ui);

    static const UIRenderData& GetRenderData(const UI& ui);
};
