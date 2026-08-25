// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "GameApp.h"

#include <MX/Game/GameEntryPoint.h>
#include <memory>

int main()
{
    std::unique_ptr<GameModule> game = mx::CreateGameModule();
    if (!game) return 1;

    GameApp app(std::move(game));
    app.Run();

    return 0;
}
