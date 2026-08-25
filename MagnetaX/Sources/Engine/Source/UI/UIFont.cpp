// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include <MX/UI/UIFont.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

bool UIFont::Create(std::span<const uint8> fontData, float32 pixelHeight, uint32 width, uint32 height)
{
    if (fontData.empty() || pixelHeight <= 0.0f || width == 0 || height == 0) return false;

    Destroy();

    atlasWidth = width;
    atlasHeight = height;

    atlasPixels.resize(static_cast<uint64>(atlasWidth) * atlasHeight);

    stbtt_fontinfo fontInfo{};

    int32 fontOffset = stbtt_GetFontOffsetForIndex(fontData.data(), 0);
    if (fontOffset < 0) return false;

    if (!stbtt_InitFont(&fontInfo, fontData.data(), fontOffset)) return false;

    int32 asc = 0;
    int32 desc = 0;
    int32 gap = 0;

    stbtt_GetFontVMetrics(&fontInfo, &asc, &desc, &gap);

    float32 fontScale = stbtt_ScaleForPixelHeight(&fontInfo, pixelHeight);

    baseline = asc * fontScale;
    lineHeight = (asc - desc + gap) * fontScale;

    std::array<stbtt_bakedchar, CHARACTER_COUNT> bakedChars{};

    int32 result = stbtt_BakeFontBitmap(fontData.data(), 0, pixelHeight, atlasPixels.data(), (int32)atlasWidth, (int32)atlasHeight,
        FIRST_CHARACTER, CHARACTER_COUNT, bakedChars.data());

    if (result <= 0)
    {
        Destroy();
        return false;
    }

    for (uint32 i = 0; i < CHARACTER_COUNT; ++i)
    {
        const stbtt_bakedchar& bakedChar = bakedChars[i];
        UIGlyph& glyph = glyphs[i];

        float32 x0 = (float32)bakedChar.x0;
        float32 x1 = (float32)bakedChar.x1;
        float32 y0 = (float32)bakedChar.y0;
        float32 y1 = (float32)bakedChar.y1;

        glyph.uvMin = Vector2f(x0 / atlasWidth, y0 / atlasHeight);
        glyph.uvMax = Vector2f(x1 / atlasWidth, y1 / atlasHeight);
        glyph.size = Vector2f(x1 - x0, y1 - y0);
        glyph.offset = Vector2f(bakedChar.xoff, bakedChar.yoff);
        glyph.advance = bakedChar.xadvance;
    }

    return true;
}

void UIFont::Destroy()
{
    atlasPixels.clear();
    glyphs = {};

    atlasWidth = 0;
    atlasHeight = 0;

    baseline = 0.0f;
    lineHeight = 0.0f;
}

const UIGlyph* UIFont::GetGlyph(uint32 codepoint) const
{
    if (codepoint < FIRST_CHARACTER || codepoint >= FIRST_CHARACTER + CHARACTER_COUNT) return nullptr;

    return& glyphs[codepoint - FIRST_CHARACTER];
}
