// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Game/GameModule.h>
#include <MX/Graphics/GraphicsDebugView.h>

class TestGame : public GameModule
{
public:
    TestGame();
    ~TestGame() override;

private:
    Scene scene1{};

    void OnCreate(GameCreateInfo& createInfo) override;
    void OnInit() override;
    void OnUpdate(float64 deltaTime) override;
    void OnExit() override;
};
