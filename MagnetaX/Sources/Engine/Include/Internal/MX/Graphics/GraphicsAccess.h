// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Graphics/Graphics.h>

class AbstractGraphicsSystem;

class GraphicsAccess
{
public:
    static void Bind(Graphics& graphics, AbstractGraphicsSystem* system);
};
