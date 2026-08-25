// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include <MX/Core/Math/Math.h>
#include <vector>
#include <array>
#include <span>

struct UIGlyph
{
    Vector2f uvMin{ 0.0f };
    Vector2f uvMax{ 0.0 };

    Vector2f size{ 0.0 };
    Vector2f offset{ 0.0f };

    float32 advance = 0.0f;
};

class UIFont
{
public:
    static constexpr uint32 FIRST_CHARACTER = 32;
    static constexpr uint32 CHARACTER_COUNT = 95;

    bool Create(std::span<const uint8> fontData, float32 pixelHeight = 20.0f, uint32 atlasWidth = 512, uint32 atlasHeight = 512);
    void Destroy();

    const UIGlyph* GetGlyph(uint32 codepoint) const;

    const std::vector<uint8>& GetAtlasPixels() const { return atlasPixels; }

    uint32 GetAtlasWidth() const { return atlasWidth; }
    uint32 GetAtlasHeight() const { return atlasHeight; }

    float32 GetBaseline() const { return baseline; }
    float32 GetLineHeight() const { return lineHeight; }

private:
    std::vector<uint8> atlasPixels;
    std::array<UIGlyph, CHARACTER_COUNT> glyphs{};

    uint32 atlasWidth = 0;
    uint32 atlasHeight = 0;    

    float32 baseline = 0.0f;
    float32 lineHeight = 0.0f;
};
