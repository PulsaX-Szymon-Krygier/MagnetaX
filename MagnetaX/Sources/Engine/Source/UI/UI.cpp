// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include <MX/Assets/AssetManager.h>
#include <MX/Graphics/Renderer/UI/UIRenderData.h>
#include <MX/UI/UI.h>
#include <MX/UI/UIAccess.h>
#include <MX/UI/UIFont.h>
#include <utility>

struct UI::UIImpl
{
    AssetManager* assetManager = nullptr;

    UIFont font{};
    UIRenderData renderData;
};

UI::UI() : _impl(std::make_unique<UIImpl>()) {}

UI::~UI() = default;

bool UI::SetFont(AssetHandle<FontAsset> font)
{
    if (!_impl->assetManager || !font) return false;

    AssetState fontState = _impl->assetManager->GetAssetState(font);

    if (fontState != AssetState::LOADED)
    {
        if (!_impl->assetManager->LoadAsset(font)) return false;
    }

    FontAsset* fontAsset = _impl->assetManager->GetAsset(font);

    if (!fontAsset || fontAsset->GetData().empty()) return false;

    UIFont newFont;

    if (!newFont.Create(fontAsset->GetData())) return false;

    _impl->font.Destroy();
    _impl->font = std::move(newFont);

    _impl->renderData.vertices.clear();
    _impl->renderData.font = &_impl->font;

    ++_impl->renderData.fontVersion;
    if (_impl->renderData.fontVersion == 0) _impl->renderData.fontVersion = 1;

    return true;
}

void UI::DrawText(std::string_view text, const Vector2f& position, const Vector4f& color, float32 scale)
{
    if (!_impl->renderData.font || text.empty() || scale <= 0.0f) return;

    float32 curX = position.x;
    float32 curY = position.y + _impl->font.GetBaseline() * scale;

    for (char c : text)
    {
        if (c == '\n')
        {
            curX = position.x;
            curY += _impl->font.GetLineHeight() * scale;

            continue;
        }

        const UIGlyph* glyph = _impl->font.GetGlyph((uint8)c);
        if (!glyph) continue;

        if (glyph->size.x > 0.0f && glyph->size.y > 0.0f)
        {
            const float32 left = curX + glyph->offset.x * scale;
            const float32 top = curY + glyph->offset.y * scale;
            const float32 right = left + glyph->size.x * scale;
            const float32 bottom = top + glyph->size.y * scale;

            UIVertex topLeft{};
            topLeft.position = Vector2f(left, top);
            topLeft.uv = glyph->uvMin;
            topLeft.color = color;

            UIVertex topRight{};
            topRight.position = Vector2f(right, top);
            topRight.uv = Vector2f(glyph->uvMax.x, glyph->uvMin.y);
            topRight.color = color;

            UIVertex bottomLeft{};
            bottomLeft.position = Vector2f(left, bottom);
            bottomLeft.uv = Vector2f(glyph->uvMin.x, glyph->uvMax.y);
            bottomLeft.color = color;

            UIVertex bottomRight{};
            bottomRight.position = Vector2f(right, bottom);
            bottomRight.uv = glyph->uvMax;
            bottomRight.color = color;

            _impl->renderData.vertices.push_back(topLeft);
            _impl->renderData.vertices.push_back(topRight);
            _impl->renderData.vertices.push_back(bottomLeft);

            _impl->renderData.vertices.push_back(bottomLeft);
            _impl->renderData.vertices.push_back(topRight);
            _impl->renderData.vertices.push_back(bottomRight);
        }

        curX += glyph->advance * scale;
    }
}

void UIAccess::Bind(UI& ui, AssetManager* assetManager)
{
    if (ui._impl->assetManager == assetManager) return;

    ui._impl->assetManager = assetManager;

    ui._impl->renderData = {};
    ui._impl->font.Destroy();
}

void UIAccess::BeginFrame(UI& ui)
{
    ui._impl->renderData.vertices.clear();
}

const UIRenderData& UIAccess::GetRenderData(const UI& ui)
{
    return ui._impl->renderData;
}
