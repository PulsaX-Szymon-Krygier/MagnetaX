// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Game/GameModule.h>
#include <memory>

class GameApp final
{
public:
    explicit GameApp(std::unique_ptr<GameModule> gameModule);
    ~GameApp();

    void Run();
    void Exit();

    bool IsRunning() const { return running; }

private:
    struct GameAppImpl;

    std::unique_ptr<GameAppImpl> _impl;

    bool running = false;
    bool exitRequested = false;

    bool Init();
    void Tick();
    void Destroy();
};
