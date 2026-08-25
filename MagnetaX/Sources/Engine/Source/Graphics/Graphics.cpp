// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include <MX/Graphics/Graphics.h>
#include <MX/Graphics/AbstractGraphicsSystem.h>
#include <MX/Graphics/GraphicsAccess.h>

struct Graphics::GraphicsImpl
{
    AbstractGraphicsSystem* system = nullptr;
};

Graphics::Graphics() : _impl(std::make_unique<GraphicsImpl>()) {}

Graphics::~Graphics() = default;

void Graphics::SetDebugView(GraphicsDebugView view)
{
    if (!_impl->system) return;

    _impl->system->SetDebugView(view);
}

GraphicsDeviceInfo Graphics::GetDeviceInfo() const
{
    if (!_impl->system) return {};

    return _impl->system->GetDeviceInfo();
}

void GraphicsAccess::Bind(Graphics& graphics, AbstractGraphicsSystem* system)
{
    graphics._impl->system = system;
}
