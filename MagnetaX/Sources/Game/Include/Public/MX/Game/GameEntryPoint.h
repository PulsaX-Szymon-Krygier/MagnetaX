// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include "GameModule.h"
#include <memory>

namespace mx
{
    std::unique_ptr<GameModule> CreateGameModule();
}

#define MX_GAME_REGISTER(GameType) \
namespace mx \
{ \
    std::unique_ptr<GameModule> CreateGameModule() \
    { \
        return std::make_unique<GameType>(); \
    } \
}
