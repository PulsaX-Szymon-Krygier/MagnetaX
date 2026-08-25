// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>

class Scene;

class SceneManager
{
public:
    Scene* GetActiveScene() const { return activeScene; }
    void SetActiveScene(Scene& scene) { activeScene = &scene; }

private:
    Scene* activeScene = nullptr;
};
