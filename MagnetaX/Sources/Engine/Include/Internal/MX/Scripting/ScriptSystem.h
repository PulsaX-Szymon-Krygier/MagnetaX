// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>

class EngineContext;
class Scene;

class ScriptSystem
{
public:
    static void Update(Scene& scene, EngineContext& engineContext, float64 deltaTime);
};
