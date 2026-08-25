// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include <MX/Graphics/GraphicsDebugView.h>
#include <MX/Graphics/GraphicsDeviceInfo.h>
#include <memory>

class GraphicsAccess;

class Graphics
{
public:
    Graphics();
    ~Graphics();

    Graphics(const Graphics&) = delete;
    Graphics& operator=(const Graphics&) = delete;

    void SetDebugView(GraphicsDebugView view);

    GraphicsDeviceInfo GetDeviceInfo() const;

private:
    friend class GraphicsAccess;

    struct GraphicsImpl;
    std::unique_ptr<GraphicsImpl> _impl;
};
