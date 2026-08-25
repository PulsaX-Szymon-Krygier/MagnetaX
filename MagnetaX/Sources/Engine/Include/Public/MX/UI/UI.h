// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include <MX/Core/Math/Vector.h>
#include <MX/Assets/AssetHandle.h>
#include <MX/Assets/Font/FontAsset.h>
#include <memory>
#include <string_view>

class UIAccess;

class UI
{
public:
    UI();

    UI(const UI&) = delete;
    UI& operator=(const UI&) = delete;

    ~UI();

    bool SetFont(AssetHandle<FontAsset> font);

    void DrawText(std::string_view text, const Vector2f& position, const Vector4f& color = Vector4f(1.0f), float32 scale = 1.0f);

private:
    friend class UIAccess;

    struct UIImpl;
    std::unique_ptr<UIImpl> _impl;
};
