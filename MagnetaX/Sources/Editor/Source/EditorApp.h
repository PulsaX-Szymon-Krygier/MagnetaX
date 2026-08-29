// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include <memory>

class EditorApp
{
public:
    EditorApp();
    ~EditorApp();

    void Run();
    void Exit();

    bool IsRunning() const { return running; }

private:
    struct EditorAppImpl;
    std::unique_ptr<EditorAppImpl> _impl;

    bool running = false;
    bool exitRequested = false;

    bool Init();
    void Tick();
    void Destroy();
};
