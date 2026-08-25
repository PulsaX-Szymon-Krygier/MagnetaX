// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/SemVer.h>
#include <MX/Window/WindowConfig.h>

using WindowCreateInfo = WindowConfig;

struct GameCreateInfo
{
    std::string name = "Game";
    SemVer version{};
    WindowCreateInfo window{};

    GameCreateInfo() = default;
};
